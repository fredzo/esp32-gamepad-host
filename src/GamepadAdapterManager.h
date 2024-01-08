#ifndef GAMEPAD_ADAPTER_MANAGER_H
#define GAMEPAD_ADAPTER_MANAGER_H
#include <Arduino.h>
#include <GamepadAdapter.h>
#include <Esp32GamepadHostConfig.h>

// Adapters
#include <adapters/WiimoteAdapter.h>

#define GAMEPAD_ADAPTER_NUMBER  2

class GamepadAdapterManager
{
    public :
        static GamepadAdapterManager *getGamepadAdapterManager();

        GamepadAdapter* findAdapter(uint16_t vendorId, uint16_t productId, uint32_t classOfDevice);

    private :
        GamepadAdapter* adapters[GAMEPAD_ADAPTER_NUMBER];
        int adapterCount = 0;
        GamepadAdapterManager();
        ~GamepadAdapterManager();

        void registerAdapter(GamepadAdapter* adapter);

        static GamepadAdapterManager *gamepadAdapterManager;

};
#endif 