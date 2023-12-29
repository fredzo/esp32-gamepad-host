
# esp-bt-terios-t3-receiver

ESP32 Bluetooth HID Host to connect, receive and parse data from Terios T3 Bluetooth Gamepad.

## Notes

- ESP-IDF v3.3, toolchain-xtensa32 v2.5 and BTStack v1.1 is used as requeriments for this project, get all of them by executing "get_requeriments" script:

```bash
./get_requeriments
```

- This project uses BTStack instead ESP-IDF Bluedroid, and needs to be included inside ESP-IDF Components. To automate BTstack add as expected to ESP-IDF, build the project and then remove it from ESP-IDF, just use "_make" script:

```bash
./_make menuconfig
./_make
./_make flash
./_make clean
```

- Remember to enable Bluetooth peripheral and disable Bluedroid component through menuconfig.
