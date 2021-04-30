//Needed Libraries for the LCD Display screen
#if defined(ENERGIA) // LaunchPad MSP430, Stellaris and Tiva, Experimeter Board FR5739 specific
#include "Energia.h"
#else // error
#error Platform not defined
#endif
//Include application, user and local libraries
#include <SPI.h>

#include <LCD_screen.h>
#include <LCD_screen_font.h>
#include <LCD_utilities.h>
#include <Screen_HX8353E.h>
#include <Terminal12e.h>
#include <Terminal6e.h>
#include <Terminal8e.h>
Screen_HX8353E myScreen;

#include <Wire.h>      //needed libraries for the temperature sensor
#include "Adafruit_TMP006.h"
//variable used to access the temperature sensor 
Adafruit_TMP006 temp_sen;
float objectT;

//used for the ambient light
#include "OPT3001.h"
#define USE_USCI_B1 
opt3001 op;
uint32_t readings;  //holds the value of the ambient light

const int joystickSel = 5;     // the number of the joystick select pin
const int joystickX = 2;       // the number of the joystick X-axis analog
const int joystickY =  26;     // the number of the joystick Y-axis analog
int joystickSelState = 0;      // variable for reading the joystick sel status
float joystickXState, joystickYState, joystickX_angle, joystickY_angle;
int ledRed = 39, ledBlue = 37, ledGreen = 38;    // LED connected to digital pin 9

//used for accelerometer 
const int xpin = 23;                  // x-axis of the accelerometer
const int ypin = 24;                  // y-axis
const int zpin = 25;                  // z-axis (only on 3-axis models)
float xpin_current = 0.0;  //holds the current value of the x-pin
float ypin_current = 0.0;  //holds the current value of the y-pin
float xpin_angle = 0.0;  //holds the angle of the x-pin 
float ypin_angle = 0.0;   //holds the angle of the y-pin

long previousMillis = 0;  //used to keep track of the elaspsed two to increment the seconds
int buzzerPin = 40; //used to set up the buzzer pin
int userinput; //holds the input from the user
int next_freq = 0; //increments the frequency values for the buzzer
int incomingByte = -1;   //used to read in the values from the user
int u_input = -1;

const byte numChars = 32;
char receivedChars[numChars]; //an array to store the received data
char tempChars[numChars];  //temporary array
boolean newData = false;  //checks if new data is available
int rlen, slen, glen; //holds garbage data
String userin;
int count = 0;
int alpha_count = 0;
int space_count = 0;
int vowel_count = 0;
int words_count = 0;
uint16_t x, y, x00, y00;
char buf[numChars]; //an array to store the received data
char buf_num[numChars];  //temporary array
int Brightness;  //holds the brightness of the RGB light, a user input
int rValue; //used to convert the brightness and RGB value
int gValue; //used to convert the brightness and RGB value
int bValue; //used to convert the brightness and RGB value 

String user_text;
String alpha_text;
String vowel_text;
String space_text;
String words_text;
int flag_menu = false;
int flag_five_sec = true;
int flag_data = true;
int flag_count = true;

void setup()
{
  // put your setup code here, to run once:
  analogReadResolution(12);
  unsigned int readings = 0;
  op.begin();   //set up the ambient light reading
  // initialize the pushbutton pin as an input:
  pinMode(joystickSel, INPUT_PULLUP);   
  Serial.begin(9600);      //open the serial port at 9600 bps:
  myScreen.begin();
  x00 = 0;
  y00 = 0;
  //initializes the led
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  
  if(!temp_sen.begin())  //corresponding message is printed out if the temperature sensor is not found
  {
     Serial.println("No sensor found");
     while(1); 
  }
   //prints out the menu once
   Serial.print("\n");
   Serial.println("\t \t Troubleshooting Program \t \t");
   Serial.print("\n");
   Serial.println("1. Read the object temperature and print the value in Celsius");
   Serial.println("2. Read the accelerometer and print the x and y angle");
   Serial.println("3. Read the ambient light intensity and print the Lux value");
   Serial.println("4. Read the joystick x and y and print x and y angle");
   Serial.println("5. Write the input text to the LCD screen by capitalzing the first alphabet of the input text");
   Serial.println("6. Generate a color on the LED [indigo or tan] with a brightness [0 to 100%]");
   Serial.println("7. Generate a beep tone on the buzzer"); 
   Serial.print("\n");
   Serial.print("Please input the selection[1-7]: "); 
}

