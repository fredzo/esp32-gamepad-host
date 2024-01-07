#include <Esp32GamepadHost.h>
#include <BluetoothManager.h>


Esp32GamepadHost* Esp32GamepadHost::esp32GamepadHostInstance = nullptr;


void btTask(void* args)
{
    int maxGamepads = ((int *)args)[0];
    LOG_INFO("Esp32GamepadHost bluetooth task running on core %d\n",xPortGetCoreID());
    btstackInit(maxGamepads+1); // We need to conenxions per gamepad + 1 for gap inquiery
    btstackRun();
}

void Esp32GamepadHost::init()
{
    init(createDefaultConfig());
}
void Esp32GamepadHost::init(Config config)
{
    int args[] = {config.maxGamepads};
    xTaskCreatePinnedToCore(btTask, "esp32GamepadHostBtTask",config.btTaskStackDepth, args, config.btTaskPriority, NULL, config.btTaskCoreId);
}

int Esp32GamepadHost::getGamepadCount()
{
    return gamepadCount;
}

Gamepad* Esp32GamepadHost::getGamepad(int index)
{
    return NULL;
}

Gamepad::Command Esp32GamepadHost::getCommandForGamepad(int idndex)
{
    Gamepad::Command result = Gamepad::NO_COMMAND;
    return result;
}

Gamepad::Command Esp32GamepadHost::getCommand()
{
    Gamepad::Command result = Gamepad::NO_COMMAND;
    return result;
}

void Esp32GamepadHost::processTasks()
{
    maybeRumble();
}

Gamepad* Esp32GamepadHost::addGamepad(bd_addr_t address, uint8_t pageScanRepetitionMode, uint16_t clockOffset, uint32_t classOfDevice, Gamepad::State state)
{
    Gamepad* gamepad = new Gamepad();
    memcpy(gamepad->address, address, 6);
    gamepad->pageScanRepetitionMode = pageScanRepetitionMode;
    gamepad->clockOffset = clockOffset;
    gamepad->classOfDevice = classOfDevice;
    gamepad->state = state;
    gamepad->index = gamepadIndex;
    gamepads[gamepadIndex] = gamepad;
    if(state == Gamepad::State::CONNECTING && connectingGamepad == NULL)
    {   // New connecting device
        connectingGamepad = gamepad;
    }
    gamepadIndex++;
    if(gamepadIndex >= MAX_GAMEPADS)
    {   // Cirecle back to 0
        gamepadIndex = 0;
    }
    if(gamepadCount < MAX_GAMEPADS)
    {
        gamepadCount++;
    }
    return gamepad;
}


Gamepad* Esp32GamepadHost::getGamepadForAddress(bd_addr_t addr)
{
    for (int j=0; j < gamepadCount; j++){
        if (bd_addr_cmp(addr, gamepads[j]->address) == 0){
            return gamepads[j];
        }
    }
    return NULL;
}

Gamepad* Esp32GamepadHost::getGamepadForChannel(uint16_t channel)
{
    if(channel > 0)
    {
        for (int j=0; j< gamepadCount; j++){
            if (gamepads[j]->l2capHidControlCid == channel || gamepads[j]->l2capHidInterruptCid == channel){
                return gamepads[j];
            }
        }
    }
    return NULL;
}

Gamepad* Esp32GamepadHost::askGamepadConnection()
{
    // Prevent simmultaneous connection process
    if(connectingGamepad != NULL) return NULL;
    int j;
    for (j=0; j < gamepadCount; j++){
        if (gamepads[j]->state == Gamepad::State::CONNECTION_REQUESTED)
        {   
            connectingGamepad = gamepads[j];
            connectingGamepad->state = Gamepad::State::CONNECTING;
            return connectingGamepad;
        }
    }
    return NULL;
}

bool Esp32GamepadHost::hasConnectedGamepad()
{
    for (int j=0; j < gamepadCount; j++){
        if (gamepads[j]->state == Gamepad::State::CONNECTED){
            return true;
        }
    }
    return false;
}
