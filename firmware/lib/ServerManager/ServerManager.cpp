#include "ServerManager.h"
#include <ArduinoJson.h> // <----- include ArduinoJson

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
    for (auto router : _routers) {
        auto registerRoute = [this](Router::Route route, WebRequestMethod method) {
            _server.on(route.path.c_str(), method, [this, route](AsyncWebServerRequest* request){
                size_t i = 0;
                std::function<void()> runMiddlewares;
                runMiddlewares = [&]() {
                    if (i < _middlewares.size()) {
                        Middleware* mw = _middlewares[i++];
                        mw->handle(request, runMiddlewares);
                    } else {
                        // Interceptor
                        DynamicJsonDocument doc(2048);
                        try {
                            // Call handler and capture result as string
                            String result = route.handler(request);

                            // Attempt to parse as JSON
                            DynamicJsonDocument tmp(1024);
                            DeserializationError err = deserializeJson(tmp, result);
                            doc["ok"] = true;
                            if (!err) {
                                doc["data"] = tmp.as<JsonVariant>();
                            } else {
                                doc["data"] = result;
                            }
                        } catch (const std::exception &e) {
                            doc["ok"] = false;
                            doc["error"] = e.what();
                        } catch (...) {
                            doc["ok"] = false;
                            doc["error"] = "Unknown error";
                        }

                        String output;
                        serializeJson(doc, output);
                        request->send(200, "application/json", output);
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
