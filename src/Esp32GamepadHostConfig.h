
#ifndef ESP32_GAMEPAD_HOST_CONFIG_H
#define ESP32_GAMEPAD_HOST_CONFIG_H


#ifdef ESP32_GAMEPAD_HOST_LOG_DEBUG
    #define LOG_DEBUG(format, ...)  printf(__VA_ARGS__)
    #define LOG_INFO(format, ...)  printf(__VA_ARGS__)
    #define LOG_WARN(format, ...)  printf(__VA_ARGS__)
    #define LOG_ERROR(format, ...)  printf(__VA_ARGS__)
#else
    #define LOG_DEBUG(...) (void)(0)
    #ifdef ESP32_GAMEPAD_HOST_LOG_INFO
        #define LOG_INFO(format, ...)  printf(__VA_ARGS__)
        #define LOG_WRRN(format, ...)  printf(__VA_ARGS__)
        #define LOG_ERROR(format, ...)  printf(__VA_ARGS__)
    #else
        #define LOG_INFO(...) (void)(0)
        #ifdef ESP32_GAMEPAD_HOST_LOG_WARN
            #define LOG_WARN(format, ...)  printf(__VA_ARGS__)
            #define LOG_ERROR(format, ...)  printf(__VA_ARGS__)
        #else
                #define LOG_WARN(...) (void)(0)
                #if ESP32_GAMEPAD_HOST_LOG_ERROR
                    #define LOG_ERROR(format, ...)  printf(__VA_ARGS__)
                #else
                    #define LOG_ERROR(...) (void)(0)
                #endif
        #endif
    #endif
#endif

// Defaimt values for lib config
#define DEFAULT_BT_TASK_SIZE        5*1024
#define DEFAULT_BT_TASK_PRIORITY    1
#define DEFAULT_BT_TASK_CORE_ID     0

// Max number of connected gamepads
#define MAX_GAMEPADS                4

// Bluetooth class of devices
#define CLASS_OF_DEVICE_GAMEPAD_START  0x002500
#define CLASS_OF_DEVICE_GAMEPAD_END    0x0025FF
#define CLASS_OF_DEVICE_WIIMOTE        0x002504

#endif