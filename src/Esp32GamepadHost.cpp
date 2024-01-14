#include <Esp32GamepadHost.h>
#include <BluetoothManager.h>


Esp32GamepadHost* Esp32GamepadHost::esp32GamepadHostInstance = nullptr;


void btTask(void* args)
{
    int maxGamepads = ((int *)args)[0];
    LOG_INFO("Esp32GamepadHost bluetooth task running on core %d\n",xPortGetCoreID());
    bluetoothManagerInit(maxGamepads+1); // We need to conenxions per gamepad + 1 for gap inquiery
    bluetoothManagerRun();
}

void Esp32GamepadHost::init()
{
    init(createDefaultConfig());
}
void Esp32GamepadHost::init(Config config)
{   // TODO add filters to configuration
    adapterManager = GamepadAdapterManager::getGamepadAdapterManager();
    int args[] = {config.maxGamepads};
    xTaskCreatePinnedToCore(btTask, "esp32GamepadHostBtTask",config.btTaskStackDepth, args, config.btTaskPriority, NULL, config.btTaskCoreId);
}

int Esp32GamepadHost::getGamepadCount()
{
    return gamepadCount;
}

Gamepad* Esp32GamepadHost::getGamepad(int index)
{
    if(index >= 0 && index < gamepadCount)
    {
        return gamepads[index];
    }
    else
    {
        LOG_ERROR("Invalid gamepad index %d, gamepadCount = %d.\n",index,gamepadCount);
        return NULL;
    }
}

GamepadCommand* Esp32GamepadHost::getCommandForGamepad(int index)
{
    if(index >= 0 && index < gamepadCount)
    {
        return gamepads[index]->getCommand();
    }
    else
    {
        LOG_ERROR("Invalid gamepad index %d, gamepadCount = %d.\n",index,gamepadCount);
        return NULL;
    }
}

GamepadCommand* Esp32GamepadHost::getCommand()
{
    for(int i = 0 ; i < gamepadCount ; i++)
    {
        GamepadCommand* command = gamepads[i]->getCommand();
        if(command->hasChanged())
        {
            return command;
        }
    }
    return NULL;
}

void Esp32GamepadHost::processTasks()
{   // Handle rumble timer and led fading
    for (int j=0; j < gamepadCount; j++){
        return gamepads[j]->processTasks();
    }
}

Gamepad* Esp32GamepadHost::addGamepad(bd_addr_t address, Gamepad::State state, uint32_t classOfDevice, uint16_t vendorId, uint16_t productId, uint8_t pageScanRepetitionMode, uint16_t clockOffset)
{
    Gamepad* gamepad = new Gamepad();
    memcpy(gamepad->address, address, 6);
    gamepad->pageScanRepetitionMode = pageScanRepetitionMode;
    gamepad->clockOffset = clockOffset;
    gamepad->classOfDevice = classOfDevice;
    gamepad->state = state;
    gamepad->index = gamepadIndex;
    if(vendorId != 0 || productId != 0 || classOfDevice != 0)
    {   // Try and find an adapter
        GamepadAdapter* adapter = adapterManager->findAdapter(vendorId,productId,classOfDevice);
        // Set adapter alse updates the name
        gamepad->setAdapter(adapter);
    }
    else
    {
        // Update the name with the index
        gamepad->updateName();
        // We need a sdp report to get informations about the device
        // TODO
    }

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

void  Esp32GamepadHost::completeConnection(Gamepad* gamepad) 
{   // Connection successfull
    gamepad->connectionComplete();
    connectingGamepad = NULL; 
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
