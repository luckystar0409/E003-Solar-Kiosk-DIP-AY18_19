#include "ui_screen.h"

//Private Methods

/* function: keyInValue
    A function that can save values (3 digits max)into the transmission packet variable 'data'.
    Sample usage:
    char output = keyInValue(STAGE1_TEMP, keypad); //this will store the input values on the Stage1 temperature only
    char output = keyInValue(START_TEMP, keypad); //this will store the input values on the Start Temp parameter only

    See param_t definition for a list of possible inputs on the first argument.
    This function can only be used within the library. Arduino cannot access this.
*/
char UIScreen::keyInValue(param_t row, Keypad keypad){
    char mode = '\0';
    int index;
    for(index = 0; index < 3; index++){ //the largest input value will have 3 digits
        mode = keypad.waitForKey();
        switch(mode){
        case '#':
        case 'A':
            index++;
            break;
        default:
            data[index][row] = mode;
        }

    }
    switch(index){
    case 0:
        //no input; user cancelled or mispressed A
        break;
    case 1:
        //1 digit input
        data[2][row] = data[0][row];
        data[0][row] = '0';
        data[1][row] = '0';
        data[3][row]='%'; //terminate current row with '%' delimiter
        break;
    case 2:
        data[2][row] = data[1][row];
        data[1][row] = data[2][row];
        data[0][row] = '0';
        data[index][row]='%'; //terminate current row with '%' delimiter
        break;
    case 3:
         data[index][row]='%'; //terminate current row with '%' delimiter
         break;
    }
    return mode;
}

/* Function: displayText
    Makes displaying text on the OLED screen easier.
    Sample usage:
        displayText(scr, "1. Start Temp", "2. Stage1 Temp", "3. Stage2 Temp");

    Only the library can use this function.
*/
void displayText(SCREEN scr, const char* line1, const char* line2, const char* line3){
    scr.firstPage();
    do{
        scr.setFont(u8g_font_unifont);
        scr.setPrintPos(0,20);
        scr.print(line1);
        scr.setPrintPos(0,40);
        scr.print(line2);
        scr.setPrintPos(0,60);
        scr.print(line3);
    }while(scr.nextPage());
}

void UIScreen::computeHoldTime(){
    int endTemp[STAGES+1];
    int startTemp[STAGES+1];
    int rates[STAGES+1];
    int times[STAGES+1];
    char holder[3];
    int rate_index, temp_index;

    temp_index = 0;
    //convert for all setpoint temps for all stages
    for(int row = 1; row < DATAROWSIZE; row += 3){
        for(int col = 0; col < 3; col+=3){
            holder[col] = data[col][row];
        }
        endTemp[temp_index] = atoi(holder);
        temp_index++;
    }

    rate_index = 0;
    for(int row = 2; row < DATAROWSIZE; row+=3){
        for(int col = 0; col < 3; col+=3){
            holder[col] = data[col][row];
        }
        rates[rate_index] = atoi(holder);    //converts rates to int
        rate_index++;
    }

    for(int col = 0; col < 3; col+=3){
        holder[col] = data[col][START_TEMP];
    }
    startTemp[0] = atoi(holder);

    for (int i = 0; i < temp_index - 1; i++){
        startTemp[i] = endTemp[i+1];
        if(rates[i] != 0)
            times[i] = (endTemp[i] - startTemp[i])/rates[i];
        else
            times[i] = defaults[(i+1)*3];
    }

    //convert to string
    for(int i = 0; i < rate_index; i++){
        sprintf(holder, "%03d", times[i]);
        strcpy(data[i*3], holder);
        data[3][i*3]='%';
    }
}

//Public Methods
unsigned long UIScreen::getNextRSData(){
    return nextRSData;
}

void UIScreen::setNextRSData(unsigned long value){
    nextRSData = value;
}

void UIScreen::addNextRSData(unsigned long value){
    nextRSData += value;
}

unsigned long UIScreen::getNextKeypadCheck(){
    return nextKeypadCheck;
}

void UIScreen::setNextKeypadCheck(unsigned long value){
    nextKeypadCheck = value;
}

void UIScreen::addNextKeypadCheck(unsigned long value){
    nextKeypadCheck += value;
}

void UIScreen::print_str(SCREEN u8g, const char* str, int hpos, int vpos){
  u8g.firstPage();
  do{
    u8g.setFont(u8g_font_unifont);
    u8g.setPrintPos(hpos, vpos);
    u8g.print(str);
  }while(u8g.nextPage());
}

