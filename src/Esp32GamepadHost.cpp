#include <Esp32GamepadHost.h>
#include <BluetoothManager.h>


Esp32GamepadHost* Esp32GamepadHost::esp32GamepadHostInstance = nullptr;


// dual core mode runs emulator on comms core
void btTask(void* args)
{
    int maxGamepads = ((int *)args)[0];
    LOG_INFO("Esp32GamepadHost bluetooth task running on core %d\n",xPortGetCoreID());
    //btstackInit(config->maxGamepads*2); // We need to conenxions per gamepad
    btstackInit(maxGamepads); // We need to conenxions per gamepad
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

