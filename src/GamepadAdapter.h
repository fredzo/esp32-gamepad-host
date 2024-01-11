#ifndef GAMEPAD_ADAPTER_H
#define GAMEPAD_ADAPTER_H
#include <Arduino.h>
#include <Esp32GamepadHostConfig.h>
#include <GamepadCommand.h>
#include <Gamepad.h>

#define GAMEPAD_ADAPTER_NUMBER  2

#define MAX_PLAYERS             6

#define UINT8_TO_INT(value)     value - 0x7F            

class GamepadAdapter
{
    public :
        const static GamepadColor PLAYER_COLORS[MAX_PLAYERS];

        virtual const char* getName() = 0;
        virtual bool match(uint16_t vendorId, uint16_t deviceId, uint32_t classOfDevice) = 0;
        virtual void parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize) = 0;
        virtual void connectionComplete(Gamepad* gamepad);
        virtual void setRumble(Gamepad* gamepad, uint8_t left, uint8_t right) {};
        virtual void setLed(Gamepad* gamepad, GamepadColor color) {};
        virtual void setPlayer(Gamepad* gamepad, uint8_t playerNumber);
};

#endif 