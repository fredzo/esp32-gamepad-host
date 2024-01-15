#include <GamepadAdapterManager.h>
#include <Esp32GamepadHostConfig.h>

// Adapters
#include <adapters/WiimoteAdapter.h>
#include <adapters/DS4Adapter.h>


GamepadAdapterManager* GamepadAdapterManager::gamepadAdapterManager = nullptr;

GamepadAdapterManager *GamepadAdapterManager::getGamepadAdapterManager(Config config)
{
    if (gamepadAdapterManager == nullptr) {
        gamepadAdapterManager = new GamepadAdapterManager(config);
    }
    return gamepadAdapterManager;
}

GamepadAdapterManager::GamepadAdapterManager(Config config)
{   
    this->config = config;
    // Register adapters
    registerAdapter(new WiimoteAdapter());
    registerAdapter(new DS4Adapter());
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
        adapter->setConfig(config);
        adapters[adapterCount] = adapter;
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