/* Function: program
    Function that handles the configuring system parameters

    Note: All menus are consolidated into this single function. It makes the flow of the program simpler.
    Take note of the comments below. They include discussions on cancelling halfway in the menus.

    This can be used in Arduino code.
*/
void UIScreen::program(SCREEN scr, Keypad keypad, char keyIn){
    recentSelection = '\0'; //values for this variable will be stored for inputHistory
    isCancel = '\0'; //variable for checking cancellation during inputting values

    menuInput[0]=keyIn;
    //first menu
    switch(keyIn){
      case '*':
        displayText(scr, "1. Start Temp", "2. Stage1 Temp", "3. Stage2 Temp");
        recentSelection = keypad.waitForKey();
        break;
      case 'D':
        displayText(scr, "Running system with", "configured settings.", "Confirm? [D]");
        recentSelection = keypad.waitForKey();
        break;
      case 'C':
        displayText(scr, "Switching system to", "idle mode.", "Confirm? [C]");
        recentSelection = keypad.waitForKey();
      default:
        //do nothing
        return;
    }

    menuInput[1] = recentSelection;
    //second menu
    switch(recentSelection){
        case '1':
            displayText(scr,"Enter StartTemp","Current value",data[START_TEMP]);
            isCancel = keyInValue(START_TEMP, keypad);
            if(isCancel == '#')
                return;
            delay(1000);
            break;
        case '2':
            displayText(scr, "1. Stage1 Temp", "2. Stage1 Rate/min", " ");
            recentSelection = keypad.waitForKey(); //goes to the third menu
            break;
        case '3':
            displayText(scr, "1. Stage2 Temp", "2. Ramp Rate/min"," ");
            recentSelection = keypad.waitForKey();
            break;
        case '#':
            displayText(scr, "Cancel", " ", " ");
            delay(1000);
            return;
        case 'D':
            displayText(scr, "Confirmed!", "Sending config", "to system...");
            //check for holdtime computations
            computeHoldTime();
            for(int row=0;row<10;row++){
                for(int col=0;col!=4;col++){
                    Serial2.print(data[col][row]); //receiver will interpret the contents of this packet stream
                }
            }
            delay(2000);
            break;
        case 'C':
            displayText(scr, "Confirmed!", "Switching system to", "idle state...");
            for(int row=0;row<10;row++){
                for(int col=0;col!=4;col++){
                    Serial2.print('#'); //receiver should be able to detect this and command the system to stop
                }
            }
            delay(2000);
            break;
        default:
            displayText(scr, "You entered an", "invalid input.", " ");
            delay(2000);
            return;
    }

    menuInput[2] = recentSelection;
    //thirdMenu
    if(menuInput[1] == '2'){
        switch(recentSelection){
        case '1':
            displayText(scr,"Enter Stage1Temp","Current value",data[STAGE1_TEMP]);
            isCancel = keyInValue(STAGE1_TEMP, keypad);
            if (isCancel == '#')
                return;
            delay(1000);
            break;
        case '2':
            displayText(scr, "Enter Stage1 Rate", "Current value", data[STAGE1_RATE]);
            isCancel = keyInValue(STAGE1_RATE, keypad);
            if(isCancel == '#')
                return;
            delay(1000);
            break;
        default:
            displayText(scr, "You entered an", "invalid input.", " ");
            delay(2000);
            return;
        }
    }else if(menuInput[1] == '3'){
        switch(recentSelection){
        case '1':
            displayText(scr, "Enter Stage2 Temp", "Current value:", data[STAGE2_TEMP]);
            isCancel = keyInValue(STAGE2_TEMP, keypad);
            if(isCancel == '#')
                return;
            delay(1000);
            break;
        case '2':
            displayText(scr, "Enter Stage2 Rate", "Current value:", data[STAGE2_RATE]);
            isCancel = keyInValue(STAGE2_RATE, keypad);
            if(isCancel == '#')
                return;
            delay(1000);
            break;
        default:
            displayText(scr, "You entered an", "invalid input.", " ");
            delay(2000);
            return;
        }
    }
    return;
    //at this point, the user will return to main display. The user should long press 'D' to transmit the input values.
}

void UIScreen::displayTemp(SCREEN scr, double val1, double val2){
    scr.firstPage();
    do{
      scr.setFont(u8g_font_unifont);
      scr.setPrintPos(0,20);
      scr.print("Temp Stat");
      scr.setPrintPos(0,40);
      scr.print("RS1: ");
      scr.print(val1);
      scr.print(" C");
      scr.setPrintPos(0,60);
      scr.print("RS2: ");
      scr.print(val2);
      scr.print(" C");
    }while(scr.nextPage());

}


