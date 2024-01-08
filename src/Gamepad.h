#ifndef GAMEPAD_H
#define GAMEPAD_H
#include <Arduino.h>
#include <Esp32GamepadHostConfig.h>
#include <GamepadCommand.h>
extern "C" {
#include <btstack.h>
}

// Forward declaration for adapter to avoid circular dependency
class GamepadAdapter;

class Gamepad
{
    public :
        enum class State { CONNECTION_REQUESTED, CONNECTING, CONNECTED, DISCONNECTED };

        GamepadCommand* getCommand();

        // Index of this gamepad in the gamepadHost
        int index;

        // Bluetooth data
        bd_addr_t          address;
        uint8_t            pageScanRepetitionMode;
        uint16_t           clockOffset;
        uint32_t           classOfDevice;
        uint16_t           l2capHidControlCid;
        uint16_t           l2capHidInterruptCid;
        // Four output reports
        uint8_t            reportId;
        uint8_t            report[MAX_BT_DATA_SIZE];
        uint16_t           reportLength;

        // For data packet history
        uint8_t            lastPacket[MAX_BT_DATA_SIZE];

        // Bluetooth state
        State              state; 

        bool parseDataPacket(uint8_t * packet, uint16_t packetSize);

        void setAdapter(GamepadAdapter* adapterParam) { adapter = adapterParam; };


    private :
        bool logging = false;
        long last_ms = 0;
        int num_run = 0, num_updates = 0;
        GamepadCommand*  currentCommand = new GamepadCommand();
        GamepadAdapter*  adapter = NULL;

};

#endif 