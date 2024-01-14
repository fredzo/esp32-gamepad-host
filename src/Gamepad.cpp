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

const GamepadColor Gamepad::PLAYER_COLORS[] = { 
    Gamepad::PURPLE,
    Gamepad::CYAN,
    Gamepad::RED,
    Gamepad::GREEN,
    Gamepad::BLUE,
    Gamepad::YELLOW
};

void Gamepad::setAdapter(GamepadAdapter * adapter)
{
    this->adapter = adapter;
    updateName();
}

bool Gamepad::isLowLevelSecurity()
{
    if(adapter)
    {
        return adapter->isLowLevelSecurity(this);
    }
    return false;
}

bool Gamepad::parseDataPacket(uint8_t * packet, uint16_t packetSize)
{
    bool parsed = false;
    if(packetSize < MAX_BT_DATA_SIZE && (memcmp(lastPacket,packet,packetSize)!=0))
    {
        if(adapter)
        {
            if(adapter->parseDataPacket(this,packet,packetSize))
            {
                LOG_INFO("%s\n",currentCommand->toString().c_str());
                parsed = true;
            }
        }
        else
        {
            LOG_INFO("No adapter for gamepad %s and packet:\n", toString().c_str());
            LOG_HEXDUMP(packet,packetSize);
        }
        // Store packet content to last packet
        memcpy(lastPacket,packet,packetSize);
    }
    return parsed;
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

void Gamepad::setRumble(uint8_t left, uint8_t right, uint16_t duration)
{
    this->rumbleLeft = left;
    this->rumbleRight = right;
    LOG_INFO("Setting rumble to (0x%02X,0x%02X) for gamepad %s.\n",left,right,toString().c_str());
    if(adapter)
    {
        adapter->setRumble(this,left,right);
        if(duration > 0)
        {
            rumbleTimer = true;
            rumbleEndTime = millis() + duration;
        }
    }
    else
    {
        LOG_ERROR("No adapter, setRumple() impossible for gamepad %s.\n", toString().c_str());
    }
}

void Gamepad::setLed(GamepadColor color, uint16_t fadeTime)
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

void Gamepad::processTasks()
{
    if(rumbleTimer)
    {
        unsigned long now = millis();
        if(now >= rumbleEndTime)
        {   // Stop rumble
            rumbleTimer = false;
            if(adapter) adapter->setRumble(this,0,0);
        }
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


void Gamepad::sendReport(ReportType type, uint8_t header, uint8_t reportId, const uint8_t * report, uint8_t reportLength)
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
    this->reportType = type;
    this->reportHeader = header;
    this->reportId = reportId;
    if(report && (reportLength > 0)) memcpy(this->report, report, reportLength);
    this->reportLength = reportLength;
    bluetoothManagerSendReport(this);
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
