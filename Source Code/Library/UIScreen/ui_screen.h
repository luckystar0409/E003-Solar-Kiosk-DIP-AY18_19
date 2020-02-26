#ifndef UI_SCREEN_H_INCLUDED
#define UI_SCREEN_H_INCLUDED

#include "Arduino.h"
#include "U8glib.h"
#include "Keypad.h"
#include "string.h"
#include "stdlib.h"

#define SCREEN U8GLIB_SSD1306_128X64
#define STAGES 4
#define STATES 3 //TEMP, RATE, HOLDTIME
#define DATAROWSIZE STAGES*STATES+2
#define DATACOLSIZE 4

typedef enum PARAMETER_SELECT
{
    START_TEMP,         //0
    STAGE1_TEMP,        //1
    STAGE1_RATE,        //2
    STAGE1_HOLDTIME,    //3
    STAGE2_TEMP,
    STAGE2_RATE,
    STAGE2_HOLDTIME,
    STAGE3_TEMP,
    STAGE3_RATE,
    STAGE3_HOLDTIME,
    STAGE4_TEMP,
    STAGE4_RATE,
    STAGE4_HOLDTIME,
    PASSIVE_COOL
} param_t;


class UIScreen{
    unsigned long nextRSData;
    unsigned long nextKeypadCheck;
    int defaults[DATAROWSIZE] = {5, 300, 120, 300, 120, 30, 120, 30, 11, 223, 3, 12, 13, 100};
    char data[DATACOLSIZE][DATAROWSIZE];
    char recentSelection;
    char isCancel;
    char menuInput[4];
    char keyInValue(param_t row, Keypad keypad);
    void computeHoldTime(void);

public:
    unsigned long getNextRSData();
    void setNextRSData(unsigned long value);
    void addNextRSData(unsigned long value);

    unsigned long getNextKeypadCheck();
    void setNextKeypadCheck(unsigned long value);
    void addNextKeypadCheck(unsigned long value);

    void print_str(SCREEN u8g, const char* str, int hpos, int vpos);
    void print_key(SCREEN u8g, char key);
    void program(SCREEN scr, Keypad keypad, char keyIn);
    void displayTemp(SCREEN scr, double val1, double val2);
};

#endif // UI_SCREEN_H_INCLUDED
