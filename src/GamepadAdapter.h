#ifndef GAMEPAD_ADAPTER_H
#define GAMEPAD_ADAPTER_H
#include <Arduino.h>
#include <Esp32GamepadHostConfig.h>
#include <GamepadCommand.h>
#include <Gamepad.h>

#define GAMEPAD_ADAPTER_NUMBER  2

class GamepadAdapter
{
    public :
        virtual const char* getName() = 0;
        virtual bool match(uint16_t vendorId, uint16_t deviceId, uint32_t classOfDevice) = 0;
        virtual void parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize) = 0;
};

#endif 