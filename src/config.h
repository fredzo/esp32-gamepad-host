
#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG 1
#define debug(...) do { if(DEBUG) printf(__VA_ARGS__); } while (0)
#define debug_hexdump(...) do { if(DEBUG) printf_hexdump(__VA_ARGS__); } while (0)

#endif