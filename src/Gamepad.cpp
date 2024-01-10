#include <Gamepad.h>
#include <GamepadAdapter.h>
#include <Esp32GamepadHostConfig.h>
#include <BluetoothManager.h>

const GamepadColor Gamepad::PURPLE = GamepadColor(0xFF,0x00,0xFF);
const GamepadColor Gamepad::CYAN   = GamepadColor(0x00,0xFF,0xFF);
const GamepadColor Gamepad::RED    = GamepadColor(0xFF,0x00,0x00);
const GamepadColor Gamepad::GREEN  = GamepadColor(0x00,0xFF,0x00);
const GamepadColor Gamepad::BLUE   = GamepadColor(0x00,0x00,0xFF);
const GamepadColor Gamepad::YELLOW = GamepadColor(0xFF,0xFF,0x00);
const GamepadColor Gamepad::WHITE  = GamepadColor(0xFF,0xFF,0xFF);

void Gamepad::setAdapter(GamepadAdapter * adapter)
{
    this->adapter = adapter;
    updateName();
}

bool Gamepad::parseDataPacket(uint8_t * packet, uint16_t packetSize)
{
    if(packetSize < MAX_BT_DATA_SIZE && (memcmp(lastPacket,packet,packetSize)!=0))
    {
        if(adapter)
        {
            adapter->parseDataPacket(this,packet,packetSize);
            LOG_INFO("%s\n",currentCommand->toString().c_str());
        }
        else
        {
            LOG_INFO("No adapter for gamepad %s and packet:\n", toString().c_str());
            LOG_HEXDUMP(packet,packetSize);
        }
        // Store packet content to last packet
        memcpy(lastPacket,packet,packetSize);
        return true;
    }
    else
    {
        return false;
    }
}

// Called on connection complete
void Gamepad::connectionComplete()
{
    state = Gamepad::State::CONNECTED;
    if(adapter)
    {
        adapter->connectionComplete(this);
    }
}

void Gamepad::setRumble(uint8_t left, uint8_t right)
{
    this->rumbleLeft = left;
    this->rumbleRight = right;
    LOG_INFO("Setting rumble to (0x%02X,0x%02X) for gamepad %s.\n",left,right,toString().c_str());
    if(adapter)
    {
        adapter->setRumble(this,left,right);
    }
    else
    {
        LOG_ERROR("No adapter, setRumple() impossible for gamepad %s.\n", toString().c_str());
    }
}

void Gamepad::setLed(GamepadColor color)
{
    this->color = color;
    LOG_INFO("Setting led color to (0x%02X,0x%02X,0x%02X) for gamepad for gamepad %s.\n",this->color.red,this->color.green,this->color.blue,toString().c_str());
    if(adapter)
    {
        adapter->setLed(this,color);
    }
    else
    {
        LOG_ERROR("No adapter, setLed() impossible for gamepad %s.\n", toString().c_str());
    }
}

void Gamepad::setPlayer(uint8_t playerNumber)
{
    if(adapter)
    {
        adapter->setPlayer(this,playerNumber);
    }
    else
    {
        LOG_ERROR("No adapter, setPlayer() impossible for gamepad %s.\n", toString().c_str());
    }
}


void Gamepad::sendOutputReport(uint8_t reportId, const uint8_t * report, uint8_t reportLength)
{
    if(state != Gamepad::State::CONNECTED)
    {
        LOG_ERROR("ERROR : Invalid device state for gamepad %s.\n", toString().c_str());
        return;
    }
    if(reportLength > MAX_BT_DATA_SIZE)
    {
        LOG_ERROR("ERROR : Invalid report length %d, max length is %d for gamepad %s.\n", reportLength, MAX_BT_DATA_SIZE, toString().c_str());
        return;
    }
    this->reportId = reportId;
    memcpy(this->report, report, reportLength);
    this->reportLength = reportLength;
    bluetoothManagerSendOutputReport(this);
}

GamepadCommand* Gamepad::getCommand()
{
    return currentCommand;
}

void Gamepad::updateName()
{
    if(adapter)
    {
        name = adapter->getName();
    }
    else
    {
        name = UNDEFINED_ADAPTER_NAME;
    }
    name = "#" + String(index) + " - " + name;
}

String Gamepad::getName()
{
    return name;
}

String Gamepad::toString()
{
    char buffer[128];
    sprintf(buffer,"%s [%s,%s,led(0X%02X,0X%02X,0X%02X),rumble(0X%02X,0X%02X)]",name.c_str(),bd_addr_to_str(address),state == State::CONNECTED ? "connected" : state == State::DISCONNECTED ? "disconnected" : "connecting", color.red, color.green, color.blue, rumbleLeft,rumbleRight);
    return String(buffer);
}
