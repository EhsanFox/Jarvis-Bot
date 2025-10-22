#include "ServerManager.h"
#include "HttpError.h"
#include "HttpSuccess.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

ServerManager::ServerManager(uint16_t port) : _server(port) {}

void ServerManager::use(Middleware* mw) {
    _middlewares.push_back(mw);
    Serial.println("🟢 Middleware added");
}

void ServerManager::addRouter(Router* router) {
    // ✅ Attach dependency container before registration
    router->attachDependencies(&_deps);

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
            Serial.print("Registering route: ");
            Serial.println(route.path);

            // Case 1: normal GET/POST handler
            if (route.handler) {
                _server.on(route.path.c_str(), method,
                    [this, route, router](AsyncWebServerRequest* request) {
                        size_t i = 0;
                        std::function<void()> runMiddlewares;
                        runMiddlewares = [&]() {
                            if (i < _middlewares.size()) {
                                Middleware* mw = _middlewares[i++];
                                mw->handle(request, runMiddlewares);
                            } else {
                                try {
                                    // Run router guards
                                    for (auto guard : router->routerGuards())
                                        if (!guard->canActivate(request)) return;
                                    // Run route guards
                                    for (auto guard : route.guards)
                                        if (!guard->canActivate(request)) return;

                                    HttpSuccess result = route.handler(request);
                                    if (result.response()) {
                                        request->send(result.response());
                                    } else {
                                        String output = result.output();
                                        request->send(200, "application/json", output);
                                    }
                                } catch (const HttpError& e) {
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
            }

            // Case 2: POST with body handler
            else if (route.bodyHandler && method == HTTP_POST) {
                _server.on(route.path.c_str(), HTTP_POST,
                    [](AsyncWebServerRequest* request) {},
                    nullptr, // no file upload handler
                    [this, route, router](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
                        if (index + len == total) {
                            size_t i = 0;
                            std::function<void()> runMiddlewares;
                            runMiddlewares = [&]() {
                                if (i < _middlewares.size()) {
                                    Middleware* mw = _middlewares[i++];
                                    mw->handle(request, runMiddlewares);
                                } else {
                                    try {
                                        // Run guards
                                        for (auto guard : router->routerGuards())
                                            if (!guard->canActivate(request)) return;
                                        for (auto guard : route.guards)
                                            if (!guard->canActivate(request)) return;

                                        HttpSuccess result = route.bodyHandler(request, data);
                                        if (result.response()) {
                                            request->send(result.response());
                                        } else {
                                            String output = result.output();
                                            request->send(200, "application/json", output);
                                        }
                                    } catch (const HttpError& e) {
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
                        }
                    });
            }
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
    for (auto guard : routerGuards)
        if (!guard->canActivate(request)) return;
    for (auto guard : route.guards)
        if (!guard->canActivate(request)) return;

    // 2. Run handler inside try/catch
    DynamicJsonDocument doc(1024);
    try {
        HttpSuccess result = route.handler(request);
        if (result.response()) {
            request->send(result.response());
        } else {
            String output = result.output();
            request->send(200, "application/json", output);
        }
    } catch (const HttpError& e) {
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
