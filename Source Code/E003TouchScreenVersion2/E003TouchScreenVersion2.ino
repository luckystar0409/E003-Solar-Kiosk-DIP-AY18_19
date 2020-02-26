// This is UPDATED code for touchscreen interface done by SC
#include <UTFT.h>
#include <URTouch.h>

//==== Creating Objects =====
UTFT    myGLCD(ILI9341_16, 38, 39, 40, 41); //Parameters should be adjusted to your Display/Schield model
URTouch  myTouch( 6, 5, 4, 3, 2);

//==== Defining Variables ======
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

int x, y;

//Serial Communicatio Data Holder
char str[4];

char currentPage, selectedUnit;

//==== Ultrasonic Sensor ======
const int VCC = 13;
const int trigPin = 11;
const int echoPin = 12;

long duration;
int distanceInch, distanceCm;
//==============================================================================
void setup() {
  //Serial Communication Establish
  Serial.begin(9600);
  Serial1.begin(9600); //use Serial port 1 for arduino to arduino communication

  // Initial setup
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);

  drawHomeScreen();  // Draws the Home Screen
  currentPage = '0'; // Indicates that we are at Home Screen
  selectedUnit = '0'; // Indicates the selected unit for the first example, cm or inches
}
//================================================================================
void loop() {
  int i = 0;
  //check for serial data
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte);
  }

  if (Serial1.available()) {
    delay(100); //allows all serial sent to be received together
    while (Serial1.available() && i < 4) {
      str[i++] = Serial1.read();
    }
    str[i++] = '\0';
  }

  if (i > 0) {
    Serial.write(str, 4);
  }
  //Home page;
  if (currentPage == '0') {
    if (myTouch.dataAvailable()) {
      myTouch.read();
      x = myTouch.getX(); // X coordinate where the screen has been pressed
      y = myTouch.getY(); // Y coordinates where the screen has been pressed

      // If we press the Rent Button
      if ((x >= 35) && (x <= 285) && (y >= 90) && (y <= 130)) {
        drawFrame(35, 90, 285, 130); // Custom Function -Highlighs the buttons when it's pressed
        currentPage = '1'; // Indicates that we are the first example
        myGLCD.clrScr(); // Clears the screen
        Serial1.write('o');// a - Rent Sequence; send 'o' to mega2
      }
      // If we press the Return Button
      if ((x >= 35) && (x <= 285) && (y >= 140) && (y <= 180)) {
        drawFrame(35, 140, 285, 180);
        currentPage = '2';
        myGLCD.clrScr();
        Serial1.write('r');
      }
    }
  }
  if (currentPage == '1') { //at rental page; after clicking rent button
    if (myTouch.dataAvailable()) {
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
    }
    // If we press the Back Button
    if ((x >= 10) && (x <= 60) && (y >= 10) && (y <= 36)) {
      drawFrame(10, 10, 60, 36);
      currentPage = '0'; // Indicates we are at home screen
      Serial1.write('b');      
      myGLCD.clrScr();
      drawHomeScreen(); // Draws the home screen
    }
  }
  if (currentPage == '2') { //at return page; after clicking return button
    if (myTouch.dataAvailable()) {
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();

      //Back button
      if ((x >= 10) && (x <= 60) && (y >= 10) && (y <= 36)) {
        drawFrame(10, 10, 60, 36);
        currentPage = '0';
        Serial1.write('b');
        myGLCD.clrScr();
        drawHomeScreen();
      }
    }
  }
  //====================================================================
  if (str[0] == 'a') //rent portion; there is stock, proceed to scan card
  {
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0); // Sets the background color of the area where the text will be printed to black
    myGLCD.setColor(255, 255, 255); // Sets color to white
    myGLCD.setFont(BigFont); // Sets font to big
    myGLCD.print("Please Scan Card", CENTER, 60); // Prints the string on the screen
       myGLCD.print("|||",CENTER, 90);
       myGLCD.print("|||",CENTER, 110);
       myGLCD.print("|||",CENTER, 140);
     myGLCD.print("-------", CENTER, 170);
      myGLCD.print("-----", CENTER, 180);
       myGLCD.print("---", CENTER, 190);
        myGLCD.print("-", CENTER, 200);

    //Back button
    myGLCD.setColor(100, 155, 203);
    myGLCD.fillRoundRect (10, 10, 60, 36);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10, 10, 60, 36);
    myGLCD.setFont(BigFont);
    myGLCD.setBackColor(100, 155, 203);
    myGLCD.print("<-", 18, 15);
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setFont(SmallFont);
    myGLCD.print("Back to Main Menu", 70, 18);
  }
  if (str[0] == 'n') //rent portion; no stock
  {
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0); // Sets the background color of the area where the text will be printed to black
    myGLCD.setColor(255, 255, 255); // Sets color to white
    myGLCD.setFont(BigFont); // Sets font to big
    myGLCD.print("Out of Stock", CENTER, 30); // Prints the string on the screen
    delay(2000);  //flash meassage for 2 secs
    myGLCD.clrScr();
    currentPage = '0';
    drawHomeScreen();
  }
  if (str[0] == 'c') //rent portion; collection
  {
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0); // Sets the background color of the area where the text will be printed to black
    myGLCD.setColor(255, 255, 255); // Sets color to white
    myGLCD.setFont(BigFont); // Sets font to big
    myGLCD.print("Please Collect", CENTER, 30);
  }
  if (str[0] == 't') // return portion; no slot for return; t stands for taken
  {
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0); // Sets the background color of the area where the text will be printed to black
    myGLCD.setColor(255, 255, 255); // Sets color to white
    myGLCD.setFont(BigFont); // Sets font to big
    myGLCD.print("All Slots", CENTER, 30); // Prints the string on the screen
    myGLCD.print("Are Used", CENTER, 60); // Prints the string on the screen
    delay(2000);  //flash meassage for 2 secs
    myGLCD.clrScr();
    currentPage = '0';
    drawHomeScreen();
  }
  if (str[0] == 'e') // return portion; able to return; e stands for empty slot
  {
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0); // Sets the background color of the area where the text will be printed to black
    myGLCD.setColor(255, 255, 255); // Sets color to white
    myGLCD.setFont(BigFont); // Sets font to big
    myGLCD.print("Please Scan Card", CENTER, 60); // Prints the string on the screen
       myGLCD.print("|||",CENTER, 90);
       myGLCD.print("|||",CENTER, 10);
       myGLCD.print("|||",CENTER, 140);
     myGLCD.print("-------", CENTER, 170);
      myGLCD.print("-----", CENTER, 180);
       myGLCD.print("---", CENTER, 190);
        myGLCD.print("-", CENTER, 200);
        
    //back button
    myGLCD.setColor(100, 155, 203);
    myGLCD.fillRoundRect (10, 10, 60, 36);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10, 10, 60, 36);
    myGLCD.setFont(BigFont);
    myGLCD.setBackColor(100, 155, 203);
    myGLCD.print("<-", 18, 15);
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setFont(SmallFont);
    myGLCD.print("Back to Main Menu", 70, 18);
  }
  if (str[0] == 'g') // return portion; return powerbank after used; g stands for give back
  {
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0); // Sets the background color of the area where the text will be printed to black
    myGLCD.setColor(255, 255, 255); // Sets color to white
    myGLCD.setFont(BigFont); // Sets font to big
    myGLCD.print("Please Insert", CENTER, 30); // Prints the string on the screen
    myGLCD.print("Power Bank", CENTER, 60); // Prints the string on the screen
  }

  //display Thank you message; for both rent and return sides
  if (str[0] == 'f')
  { // when press return button at touchscreen and off the LED at mega2; touchscreen received 'f' after LED on mega2 is off and display thank you message on touchscreen
    myGLCD.clrScr();
    myGLCD.setBackColor(0, 0, 0); // Sets the background color of the area where the text will be printed to black
    myGLCD.setColor(255, 255, 255); // Sets color to white
    myGLCD.setFont(BigFont); // Sets font to big
    myGLCD.print("Thank you!", CENTER, 30); // Prints the string on the screen
    delay(2000);  //flash meassage for 2 secs
    myGLCD.clrScr();
    currentPage = '0';
    drawHomeScreen();

    for ( int j = 0; j < sizeof(str);  ++j ) {
      str[j] = (char)0;
    }
  }
  for ( int j = 0; j < sizeof(str);  ++j ) {
    str[j] = (char)0;
  }
}
//================================================================================
// ====== Custom Funtions ======
// drawHomeScreen - Custom Function
void drawHomeScreen() {
  // Title
  myGLCD.setBackColor(0, 0, 0); // Sets the background color of the area where the text will be printed to black
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(BigFont); // Sets font to big
  myGLCD.print("Powerbank Rental", CENTER, 10); // Prints the string on the screen
  myGLCD.setColor(255, 0, 0); // Sets color to red
  myGLCD.drawLine(0, 32, 319, 32); // Draws the red line
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.setFont(SmallFont); // Sets the font to small
  myGLCD.print("by NTU Students", CENTER, 41); // Prints the string
  myGLCD.setFont(BigFont);
  myGLCD.print("Please Select", CENTER, 64);

  // Button - Rent
  myGLCD.setColor(16, 167, 103); // Sets green color
  myGLCD.fillRoundRect (35, 90, 285, 130); // Draws filled rounded rectangle
  myGLCD.setColor(255, 255, 255); // Sets color to white
  myGLCD.drawRoundRect (35, 90, 285, 130); // Draws rounded rectangle without a fill, so the overall appearance of the button looks like it has a frame
  myGLCD.setFont(BigFont); // Sets the font to big
  myGLCD.setBackColor(16, 167, 103); // Sets the background color of the area where the text will be printed to green, same as the button
  myGLCD.print("RENT", CENTER, 102); // Prints the string

  // Button - Return
  myGLCD.setColor(16, 167, 103);
  myGLCD.fillRoundRect (35, 140, 285, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (35, 140, 285, 180);
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(16, 167, 103);
  myGLCD.print("RETURN", CENTER, 152);
}

// Highlights the button when pressed
void drawFrame(int x1, int y1, int x2, int y2) {
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
    myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}
