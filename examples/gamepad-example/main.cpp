
#include <Arduino.h>
#include "config.h"
#include "bt_manager.h"
#include "gap_inquiry.h"

/*************************************************************************************************/


// dual core mode runs emulator on comms core
void emu_task(void* arg)
{
    printf("emu_task running on core %d\n",xPortGetCoreID());
    btstack_main(0, 0);
    //btstack_main_new(0, 0);
    //btstack_gap_inquiry();
    btstack_run();
}


void setup() 
{
    // Setup

    xTaskCreatePinnedToCore(emu_task, "emu_task",5*1024, NULL, 0, NULL, 0); // nofrendo needs 5k word stack, start on core 0
    //xTaskCreate(emu_task, "emu_task",5*1024, NULL, 0, NULL); // nofrendo needs 5k word stack, start on core 0
}


void loop() {
    // Enter run loop (forever)
    maybeRumble();
}
