#include "ServerManager.h"

ServerManager::ServerManager(uint16_t port) : _server(port) {}

void ServerManager::use(Middleware* mw) {
    _middlewares.push_back(mw);
}

void ServerManager::addRouter(Router* router) {
    _routers.push_back(router);
}

void ServerManager::begin() {
    // Register all routes
    for (auto router : _routers) {
        for (auto route : router->getEndpoints()) {
            _server.on(route.path.c_str(), HTTP_GET, [this, route](AsyncWebServerRequest* request){
                size_t i = 0;
                std::function<void()> next = [&]() {};
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
                std::function<void()> next = [&]() {};
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
