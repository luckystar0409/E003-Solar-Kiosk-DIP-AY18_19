#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

/*For Neopixel ledstrip troubleshooting*/
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define RGBstrip A0
#define NUMPIXELS 7
#define sensorPin A1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, RGBstrip, NEO_GRB + NEO_KHZ800);

uint32_t yellow = pixels.Color(255, 100, 0);
uint32_t green = pixels.Color(0, 150, 0);
/*
  PINOUT:
  RC522 MODULE    Uno/Nano     MEGA
  SDA             D10          D9
  SCK             D13          D52
  MOSI            D11          D51
  MISO            D12          D50
  IRQ             N/A          N/A
  GND             GND          GND
  RST             D9           D8
  3.3V            3.3V         3.3V
*/

#define SDA_PIN 9
#define RST_PIN 8
//MFRC522 mfrc522(SDA_PIN, RST_PIN);
MFRC522 mfrc522(9, 8);

bool state;
// Set constant variables for pins used by the sensor, buzzer, servo and LEDs:
const int trigPin = A14; // Note: set these numbers to what ever pin you attached them to
const int echoPin = A13;
const int ledPin = A9;
const int actLedPin = A8;
const int buzzPin = A15;
//boolean alarmActive = false;
const String targetUID = "70 EC 7A A6";
//String targetUID[1]={"70 EC 7A A6", "70 17 7C A6"}

int servoPin1 = 40;
int servoPin2 = 41;
Servo servo;  // create servo object to control a servo
int pos = 90;   // default servo position in degrees

//=======================================
/*For light detection purpose*/
unsigned int intensity = 0;
int sensorValue = 0;

//we need to declare another ultrasonic sensor pin and names

//Variable for Ultrasonic Sensor
float flightTime_microsec, flightTime_sec;
float dist_dist_cm;
float soundspeed_dist_cmps;
float soundspeed_mps = 343.5;
float dist_cm;

