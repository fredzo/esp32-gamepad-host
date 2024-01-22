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

#define FADE_STEPS              0xFF

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

/// Check if two GamepadColor objects have the same color data
inline __attribute__((always_inline)) bool operator== (const GamepadColor& lhs, const GamepadColor& rhs)
{
    return (lhs.red == rhs.red) && (lhs.green == rhs.green) && (lhs.blue == rhs.blue);
};

/// Check if two GamepadColor objects do *not* have the same color data
inline __attribute__((always_inline)) bool operator!= (const GamepadColor& lhs, const GamepadColor& rhs)
{
    return !(lhs == rhs);
};

// For color fading : HSV -> RGB and RGB -> HSV conversion (from https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both)
typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

hsv   gamepadColor2hsv(GamepadColor in);
GamepadColor   hsv2GamepadColor(hsv in);
hsv   rgb2hsv(rgb in);
rgb   hsv2rgb(hsv in);


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

        class Report
        {
            public :
                ReportType         reportType = R_NONE;
                uint8_t            reportHeader;
                uint8_t            reportId;
                uint8_t            reportData[MAX_BT_DATA_SIZE];
                uint16_t           reportLength = 0;
                uint16_t           reportCid;
        };

        GamepadCommand* getCommand();
        void setRumble(uint8_t left, uint8_t right, uint16_t duration = 0);
        void setLed(GamepadColor color, uint16_t fadeTime = 0);
        void setPlayer(uint8_t playerNumber);

        // Bluetooth state
        State              state;

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
        SemaphoreHandle_t       reportAccessMutex = xSemaphoreCreateMutex();
        Report                  report;
        btstack_context_callback_registration_t  sendReportCallback;

        // For data packet history
        uint8_t            lastPacket[MAX_BT_DATA_SIZE];

        // State for gamepad adapter state macine
        uint8_t            adapterState = 0;

        bool isLowLevelSecurity();

        bool parseDataPacket(uint8_t * packet, uint16_t packetSize);

        void setAdapter(GamepadAdapter* adapter);

        void connectionComplete();

        void connectionLost();

        void sendReport(ReportType type, uint8_t header, uint8_t reportId, const uint8_t * report = NULL, uint8_t reportLength = 0);

        void processTasks();

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
        // Rumble timer management
        bool rumbleTimer = false;
        unsigned long rumbleEndTime;
        // Color fading management
        bool fadingTimer = false;
        unsigned long fadeStartTime;
        unsigned long fadeNextStepTime;
        uint16_t fadingStepDuration;
        hsv fromColor;
        hsv toColor;
        GamepadColor toColorRgb = PURPLE;

        void updateName();

    friend class Esp32GamepadHost;

};

#endif 