#include "ServerManager.h"
#include <typeinfo>

ServerManager::ServerManager(uint16_t port) : _server(port) {}

void ServerManager::use(Middleware* mw) {
    _middlewares.push_back(mw);
    // Log middleware class name
    Serial.print("🟢 Middleware added: ");
    Serial.println(typeid(*mw).name());
}

void ServerManager::addRouter(Router* router) {
    _routers.push_back(router);

    // Log router class name and base path if exists
    Serial.print("📦 Router mounted: ");
    Serial.print(typeid(*router).name());
    if (router->basePath() != "") {
        Serial.print(" | Base path: ");
        Serial.print(router->basePath());
    }
    Serial.println();

    // Log all GET endpoints
    for (auto route : router->getEndpoints()) {
        Serial.print("  🟢 GET ");
        Serial.println(route.path);
    }

    // Log all POST endpoints
    for (auto route : router->postEndpoints()) {
        Serial.print("  🟢 POST ");
        Serial.println(route.path);
    }
}

void ServerManager::begin() {
    // Register all routes
    for (auto router : _routers) {
        for (auto route : router->getEndpoints()) {
            _server.on(route.path.c_str(), HTTP_GET, [this, route](AsyncWebServerRequest* request){
                size_t i = 0;
                std::function<void()> runMiddlewares;
                runMiddlewares = [&]() {
                    if (i < _middlewares.size()) {
                        Middleware* mw = _middlewares[i++];
                        mw->handle(request, runMiddlewares);
                    } else {
                        route.handler(request);
                    }
                };
                runMiddlewares();
            });
        }

        for (auto route : router->postEndpoints()) {
            _server.on(route.path.c_str(), HTTP_POST, [this, route](AsyncWebServerRequest* request){
                size_t i = 0;
                std::function<void()> runMiddlewares;
                runMiddlewares = [&]() {
                    if (i < _middlewares.size()) {
                        Middleware* mw = _middlewares[i++];
                        mw->handle(request, runMiddlewares);
                    } else {
                        route.handler(request);
                    }
                };
                runMiddlewares();
            });
        }
    }

    _server.begin();
    Serial.println("✅ Webserver started!");
}
