
#include <Arduino.h>
#include <Esp32GamepadHost.h>

/*************************************************************************************************/

static Esp32GamepadHost* gamepadHost;

void setup() 
{
    // Setup
    gamepadHost = Esp32GamepadHost::getEsp32GamepadHost();
    gamepadHost->init();
}

static bool rumbleState = false;
static bool lastA = false;
static bool lastB = false;
void loop() {
    gamepadHost->processTasks();
    GamepadCommand* command = gamepadHost->getCommand();
    if(command)
    {
         if(command->buttons[GamepadCommand::N_A] != lastA)
         {
            rumbleState = !rumbleState;
            lastA = !lastA;
            uint8_t rumbleValue = rumbleState ? 0xFF : 0x00;
            command->getGamepad()->setPlayer(rumbleState ? 1 : 0);
            command->getGamepad()->setRumble(rumbleValue,rumbleValue);
         }
         else if(command->buttons[GamepadCommand::N_B] != lastB)
         {
            lastB = !lastB;
            if(lastB)
            {
                rumbleState = !rumbleState;
                uint8_t rumbleValue = rumbleState ? 0x7F : 0x00;
                command->getGamepad()->setRumble(rumbleValue,rumbleValue);
                command->getGamepad()->setLed(rumbleState ? Gamepad::RED : Gamepad::GREEN);
            }
         }
         if(command->buttons[GamepadCommand::S_OPTIONS])
         {
            command->getGamepad()->setRumble(0xFF,0xFF,1000);
         }
         if(command->buttons[GamepadCommand::S_SHARE])
         {
            command->getGamepad()->setRumble(0xFF,0xFF,5000);
         }
         if(command->buttons[GamepadCommand::N_X])
         {
            command->getGamepad()->setLed(Gamepad::RED,1000);
         }
         if(command->buttons[GamepadCommand::N_Y])
         {
            command->getGamepad()->setLed(Gamepad::GREEN,1000);
         }
    }
    
}
