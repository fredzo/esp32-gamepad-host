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

#define UNDEFINED_ADAPTER_NAME          "Undefined Gamepad"

// Report header four output reports
#define OUTPUT_REPORT_HEADER            ((HID_MESSAGE_TYPE_DATA << 4) | HID_REPORT_TYPE_OUTPUT)
// Report header for feature report request
#define FEATURE_REPORT_REQUEST_HEADER   ((HID_MESSAGE_TYPE_GET_REPORT << 4) | HID_REPORT_TYPE_FEATURE)

#define MAX_PLAYERS             6

struct GamepadColor {
    uint8_t red;    
    uint8_t green;    
    uint8_t blue;
    inline GamepadColor( uint8_t ir, uint8_t ig, uint8_t ib)  __attribute__((always_inline)) : red(ir), green(ig), blue(ib) {};
    /// Allow copy construction
    inline GamepadColor(const GamepadColor& rhs) __attribute__((always_inline)) = default;
    /// Allow assignment from one RGB struct to another
    inline GamepadColor& operator= (const GamepadColor& rhs) __attribute__((always_inline)) = default;
};

class Gamepad
{
    public :
        enum ReportType { R_NONE, R_INTERRUPT, R_CONTROL};
        enum class State { CONNECTION_REQUESTED, CONNECTING, CONNECTED, DISCONNECTED };
        static const GamepadColor PURPLE;
        static const GamepadColor CYAN;
        static const GamepadColor RED;
        static const GamepadColor GREEN;
        static const GamepadColor BLUE;
        static const GamepadColor YELLOW;
        static const GamepadColor WHITE;
        static const GamepadColor PLAYER_COLORS[MAX_PLAYERS];

        GamepadCommand* getCommand();
        void setRumble(uint8_t left, uint8_t right);
        void setLed(GamepadColor color);
        void setPlayer(uint8_t playerNumber);

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
        ReportType         reportType;
        uint8_t            reportHeader;
        uint8_t            reportId;
        uint8_t            report[MAX_BT_DATA_SIZE];
        uint16_t           reportLength;

        // For data packet history
        uint8_t            lastPacket[MAX_BT_DATA_SIZE];

        // Bluetooth state
        State              state;

        // State for gamepad adapter state macine
        uint8_t            adapterState = 0;

        bool isLowLevelSecurity();

        bool parseDataPacket(uint8_t * packet, uint16_t packetSize);

        void setAdapter(GamepadAdapter* adapter);

        void connectionComplete();

        void sendReport(ReportType type, uint8_t header, uint8_t reportId, const uint8_t * report = NULL, uint8_t reportLength = 0);

        String getName();
        
        String toString();


        // Led color
        GamepadColor       color = PURPLE;
        // Rumple
        uint8_t            rumbleLeft = 0;
        uint8_t            rumbleRight = 0;

    private :
        GamepadCommand*  currentCommand = new GamepadCommand(this);
        GamepadAdapter*  adapter = NULL;
        String           name = UNDEFINED_ADAPTER_NAME;
        void updateName();

    friend class Esp32GamepadHost;

};

#endif 