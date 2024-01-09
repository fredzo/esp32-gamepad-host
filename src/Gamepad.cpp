#include <Gamepad.h>
#include <GamepadAdapter.h>


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

GamepadCommand* Gamepad::getCommand()
{
    return currentCommand;
}
