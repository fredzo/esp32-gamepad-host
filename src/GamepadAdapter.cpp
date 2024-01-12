#include <GamepadAdapter.h>

void GamepadAdapter::setPlayer(Gamepad* gamepad, uint8_t playerNumber)
{
    gamepad->setLed(Gamepad::PLAYER_COLORS[playerNumber]);
}

void GamepadAdapter::connectionComplete(Gamepad* gamepad)
{   // On connection complete, set player led
    gamepad->setPlayer(((gamepad->index) % MAX_PLAYERS));
}


bool GamepadAdapter::packetChanged(uint8_t * oldPacket, uint8_t * packet, uint8_t * mask, uint16_t packetSize)
{
    for(int i = 0; i < packetSize ; i++)
    {
        uint8_t oldValue = oldPacket[i] & mask[i];
        uint8_t newValue = packet[i] & mask[i];
        if(oldValue != newValue)
        {
            LOG_INFO("Packet changed at index %d : 0x%02x / 0x%02x\n",i,oldValue,newValue);
            return true;
        }
    }
    return false;
}
