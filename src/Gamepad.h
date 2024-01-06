#ifndef GAMEPAD_H
#define GAMEPAD_H
#include <Arduino.h>
extern "C" {
#include <btstack.h>
}

class Gamepad
{
    public :
        class Command {
            public :
                Command() {};       // Wiimote in horizontal orientation
                bool a = false;     // Mapped to "1" button
                bool b = false;     // Mapped to "2" button
                bool up = false;    // Reversed for horizontale position
                bool down = false;  // Reversed for horizontale position
                bool left = false;  // Reversed for horizontale position
                bool right = false; // Reversed for horizontale position
                bool plus = false;  
                bool minus = false;
                bool menu = false;  // Maped to wiimote "A" button
                bool trig = false;  // Maped to wiimote trigger button
                bool home = false;  // Mapped to home button
                bool hasCommand();
        };
        static Command NO_COMMAND;
        
        enum class State { CONNECTION_REQUESTED, CONNECTING, CONNECTED, DISCONNECTED };

        Command getCommand();

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
        const uint8_t *    report;
        uint16_t           reportLength;

        // Bluetooth state
        State              state; 


    private :
        bool logging = false;
        long last_ms = 0;
        int num_run = 0, num_updates = 0;
        Command  currentCommand = NO_COMMAND;


};

#endif 