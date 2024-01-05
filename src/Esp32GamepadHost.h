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
            defaultConfig.maxGamepads = MAX_GAMEPADS;
            return defaultConfig;
        };

        void init();
        void init(Config config);
        int getGamepadCount();
        Gamepad* getGamepad(int index);
        Gamepad::Command getCommand();
        Gamepad::Command getCommandForGamepad(int idndex);
        void processTasks();

    private :
        Esp32GamepadHost()
        {
        };

        ~Esp32GamepadHost()
        {
        };

        Gamepad* gamepads[MAX_GAMEPADS];
        int gamepadCount = 0;

        static Esp32GamepadHost *esp32GamepadHostInstance;
};

#endif 