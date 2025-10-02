#pragma once

#define ROUTER(path, setup) Router* path = new Router(path, setup)
#define GET(endpoint, handler) [](Router* r){ r->get(endpoint, handler); }
#define POST(endpoint, handler) [](Router* r){ r->post(endpoint, handler); }
#define USE_MW(mw) [](Router* r){ r->addMiddleware(mw); }
#define USE_GUARD(guard) [](Router* r){ r->addGuard(guard); }
#define USE_ENDPOINT_GUARD(path, guard) [](Router* r){ r->addGuardToEndpoint(path, guard); }