char str[4];
const int LED =  13;
int inByte;
//========================================================================
void setup() {
  Serial.begin(9600); //use for computer
  Serial1.begin(9600);  // use for boards

  // Initialize RFID Reader:
  SPI.begin();
  mfrc522.PCD_Init();

  // Initialize LEDs:
  //pinMode(ledPin, OUTPUT);
  //pinMode(actLedPin, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(RGBstrip, OUTPUT);

  // Initialize ultrasonic-sensor:
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize servo:
  servo.attach(servoPin1);
  servo.attach(servoPin2);
  //servo.write(); // Turn SG90 servo to X degrees

  // Ensure alarm is set to off to prevent premature activation:
  //alarmActive = false;
  digitalWrite(actLedPin, LOW);

  //Reset Servo Position
  for (pos = 180;  pos--;) //move the servo from 0 degrees to 180 degrees
  {
    servo.write(pos); // tell servo to go to position in variable 'pos'
    delay(5); // increase 1 deg per 10millisecond
  }
  servo.detach();

  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show();
}
//========================================================================
void loop() {
  int i = 0;

  if (Serial.available()) {   //Writing TX after it send it flush by itself
    int inByte = Serial.read();
    Serial1.write(inByte);
  }

  if (Serial1.available()) {    //Receiving RX
    delay(100); //allows all serial sent to be received together
    while (Serial1.available() && i < 4) {
      str[i++] = Serial1.read();
    }
    str[i++] = '\0'; //delimiter
  }

  if (i > 0) {
    Serial.write(str, 4);
  }
  if (str[0] == 'o') // if mega2(this board) receive 'o' after pressing the 'RENT' button on touchscreen(which send 'o' to mega2)
  {
    runSensor(); //on the ultrasonic Sensor to check stock for renting
  }
  if (str[0] == 'r') // return; if mega2 received 'r' after pressing the 'RETURN' button on touchscreen(which send 'f' to mega2)
  {
    mDist();
    runReturn(); //on the ultrasonic Sensor to check availability for return
  }
  else if (str[0] == 'b') // back button is pressed
  {
    state = false;
  }
  for (int j = 0; j < sizeof(str); ++j)
  {
    str[j] = (char)0;
  }

  pixels.clear();
  lightDetection();
  for (int i = 0; i < intensity; i++) {
    pixels.setPixelColor(i, yellow);
    delay(1);
  }
  pixels.show();
}
//=====================================================================================
/*Measure my distance interval between ultrasonic sensor to the ground*/
float mDist()
{
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  flightTime_microsec = pulseIn(echoPin, HIGH);
  flightTime_sec = flightTime_microsec * pow(10, -6);

  soundspeed_dist_cmps = soundspeed_mps * 100;
  dist_dist_cm = soundspeed_dist_cmps * (flightTime_sec / 2);
  Serial.println("My Current Distance:");
  Serial.print(dist_dist_cm);
  Serial.println();
  dist_cm = dist_dist_cm;
}
//===========================================================================
void runSensor() //under void loop
{
  // Reset alarm at initialization to avoid premature activation:
  noTone(buzzPin);
  digitalWrite(ledPin, LOW);

  //Measure the distance with Ultrasonic Sensor and print the distance
  mDist();

  // Check if there is any stock(powerbank):
  checkStock(dist_cm);

  // Check if alarm has been triggered:
  // checkAlarm(dist_cm);
  Serial.println("first run complete");
  // Delay for 50 milliseconds before looping over again
  // This ensures that the sensor is pulsing fast enough to detect fast-moving objects
  // but still slow enough for the RFID Reader to check if a tag/card has been presented:
  delay(100);
}
//======================================================================================
void checkStock(long dist_cm) //subpart of runSensor(); rent portion
{
  if (dist_dist_cm < 8)
    //if (dist_dist_cm < 3) //less than 3dist_cm; TRUE
  {
    Serial.println("Available"); //there is powerbank to rent
    //digitalWrite(ledPin, HIGH); //LED light up to show there is powerbank
    Serial1.write('a'); // send 'a' to touchscreen to display "Scan card"; a stands for available
    state = true;
  }
  else  //more than dist_cm; FALSE
  {
    Serial.println("No Stock");
    //digitalWrite(ledPin, LOW);
    Serial1.write('n'); // send 'n' to touchscreen to display "Out of Stock"; n stands for no stock
    delay(500);
    state = false;
  }
  while (state)
  {
    readForCard();
  }
  pixels.clear();
}
//======================================================================
void readForCard() //under void loop
{
  Serial.println("I'm reading your RFID");
  // Look for new cards:
  while ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards:
  while ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  // Get UID from card/tag:
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  // Check UID for match and set alarm:
  content.toUpperCase();

  // Change the UID below to the tag/card that you want to use for access
  // Use DumpInfo (File > Examples > MFRC522 > DumpInfo) to get this infomation
  if (content.substring(1) == targetUID)
  {
    servo.attach(servoPin1);
    servo.attach(servoPin2);
    //digitalWrite(actLedPin, HIGH);  //on led
    // Set off a short tone and an LED to signal a sucessful activation/deactivation:
    tone(buzzPin, 500); //on
    delay(50);
    noTone(buzzPin);    //off
    delay(50);
    tone(buzzPin, 500); //on
    delay(50);
    noTone(buzzPin);    //off

    Serial1.write('c'); // send 'c' to touchscreen to display message
    analogWrite(RGBstrip, 255);
    //on servo to push powerbank out of the slot
    for (pos = 0; pos <= 180; pos++) //move the servo from 0 degrees to 180 degrees
    {
      servo.write(pos); // tell servo to go to position in variable 'pos'
      delay(5); // increase 1 deg per 10millisecond
    }
    for (pos = 180; pos >= 0; pos--) //move the servo from 180 degrees to 0 degrees
    {
      servo.write(pos); // tell servo to go to position in variable 'pos'
      delay(5); // increase in 1 deg per 10millisecond
    }
    servo.detach();
    sensorbuzz();
  }
  else // Make a short tone to indicate that the alarm has failed to activate:
  {
    Serial.println("ACCESS DENIED - ERROR");

    tone(buzzPin, 800);
    delay(200);
  }
  if (str[0]== 'b'){
   state = false; 
  }
  state = false;
}
//==========================================================================
void sensorbuzz() //under void loop
{
  // Reset alarm at initialization to avoid premature activation:

  //Ultrasonic Sensor read and measure the distance then return back (1 time)
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, green); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
  // Check if alarm has been triggered:
  //checkAlarm(dist_cm);
  do {
    mDist();
    tone(buzzPin, 2000, 500);
  } //while (dist_cm < 3);  // still detecting powerbank
  while (dist_cm > 3);

  //Power Bank Taken and send Thank you message
  //digitalWrite(ledPin, LOW);
  Serial1.write('f');
  state = false;

  // Delay for 50 milliseconds before looping over again
  // This ensures that the sensor is pulsing fast enough to detect fast-moving objects
  // but still slow enough for the RFID Reader to check if a tag/card has been presented:
  delay(50);
}
//===================================================================
void runReturn()
{
  // Reset alarm at initialization to avoid premature activation:
  noTone(buzzPin);
  //digitalWrite(ledPin, LOW);

  mDist();

  // Check if any space is occupied:
  checkbank(dist_cm);
  Serial.println("Return Process Complete");
  // Delay for 50 milliseconds before looping over again
  // This ensures that the sensor is pulsing fast enough to detect fast-moving objects
  // but still slow enough for the RFID Reader to check if a tag/card has been presented:
  delay(100);
}
//=================================================================================
void checkbank(long cm) //subpart of runReturn(); for return portion; taking
{
  if (dist_cm > 4)
    //if (dist_cm < 3)  //powerbank space is occupied
  {
    Serial.println("Occupied");
    //digitalWrite(ledPin, LOW); //LED off when there is no available space
    Serial1.write('t'); // send 't' to touchscreen; t stands for taken
    //RGB led strip show RED for no available space
    pixels.setPixelColor(7, pixels.Color(255, 0, 0));
    pixels.show();
    delay(1000);
  }
  else  //there is an empty space, more than or equal to 3cm
  {
    Serial.println("Empty");
    //digitalWrite(ledPin, HIGH); //LED light up to show there is slot to return
    Serial1.write('e'); // send 'e' to touchscreen; e stands for empty
    state = true;
    //RGB led strip show GREEN for available space
    pixels.setPixelColor(7, pixels.Color(0, 255, 0));
    pixels.show();
    delay(1000);
  }
  while (state)
  {
    CardforReturn();
  }
}
//==========================================================================================
void CardforReturn() //under void loop
{
  Serial.println("I'm reading your card for returning");
  // Look for new cards:
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards:
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  // Get UID from card/tag:
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  // Check UID for match and set alarm:
  content.toUpperCase();
  // Change the UID below to the tag/card that you want to use for access
  // Use DumpInfo (File > Examples > MFRC522 > DumpInfo) to get this infomation
  if (content.substring(1) == targetUID)
  {
    digitalWrite(actLedPin, HIGH);  //on led
    // Set off a short tone and an LED to signal a sucessful activation/deactivation:
    tone(buzzPin, 500); //on
    delay(50);
    noTone(buzzPin);    //off
    delay(50);
    tone(buzzPin, 500); //on
    delay(50);
    noTone(buzzPin);    //off

    Serial1.write('g'); // send 'g' to touchscreen to display insert powerbank message
    SenReturn();
  }
  else // Make a short tone to indicate that the alarm has failed to activate:
  {
    Serial.println("ACCESS DENIED - ERROR");

    tone(buzzPin, 800);
    delay(200);
  }
  state = false;
}
//==========================================================================
void SenReturn()
{
  // Reset alarm at initialization to avoid premature activation:
  noTone(buzzPin);
  //digitalWrite(ledPin, LOW);

  //Check if powerbank is returned:
  /*do {
    mDist();
    } while (dist_cm >= 3); //checking when no powerbank is detected

    laststep(dist_cm);*/
  do {
    mDist();
  } while (dist_cm < 4.5);

  if (dist_cm < 8)
  {
    mDist();
    delay(100);
  }
  else {
    SenReturn();
  }
  laststep(dist_cm);

  // Delay for 50 milliseconds before looping over again
  // This ensures that the sensor is pulsing fast enough to detect fast-moving objects
  // but still slow enough for the RFID Reader to check if a tag/card has been presented:
  delay(100);
}
//=================================================================================
// Checks if the sensor has detected an object and triggers the alarm
void laststep(long cm) //subpart of SenReturn()
{
  if (cm < 10)
    //if (cm < 3)
  {
    Serial.println("Received");
    Serial1.write('f'); //send 'f' to touchscreen to display Thank you message
    //digitalWrite(ledPin, HIGH);
    pixels.setPixelColor(7, pixels.Color(0, 255, 0)); //show green after success transaction
    pixels.show();
    delay(1000);
    tone(buzzPin, 500);     //RFID and alarm are activated, Buzzer on when still detecting powerbank in the range
    delay(200);
    tone(buzzPin, 300);
    delay(200);
    tone(buzzPin, 100);
    delay(500);
    noTone(buzzPin);
  }
  /*else if (alarmActive == false) //alarm is not activated but powerbank in range
    {
    Serial.println("Waiting for card reader");
    digitalWrite(ledPin, LOW);
    noTone(buzzPin);
    //delay(150);
    }*/
  else  // more than 3cm
  {
    Serial.println("Not in range"); //powerbank is taken
    //digitalWrite(ledPin, LOW);
    noTone(buzzPin);
  }
}
//=======================================================================================
void lightDetection()
{
  sensorValue = analogRead(sensorPin);
  intensity = sensorValue / 50;
  /*Set an indication of maximum intensity at 7 led strip
     low light intensity at 1
     so it is from the range of 1-7 brightness indication
  */
}
