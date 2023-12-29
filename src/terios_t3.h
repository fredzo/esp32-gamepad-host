
/*
*  Terios T3 Bluetooth Gamepad Codes
*  
*  HID Frame Example:
*  A1 07 80 80 80 80 88 00 00 00 00 00
*  
*  Bytes 0 y 1: Header (0xA1 0x07)
*  Byte 2: Analog Left Pad X (Center - 0x80; Left - 0x00; Right - 0xff)
*  Byte 3: Analog Left Pad Y (Center - 0x80; Up - 0x00; Down - 0xff)
*  Byte 4: Analog Right Pad X (Center - 0x80; Left - 0x00; Right - 0xff)
*  Byte 5: Analog Right Pad Y (Center - 0x80; Up - 0x00; Down - 0xff)
*  Byte 6: Digital Pad (None - 0x88; up - 0x00; right - 0x02; down - 0x04; left - 0x06)
*  Byte 7: Buttons I  (None - 0x00; L1 - 0x40; R1 - 0x80; A - 0x01; B - 0x02; X - 0x08; Y - 0x10)
*  Byte 8: Buttons II (None - 0x00; L2 - 0x01; R2 - 0x02; R3 - 0x40; L3 - 0x20; Start - 0x08; Select - 0x04)
*  Byte 9: R2 Analog Value (Release: 0x00; Pressed 0xff)
*  Byte 10: L2 Analog Value (Release: 0x00; Pressed 0xff)
*  Byte 11: Unknown
*/

#define T3_BTN_NONE         0x00
#define T3_BTN_A            0x01
#define T3_BTN_B            0x02
#define T3_BTN_X            0x08
#define T3_BTN_Y            0x10
#define T3_BTN_R1           0x80
#define T3_BTN_R2           0x02
#define T3_BTN_R3           0x40
#define T3_BTN_L1           0x40
#define T3_BTN_L2           0x01
#define T3_BTN_L3           0x20
#define T3_BTN_START        0x08
#define T3_BTN_SELECT       0x04

#define T3_DPAD_NONE        0x88
#define T3_DPAD_UP          0x00
#define T3_DPAD_RIGHT       0x02
#define T3_DPAD_DOWN        0x04
#define T3_DPAD_LEFT        0x06
