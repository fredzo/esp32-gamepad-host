#ifndef GENERIC_ADAPTER_H
#define GENERIC_ADAPTER_H
#include <Arduino.h>
#include <GamepadAdapter.h>
#include <Gamepad.h>

#define CLASS_OF_DEVICE_GENERIC_ADAPTER 0x002508


class GenericAdapter : public GamepadAdapter
{
    public :

        const char* getName() { return "Generic Gamepad"; };
        
        bool match(uint16_t vendorId, uint16_t productId, uint32_t classOfDevice)
        {
            return ((classOfDevice == CLASS_OF_DEVICE_GENERIC_ADAPTER) && (vendorId == 0) && (productId == 0));
        };

        bool parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize)
        {
            LOG_INFO("%s received packet:\n", gamepad->toString().c_str());
            LOG_HEXDUMP(packet,packetSize);
            return true;
        };
};

#endif 