#include <Gamepad.h>
#include <GamepadAdapter.h>
#include <Esp32GamepadHostConfig.h>
#include <BluetoothManager.h>


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
            LOG_INFO("No adapter for packet:\n");
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
    if(adapter)
    {
        adapter->setRumble(this,left,right);
    }
    else
    {
        LOG_INFO("No adapter, rumple impossible for gamepad #%d:\n",index);
    }
}


void Gamepad::sendOutputReport(uint8_t reportId, const uint8_t * report, uint8_t reportLength)
{
    if(state != Gamepad::State::CONNECTED)
    {
        LOG_ERROR("ERROR : Invalid device state for device with index %d.\n", index);
        return;
    }
    if(reportLength > MAX_BT_DATA_SIZE)
    {
        LOG_ERROR("ERROR : Invalid report length %d, max length is %d.\n", reportLength, MAX_BT_DATA_SIZE);
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
