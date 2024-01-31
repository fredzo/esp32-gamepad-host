#ifndef GAMEPAD_ADAPTER_H
#define GAMEPAD_ADAPTER_H
#include <Arduino.h>
#include <Esp32GamepadHostConfig.h>
#include <GamepadCommand.h>
#include <Gamepad.h>

#define UINT8_TO_INT(value)     value - 0x7F            
#define UINT10_TO_UINT(msb,lsb) (int16_t)((((uint16_t)(msb & 0x03) << 8) | (uint16_t)lsb))
#define UINT10_TO_INT(msb,lsb)  (int16_t)((((uint16_t)(msb & 0x03) << 8) | (uint16_t)lsb) - 0x1FF)
#define UINT16_TO_UINT(msb,lsb) (int16_t)((((uint16_t)msb << 8) | (uint16_t)lsb))
#define UINT16_TO_INT(msb,lsb)  (int16_t)((((uint16_t)msb << 8) | (uint16_t)lsb) - 0x8000)

class GamepadAdapter
{
    public :
        virtual void setConfig(Config config) { this->config = config; };
        virtual const char* getName() = 0;
        virtual bool match(uint16_t vendorId, uint16_t deviceId, uint32_t classOfDevice) = 0;
        virtual bool isLowLevelSecurity(Gamepad* gamepad);
        virtual bool parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize) = 0;
        virtual void connectionComplete(Gamepad* gamepad);
        virtual void setRumble(Gamepad* gamepad, uint8_t left, uint8_t right) {};
        virtual void setLed(Gamepad* gamepad, GamepadColor color) {};
        virtual void setPlayer(Gamepad* gamepad, uint8_t playerNumber);
    protected :
        bool packetChanged(uint8_t * oldPacket, uint8_t * packet, uint8_t * mask, uint16_t packetSize);
        Config config;
};

#endif 