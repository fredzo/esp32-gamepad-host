
#include "gamepad_manager.h"
#include "terios_t3.h"

#include <stdio.h>

/*************************************************************************************************/

typedef struct t_gpad_keys
{
    uint8_t a;
    uint8_t b;
    uint8_t x;
    uint8_t y;
    uint8_t r1;
    uint8_t l1;
    uint8_t r2;
    uint8_t l2;
    uint8_t r3;
    uint8_t l3;
    uint8_t start;
    uint8_t select;
} t_gpad_keys;

typedef struct t_gpad_dpad
{
    uint8_t up;
    uint8_t right;
    uint8_t down;
    uint8_t left;
} t_gpad_dpad;

typedef struct t_gpad_apad
{
    uint8_t x;
    uint8_t y;
} t_gpad_apad;

typedef struct t_gamepad
{
    t_gpad_keys key;
    t_gpad_dpad dpad;
    t_gpad_apad apad_left;
    t_gpad_apad apad_right;
} t_gamepad;

/*************************************************************************************************/

void gamepad_set_default_state(t_gamepad* gamepad)
{
    gamepad->key.a = 0;
    gamepad->key.b = 0;
    gamepad->key.x = 0;
    gamepad->key.y = 0;
    gamepad->key.r1 = 0;
    gamepad->key.l1 = 0;
    gamepad->key.r2 = 0;
    gamepad->key.l2 = 0;
    gamepad->key.r3 = 0;
    gamepad->key.l3 = 0;
    gamepad->key.select = 0;
    gamepad->key.start = 0;
    gamepad->dpad.up = 0;
    gamepad->dpad.right = 0;
    gamepad->dpad.down = 0;
    gamepad->dpad.left = 0;
    gamepad->apad_left.x = 0x80;
    gamepad->apad_left.y = 0x80;
    gamepad->apad_right.x = 0x80;
    gamepad->apad_right.y = 0x80;
}

void show_gamepad_state(t_gamepad* gamepad)
{
    printf("\nGamepad: ");
    printf("0x%02X 0x%02X ", gamepad->apad_left.x, gamepad->apad_left.y);
    printf("0x%02X 0x%02X ", gamepad->apad_right.x, gamepad->apad_right.y);
    printf("%d %d %d %d ", gamepad->dpad.up, gamepad->dpad.right, gamepad->dpad.down, gamepad->dpad.left);
    printf("%d %d %d %d ", gamepad->key.a, gamepad->key.b, gamepad->key.x, gamepad->key.y);
    printf("%d %d %d %d ", gamepad->key.r1, gamepad->key.l1, gamepad->key.r3, gamepad->key.l3);
    printf("%d %d %d %d\n", gamepad->key.select, gamepad->key.start, gamepad->key.r2, gamepad->key.l2);
}

void parse_hid_to_gamepad(uint8_t* hid_report_data, t_gamepad* gamepad)
{
    // Set gamepad data to default state values
    gamepad_set_default_state(gamepad);

    // Analog Pads data
    gamepad->apad_left.x = hid_report_data[2];
    gamepad->apad_left.y = hid_report_data[3];
    gamepad->apad_right.x = hid_report_data[4];
    gamepad->apad_right.y = hid_report_data[5];

    // Digital Pad data
    if(hid_report_data[6] == T3_DPAD_UP)
        gamepad->dpad.up = 1;
    if(hid_report_data[6] == T3_DPAD_RIGHT)
        gamepad->dpad.right = 1;
    if(hid_report_data[6] == T3_DPAD_DOWN)
        gamepad->dpad.left = 1;
    if(hid_report_data[6] == T3_DPAD_LEFT)
        gamepad->dpad.down = 1;

    // Buttons I data
    if(hid_report_data[7] & T3_BTN_A)
        gamepad->key.a = 1;
    if(hid_report_data[7] & T3_BTN_B)
        gamepad->key.b = 1;
    if(hid_report_data[7] & T3_BTN_X)
        gamepad->key.x = 1;
    if(hid_report_data[7] & T3_BTN_Y)
        gamepad->key.y = 1;
    if(hid_report_data[7] & T3_BTN_R1)
        gamepad->key.r1 = 1;
    if(hid_report_data[7] & T3_BTN_L1)
        gamepad->key.l1 = 1;

    // Buttons II data
    if(hid_report_data[8] & T3_BTN_R2)
        gamepad->key.r2 = 1;
    if(hid_report_data[8] & T3_BTN_L2)
        gamepad->key.l2 = 1;
    if(hid_report_data[8] & T3_BTN_R3)
        gamepad->key.r3 = 1;
    if(hid_report_data[8] & T3_BTN_L3)
        gamepad->key.l3 = 1;
    if(hid_report_data[8] & T3_BTN_START)
        gamepad->key.start = 1;
    if(hid_report_data[8] & T3_BTN_SELECT)
        gamepad->key.select = 1;

    // Triggers
    if(gamepad->key.r2)
        gamepad->key.r2 = hid_report_data[9];
    if(gamepad->key.l2)
        gamepad->key.l2 = hid_report_data[10];
}

void gamepad_handler(uint8_t* hid_report_data, const uint16_t report_len)
{
    static t_gamepad gamepad;

    // Check for expected report size and start bytes
    if(report_len != 12)
        return;
    if((hid_report_data[0] != 0xA1) || (hid_report_data[1] != 0x07))
        return;

    parse_hid_to_gamepad(hid_report_data, &gamepad);

    show_gamepad_state(&gamepad);
}
