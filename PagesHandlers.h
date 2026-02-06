#ifndef PAGES_HANDLERS_H
#define PAGES_HANDLERS_H

#include <ESPAsyncWebServer.h>
#include "Garland.h"
//#include "Config.h" // Try to keep if needed, or remove if unused. User said "same as MySketch", but MySketch uses Config. 
// Assuming for now I can just pass Garland ref directly and maybe Config if needed for WiFi creds, but WiFi is in main sketch usually or Config.
// Let's stick to the plan: PagesHandlers logic mostly.

class PagesHandlers {
public:
    PagesHandlers(Garland& garland);
    void initPagesHandlers(AsyncWebServer& webServer);

private:
    Garland& _garland;

    void setupHomePageHandler(AsyncWebServer& webServer);
    void setupApiStatusHandler(AsyncWebServer& webServer);
    void setupApiSetHandler(AsyncWebServer& webServer);
};

#endif // PAGES_HANDLERS_H
