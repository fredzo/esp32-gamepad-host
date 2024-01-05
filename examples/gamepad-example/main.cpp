
#include <Arduino.h>
#include <Esp32GamepadHost.h>

/*************************************************************************************************/


void setup() 
{
    // Setup
    Esp32GamepadHost::getEsp32GamepadHost()->init();
}


void loop() {
    Esp32GamepadHost::getEsp32GamepadHost()->processTasks();
}
