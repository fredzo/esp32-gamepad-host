#ifndef ESP32_GAMEPAD_HOST_H
#define ESP32_GAMEPAD_HOST_H

#include <Arduino.h>
#include <Gamepad.h>
#include <GamepadAdapterManager.h>
#include <Esp32GamepadHostConfig.h>

#define CLASS_OF_DEVICE_UNKNOWN 0

class Esp32GamepadHost
{
    public :
        static Esp32GamepadHost *getEsp32GamepadHost()
        {
            if (esp32GamepadHostInstance == nullptr) {
                esp32GamepadHostInstance = new Esp32GamepadHost();
            }
            return esp32GamepadHostInstance;
        }

        static Config createDefaultConfig()
        {
            Config defaultConfig;
            defaultConfig.btTaskStackDepth = DEFAULT_BT_TASK_SIZE;
            defaultConfig.btTaskPriority = DEFAULT_BT_TASK_PRIORITY;
            defaultConfig.btTaskCoreId = DEFAULT_BT_TASK_CORE_ID;
            defaultConfig.maxGamepads = MAX_CONNECTED_GAMEPADS;
            defaultConfig.filterAccel = DEFAULT_FILTER_ACCEL;
            defaultConfig.filterTouchpad = DEFAULT_FILTER_TOUCHPAD;
            return defaultConfig;
        };

        void init();
        void init(Config config);
        int getGamepadCount();
        Gamepad* getGamepad(int index);
        GamepadCommand* getCommand();
        GamepadCommand* getCommandForGamepad(int index);
        void processTasks();

        Gamepad* addGamepad(bd_addr_t address, Gamepad::State state, uint32_t classOfDevice = CLASS_OF_DEVICE_UNKNOWN, uint16_t vendorId = 0, uint16_t productId = 0, uint8_t pageScanRepetitionMode = 0, uint16_t clockOffset = 0);

        Gamepad* getGamepadForAddress(bd_addr_t addr);
        Gamepad* getGamepadForChannel(uint16_t channel);
        Gamepad* getConnectingGamepad() { return connectingGamepad; };
        Gamepad* askGamepadConnection();
        void     completeConnection(Gamepad* gamepad);
        bool     hasConnectedGamepad();
        void     setLastCommand(GamepadCommand* command) { lastCommand = command; };

    private :
        Esp32GamepadHost()
        {
        };

        ~Esp32GamepadHost()
        {
            for(int i = 0 ; i < gamepadCount ; i++)
            {
                free(gamepads[i]->getCommand());
                free(gamepads[i]);
                gamepads[i] = NULL;
            }
        };

        Gamepad* gamepads[MAX_GAMEPADS];
        GamepadCommand* lastCommand = NULL;
        int gamepadCount = 0;
        int gamepadIndex = 0;
        Gamepad* connectingGamepad = NULL;
        // Reference to adapter manager
        GamepadAdapterManager* adapterManager = NULL;

        static Esp32GamepadHost *esp32GamepadHostInstance;
};

#endif 