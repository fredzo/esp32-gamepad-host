
#ifndef ESP32_GAMEPAD_HOST_CONFIG_H
#define ESP32_GAMEPAD_HOST_CONFIG_H


#ifdef ESP32_GAMEPAD_HOST_LOG_DEBUG
    #define LOG_HEXDUMP(...)  printf_hexdump(__VA_ARGS__)
    #define LOG_DEBUG(...)  printf(__VA_ARGS__)
    #define LOG_INFO(...)  printf(__VA_ARGS__)
    #define LOG_WARN(...)  printf(__VA_ARGS__)
    #define LOG_ERROR(...)  printf(__VA_ARGS__)
#else
    #define LOG_HEXDUMP(...)  (void)(0)
    #define LOG_DEBUG(...) (void)(0)
    #ifdef ESP32_GAMEPAD_HOST_LOG_INFO
        #define LOG_INFO(...)  printf(__VA_ARGS__)
        #define LOG_WRRN(...)  printf(__VA_ARGS__)
        #define LOG_ERROR(...)  printf(__VA_ARGS__)
    #else
        #define LOG_INFO(...) (void)(0)
        #ifdef ESP32_GAMEPAD_HOST_LOG_WARN
            #define LOG_WARN(...)  printf(__VA_ARGS__)
            #define LOG_ERROR(...)  printf(__VA_ARGS__)
        #else
                #define LOG_WARN(...) (void)(0)
                #if ESP32_GAMEPAD_HOST_LOG_ERROR
                    #define LOG_ERROR(...)  printf(__VA_ARGS__)
                #else
                    #define LOG_ERROR(...) (void)(0)
                #endif
        #endif
    #endif
#endif

// Default values for lib config
#define DEFAULT_BT_TASK_SIZE        4*1024
#define DEFAULT_BT_TASK_PRIORITY    1
#define DEFAULT_BT_TASK_CORE_ID     0
// Filters
#define DEFAULT_FILTER_ACCEL        1
#define DEFAULT_FILTER_TOUCHPAD     0

// Max size for data packets
#define MAX_BT_DATA_SIZE            128

// Max number of connected gamepads
#define MAX_CONNECTED_GAMEPADS      6

// Max number of listed gamepads (regardless of their connection status)
#define MAX_GAMEPADS                16

// Bluetooth class of devices
#define CLASS_OF_DEVICE_GAMEPAD_START  0x002500
#define CLASS_OF_DEVICE_GAMEPAD_END    0x0025FF


class Config {
    public :
        // Bluetooth config
        uint32_t btTaskStackDepth;
        UBaseType_t btTaskPriority;
        BaseType_t btTaskCoreId;
        int maxGamepads;
        // Filters
        bool filterAccel;
        bool filterTouchpad;
};


#endif