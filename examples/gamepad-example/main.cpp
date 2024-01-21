
#include <Arduino.h>
#include <Esp32GamepadHost.h>

/*************************************************************************************************/

static Esp32GamepadHost* gamepadHost;

void setup() 
{
    // Setup
    Config config = Esp32GamepadHost::createDefaultConfig();
    config.filterAccel=true;
    config.filterTouchpad=false;
    //config.maxGamepads = 1;
    //config.btTaskStackDepth = 4*1024;
    gamepadHost = Esp32GamepadHost::getEsp32GamepadHost();
    gamepadHost->init(config);
}

static bool rumbleState = false;
static bool lastA = false;
static bool lastB = false;
static int curColor = 0;
static int curRumble = 0;
void loop() {
    gamepadHost->processTasks();
    GamepadCommand* command = gamepadHost->getCommand();
    if(command && command->hasChanged())
    {
         if(command->buttons[GamepadCommand::N_A] != lastA)
         {
            rumbleState = !rumbleState;
            lastA = !lastA;
            uint8_t rumbleValue = rumbleState ? 0xFF : 0x00;
            gamepadHost->setPlayer(rumbleState ? 1 : 0);
            gamepadHost->setRumble(rumbleValue,rumbleValue);
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
            gamepadHost->setRumble(0xFF,0xFF,1000);
         }
         if(command->buttons[GamepadCommand::S_SHARE])
         {
            gamepadHost->setRumble(0xFF,0xFF,5000);
         }
         if(command->buttons[GamepadCommand::N_X])
         {
            gamepadHost->setLed(Gamepad::RED,3000);
         }
         if(command->buttons[GamepadCommand::N_Y])
         {
            gamepadHost->setLed(Gamepad::GREEN,500);
         }
         int newColor = command->axes[GamepadCommand::AxesLeft::L_VERTICAL];
         if(newColor != curColor)
         {
            curColor = newColor;
            hsv hsvColor;
            hsvColor.h = (((double)(newColor + 0x7F))*360)/0xFF;
            hsvColor.s = 1;
            hsvColor.v = 1;
            command->getGamepad()->setLed(hsv2GamepadColor(hsvColor));
         }
         int newRumble = command->axes[GamepadCommand::AxesRight::R_HORIZONTAL];
         if(newRumble != curRumble)
         {
            curRumble = newRumble;
            command->getGamepad()->setRumble(curRumble < 0 ? (-curRumble-1)*2 : 0, curRumble > 0 ? (curRumble-1)*2 : 0 );
         }
    }
    
}
