#ifndef WIIMOTE_ADAPTER_H
#define WIIMOTE_ADAPTER_H
#include <Arduino.h>
#include <GamepadAdapter.h>
#include <Gamepad.h>

class WiimoteAdapter : public GamepadAdapter
{
    public :

        const char* getName() { return "Wiimote"; };
        bool match(uint16_t vendorId, uint16_t deviceId, uint32_t classOfDevice);


        bool parseDataPacket(uint8_t * packet, uint16_t packetSize);


    private :


};

#endif 