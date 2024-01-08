#include <GamepadAdapterManager.h>
#include <Esp32GamepadHostConfig.h>


GamepadAdapterManager* GamepadAdapterManager::gamepadAdapterManager = nullptr;

GamepadAdapterManager *GamepadAdapterManager::getGamepadAdapterManager()
{
    if (gamepadAdapterManager == nullptr) {
        gamepadAdapterManager = new GamepadAdapterManager();
    }
    return gamepadAdapterManager;
}

GamepadAdapterManager::GamepadAdapterManager()
{   // Register adapters
    registerAdapter(new WiimoteAdapter());
};

GamepadAdapterManager::~GamepadAdapterManager()
{
    for(int i=0 ; i<adapterCount ; i++)
    {
        free(adapters[i]);
        adapters[i] = nullptr;
    }
};

void GamepadAdapterManager::registerAdapter(GamepadAdapter* adapter)
{
    if(adapterCount<GAMEPAD_ADAPTER_NUMBER)
    {
        adapters[adapterCount] = new WiimoteAdapter();
        adapterCount++;
    }
    else
    {
        LOG_ERROR("Could not register adapter %s : max adapter count reached (%d), change GAMEPAD_ADAPTER_NUMBER value", adapter->getName(), adapterCount);
    }
}

GamepadAdapter* GamepadAdapterManager::findAdapter(uint16_t vendorId, uint16_t productId, uint32_t classOfDevice)
{
    for(int i = 0 ; i < adapterCount ; i++)
    {
        if(adapters[i]->match(vendorId,productId,classOfDevice))
        {
            return adapters[i];
        }
    }
    return NULL;
}

