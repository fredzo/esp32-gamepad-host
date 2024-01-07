#ifndef ESP32_GAMEPAD_HOST_H
#define ESP32_GAMEPAD_HOST_H
#include <Arduino.h>
#include <Gamepad.h>
#include <Esp32GamepadHostConfig.h>

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
        class Config {
            public :
                uint32_t btTaskStackDepth;
                UBaseType_t btTaskPriority;
                BaseType_t btTaskCoreId;
                int maxGamepads;
        };

        static Config createDefaultConfig()
        {
            Config defaultConfig;
            defaultConfig.btTaskStackDepth = DEFAULT_BT_TASK_SIZE;
            defaultConfig.btTaskPriority = DEFAULT_BT_TASK_PRIORITY;
            defaultConfig.btTaskCoreId = DEFAULT_BT_TASK_CORE_ID;
            defaultConfig.maxGamepads = MAX_CONNECTED_GAMEPADS;
            return defaultConfig;
        };

        void init();
        void init(Config config);
        int getGamepadCount();
        Gamepad* getGamepad(int index);
        Gamepad::Command getCommand();
        Gamepad::Command getCommandForGamepad(int idndex);
        void processTasks();

        Gamepad* addGamepad(bd_addr_t address, uint8_t pageScanRepetitionMode, uint16_t clockOffset, uint32_t classOfDevice, Gamepad::State state);

        Gamepad* getGamepadForAddress(bd_addr_t addr);
        Gamepad* getGamepadForChannel(uint16_t channel);
        Gamepad* getConnectingGamepad() { return connectingGamepad; };
        Gamepad* askGamepadConnection();
        void     finishConnectingGamepad() { connectingGamepad = NULL; };
        bool     hasConnectedGamepad();

    private :
        Esp32GamepadHost()
        {
        };

        ~Esp32GamepadHost()
        {
        };

        Gamepad* gamepads[MAX_GAMEPADS];
        int gamepadCount = 0;
        int gamepadIndex = 0;
        Gamepad* connectingGamepad = NULL;

        static Esp32GamepadHost *esp32GamepadHostInstance;
};

#endif 