void loop()
{
  // put your main code here, to run repeatedly:
  if(Serial.available() > 0)  //if data is available, read from the buffer
  {
      u_input = Serial.parseInt();   //read from the serial
      if((u_input > 0) && (flag_data))
      {
         incomingByte = u_input;
         flag_data = false;
         flag_menu = false;
      }
  }
  
  if((incomingByte > 0))   //menu prints out every 5 seconds //&&((millis() - previousMillis) >= 5000)
  {   /*
      Serial.print("\n");
      Serial.println("\t \t Troubleshooting Program \t \t");
      Serial.print("\n");
      Serial.println("1. Read the object temperature and print the value in Celsius");
      Serial.println("2. Read the accelerometer and print the x and y angle");
      Serial.println("3. Read the ambient light intensity and print the Lux value");
      Serial.println("4. Read the joystick x and y and print x and y angle");
      Serial.println("5. Write the input text to the LCD screen by capitalzing the first alphabet of the input text");
      Serial.println("6. Generate a color on the LED [indigo or tan] with a brightness [0 to 100%]");
      Serial.println("7. Generate a beep tone on the buzzer"); 
      Serial.print("\n");
      Serial.print("Please input the selection[1-7]: "); 
      */
      
      if((incomingByte > 0))
      {//print only a selected range of numbers
        Serial.println(incomingByte);
        //flag_menu = false; //sets a flag to control how often the menu is printed
        flag_data = true; //enables new data to be read into a variable
        flag_menu = true;
      } 
      previousMillis  = millis();    //resets the clock to the elapsed time
      myScreen.clear();  //clears the board, restores it to its original state 
      digitalWrite(ledRed, LOW);
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledBlue, LOW);
  
   //the user input, incomingByte, determines the command
  switch(incomingByte)
  {
    case 1:
      //print the temperature values
      objectT = temp_sen.readObjTempC();
      Serial.print("Object temperature: ");
      Serial.print(objectT);
      Serial.println(" C");
      incomingByte = 0;  //clears the data variable that holds the data read
      previousMillis  = millis();    //resets the clock to the elapsed time
      break;
    case 2:
      //reading the accelerometer sensor values 
      xpin_current = ((int) analogRead(xpin)) - 2048;
      ypin_current = ((int) analogRead(ypin)) - 2048;
      xpin_angle = (xpin_current*.11) + .90;  //converts the xpin into angle values
      ypin_angle = (ypin_current* -.11)- 1.02;  //converts the ypin into angle values
      Serial.print(xpin_angle);  //prints out the values
      Serial.print(", ");
      Serial.println(ypin_angle);
      incomingByte = 0; //clears the data value read
      previousMillis  = millis();    //resets the clock to the elapsed time
      break;
    case 3: 
       //prints the ambient light intesnity 
       readings = op.readResult();  //reads the ambient light intensity 
      Serial.println(readings);
      incomingByte = 0;  //clears the variable that reads the users input
      previousMillis  = millis();    //resets the clock to the elapsed time
      break; 
   case 4:
      // read the analog value of joystick x axis
      joystickXState = analogRead(joystickX);
      // read the analog value of joystick y axis
      joystickYState = analogRead(joystickY);
      joystickX_angle = (joystickXState * .0093) - 20.033;
      joystickY_angle = (joystickYState *.0099) -20.4811;
      Serial.print(joystickX_angle);
      Serial.print(", ");
      Serial.println(joystickY_angle);
      incomingByte = 0; //clears the variable that reads the users input.
      previousMillis  = millis();    //resets the clock to the elapsed time
      break;
   case 5:
      //print the input text on the LCD display
      Serial.print("Input the string: "); //print statement
      if(Serial.available() > 0)
      {
        int discard1 = Serial.read();
      }
      slen = Serial.readBytesUntil('\r', tempChars, numChars); //reads until carriage return
      Serial.println(tempChars);  //prints the users input
      for(int i = 0; i < 32; i++)  //iterates through the character array and processes the information
      {
        if((isAlpha(tempChars[i])) && (count == 0))
        {  
          tempChars[i] = toUpperCase(tempChars[i]);
          count++;
        }
        else if(isAlpha(tempChars[i]) && (count >= 1))
        {
          tempChars[i] = toLowerCase(tempChars[i]);
        }
        else
        {
           tempChars[i] = tempChars[i]; 
        }
        
        if(isAlpha(tempChars[i]))
        {
          alpha_count++;
        }
        
        if(isSpace(tempChars[i]))
        {
          space_count++;
          if(isAlpha(tempChars[i+1]))
          {
            words_count++;
          }
        }
        if (i == 0)
        {
          if(isAlpha(tempChars[i]))
          {
            words_count++;
          }
        }
        if(tempChars[i] == 'a' || tempChars[i] == 'i' || tempChars[i] == 'e' || tempChars[i] == 'o' || tempChars[i] == 'u')
        {
            vowel_count++;
        }
      }
     user_text = String(tempChars);  //convert the variables into strings, to print on the LCD screen
     alpha_text = String(alpha_count);
     vowel_text = String(vowel_count);
     space_text = String(space_count);
     words_text = String(words_count);
    
     myScreen.gText(1,44, "Input text: " + user_text);  //print onto the LCD screen
     myScreen.gText(1,54, "Aplha count: " + alpha_text);
     myScreen.gText(1,64, "Space count: " + space_text);
     myScreen.gText(1,74, "Words count: " + words_text);
     myScreen.gText(1,84, "Vowel count: " + vowel_text); 
     myScreen.setFontSize(1);  //set the LCD screen
     //reset the values of the needed variables
     count = 0;
     alpha_count = 0;
     space_count = 0;
     words_count = 0;
     vowel_count = 0;
     incomingByte = 0; //clears the variable that reads the users input.
     previousMillis  = millis();    //resets the clock to the elapsed time
     break; 
   case 6: 
     //prints indigo or tan depending on the input
     if(Serial.available() > 0)  //if data is available, read from the buffer
     {
       int discard2 = Serial.read();
     }
    
     strcpy(buf, "");
     Serial.print("Input the color: ");
     rlen = Serial.readBytesUntil('\r', buf, numChars);  //read data from the buffer
     if((buf[0] == 't') && (buf[1] == 'a') && (buf[2] == 'n'))
     { 
         Serial.print("tan");
     }
     else
     {
       Serial.print(buf);
     }
     Serial.print("\nInput the brightness: ");
     glen = Serial.readBytesUntil('\r', buf_num, numChars);  //read data from the buffer
     Brightness = atoi(buf_num);  //convert data to an integer
     Serial.print(Brightness);
     Serial.println("%");
     if((buf[0] == 't') && (buf[1] == 'a') && (buf[2] == 'n'))
     {  //set up the rgb values
      rValue = analogRead(0);
      rValue = map(Brightness, 0, 100, 0, 210);
      gValue = analogRead(0);
      gValue = map(Brightness, 0, 100, 0, 180);
      bValue = analogRead(0);
      bValue = map(Brightness, 0, 100, 0, 140);
      //3 RGB channels for tan
      analogWrite(ledRed, rValue);
      analogWrite(ledGreen, gValue);
      analogWrite(ledBlue, bValue); 
     }
  
     if(strcmp(buf, "indigo")== 0)  //the following set of actions ofccur when the input is equal to indigo
     {  //set up the rgb values
      rValue = analogRead(0);
      rValue = map(Brightness, 0, 100, 0, 75);
      gValue = analogRead(0);
      gValue = map(Brightness, 0, 100, 0, 0);
      bValue = analogRead(0);
      bValue = map(Brightness, 0, 100, 0, 130);
      //3 RGB channels for indigo
      analogWrite(ledRed, rValue);
      analogWrite(ledGreen, gValue);
      analogWrite(ledBlue, bValue); 
     }
      incomingByte = 0; //clears the variable that reads the users input
      previousMillis  = millis();    //resets the clock to the elapsed time
      break;
    case 7: 
     //grenerates a tone buzzer 
     tone(buzzerPin, 100 + 10*next_freq, 200);  //turn on the tone function 
     next_freq++;
     Serial.println("Tone generated!");
     incomingByte = 0; //clears the variable that reads the users input
     previousMillis  = millis();    //resets the clock to the elapsed time
     break;
    default: 
     incomingByte = 0; //clears the variable that reads the users input. 
  }
  }
  if(((millis() - previousMillis) >= 5000) && (flag_menu)&& (incomingByte <= 0))
  {
       print_menu(); 
       flag_menu = false;
  }
  
}

void print_menu()
{
    Serial.print("\n");
    Serial.println("\t \t Troubleshooting Program \t \t");
    Serial.print("\n");
    Serial.println("1. Read the object temperature and print the value in Celsius");
    Serial.println("2. Read the accelerometer and print the x and y angle");
    Serial.println("3. Read the ambient light intensity and print the Lux value");
    Serial.println("4. Read the joystick x and y and print x and y angle");
    Serial.println("5. Write the input text to the LCD screen by capitalzing the first alphabet of the input text");
    Serial.println("6. Generate a color on the LED [indigo or tan] with a brightness [0 to 100%]");
    Serial.println("7. Generate a beep tone on the buzzer"); 
    Serial.print("\n");
    Serial.print("Please input the selection[1-7]: "); 
}
