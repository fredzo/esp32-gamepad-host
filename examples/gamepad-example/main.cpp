
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
            printf("Before Rumble with state %d.\n",rumbleState);
            uint8_t rumbleValue = rumbleState ? 0xFF : 0x00;
            command->getGamepad()->setRumble(rumbleValue,rumbleValue);
            command->getGamepad()->setLed(rumbleState ? Gamepad::WHITE : Gamepad::YELLOW);
            printf("After Rumble !\n");
         }
         else if(command->buttons[GamepadCommand::N_B] != lastB)
         {
            lastB = !lastB;
            if(lastB)
            {
                rumbleState = !rumbleState;
                printf("Before Rumble with state %d.\n",rumbleState);
                uint8_t rumbleValue = rumbleState ? 0x7F : 0x00;
                command->getGamepad()->setRumble(rumbleValue,rumbleValue);
                command->getGamepad()->setLed(rumbleState ? Gamepad::PURPLE : Gamepad::BLUE);
                printf("After Rumble !\n");
            }
         }
    }
    
}
