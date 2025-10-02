#include "ServerManager.h"
#include "../include/HttpError.h"
#include "../include/HttpSuccess.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

ServerManager::ServerManager(uint16_t port) : _server(port) {}

void ServerManager::use(Middleware* mw) {
    _middlewares.push_back(mw);
    Serial.println("🟢 Middleware added");
}

void ServerManager::addRouter(Router* router) {
    _routers.push_back(router);
    Serial.print("📦 Router mounted with base path: ");
    Serial.println(router->basePath());

    for (auto route : router->getEndpoints()) {
        Serial.print("  🟢 GET ");
        Serial.println(route.path);
    }

    for (auto route : router->postEndpoints()) {
        Serial.print("  🟢 POST ");
        Serial.println(route.path);
    }
}

void ServerManager::begin() {
    // Mount FS
    if (!LittleFS.begin(true)) {
        Serial.println("⚠️ Failed to mount LittleFS");
        return;
    }

    // Serve static files (React build in /web)
    _server.serveStatic("/", LittleFS, "/web/").setDefaultFile("index.html");

    for (auto router : _routers) {
        auto registerRoute = [this, router](Router::Route route, WebRequestMethod method) {
            _server.on(route.path.c_str(), method, [this, route, router](AsyncWebServerRequest* request) {
                size_t i = 0;
                std::function<void()> runMiddlewares;
                runMiddlewares = [&]() {
                    if (i < _middlewares.size()) {
                        Middleware* mw = _middlewares[i++];
                        mw->handle(request, runMiddlewares);
                    } else {
                        try {
                            // ✅ Run router-level guards
                            for (auto guard : router->routerGuards()) {
                                if (!guard->canActivate(request)) return;
                            }
                            // ✅ Run route-level guards
                            for (auto guard : route.guards) {
                                if (!guard->canActivate(request)) return;
                            }

                            // Then run handler
                            DynamicJsonDocument doc(2048);
                            String result = route.handler(request);

                            DynamicJsonDocument tmp(1024);
                            DeserializationError err = deserializeJson(tmp, result);
                            doc["ok"] = true;
                            if (!err) {
                                doc["data"] = tmp.as<JsonVariant>();
                            } else {
                                doc["data"] = result;
                            }

                            String output;
                            serializeJson(doc, output);
                            request->send(200, "application/json", output);
                        } catch (const HttpSuccess& s) {
                            // Handle HttpSuccess responses
                            if (s.response()) {
                                request->send(s.response());
                            } else {
                                DynamicJsonDocument tmp(1024);
                                DeserializationError err = deserializeJson(tmp, s.message());
                                DynamicJsonDocument doc(256);
                                doc["ok"] = true;
                                if (!err) {
                                    doc["data"] = tmp.as<JsonVariant>();
                                } else {
                                    doc["data"] = s.message();
                                }
                                String output;
                                serializeJson(doc, output);
                                request->send(200, "application/json", output);
                            }
                        } catch (const HttpError& e) {
                            // Handle HttpError exceptions
                            DynamicJsonDocument doc(256);
                            doc["ok"] = false;
                            doc["error"] = e.message();
                            String output;
                            serializeJson(doc, output);
                            request->send(e.statusCode(), "application/json", output);
                        } catch (const std::exception& e) {
                            DynamicJsonDocument doc(256);
                            doc["ok"] = false;
                            doc["error"] = e.what();
                            String output;
                            serializeJson(doc, output);
                            request->send(500, "application/json", output);
                        } catch (...) {
                            DynamicJsonDocument doc(256);
                            doc["ok"] = false;
                            doc["error"] = "Unknown error";
                            String output;
                            serializeJson(doc, output);
                            request->send(500, "application/json", output);
                        }
                    }
                };
                runMiddlewares();
            });
        };

        for (auto route : router->getEndpoints()) registerRoute(route, HTTP_GET);
        for (auto route : router->postEndpoints()) registerRoute(route, HTTP_POST);
    }

    _server.begin();
    Serial.println("✅ Webserver started!");
}

void ServerManager::processRequest(AsyncWebServerRequest* request,
                                   const Router::Route& route,
                                   const std::vector<Guard*>& routerGuards,
                                   std::function<void()> next) {
    // 1. Run guards
    for (auto guard : routerGuards) {
        if (!guard->canActivate(request)) return;
    }
    for (auto guard : route.guards) {
        if (!guard->canActivate(request)) return;
    }

    // 2. Run handler inside try/catch
    DynamicJsonDocument doc(2048);
    try {
        String result = route.handler(request);

        // Try parsing result as JSON
        DynamicJsonDocument tmp(1024);
        DeserializationError err = deserializeJson(tmp, result);
        doc["ok"] = true;
        if (!err) {
            doc["data"] = tmp.as<JsonVariant>();
        } else {
            doc["data"] = result;
        }
    } catch (const HttpError& e) {
        // Handle HttpError exceptions
        doc["ok"] = false;
        doc["error"] = e.message();
        String output;
        serializeJson(doc, output);
        request->send(e.statusCode(), "application/json", output);
        return;
    } catch (const std::exception& e) {
        doc["ok"] = false;
        doc["error"] = e.what();
    } catch (...) {
        doc["ok"] = false;
        doc["error"] = "Unknown error";
    }

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);

    if (next) next();
}
