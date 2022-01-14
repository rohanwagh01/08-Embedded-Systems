#include<string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "dome_image.h"
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
const int BUTTON = 5;
int state = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //initialize serial!
  // Set up Active Low Switch
  pinMode(BUTTON,INPUT_PULLUP); //sets IO pin 5 as an input which defaults to a 3.3V signal when unconnected and 0V when the switch is pushed
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  // initial drawing
  tft.setTextColor(TFT_RED, TFT_LIGHTGREY);
  tft.fillScreen(TFT_LIGHTGREY);
  tft.drawString("6.08 Lab ONE!!", 40, 0, 2); //viewable on Screen 
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = digitalRead(BUTTON);
  if (value){ // button not pressed
     if (state == 1) { // change to state 2
       state = 2;
     }
     if (state == 3) { // change to state 0
       state = 0;
     }   
  }else{ // button pressed
     if (state == 0){ // change the picture and state to 1
       tft.setTextColor(TFT_RED, TFT_LIGHTGREY);
       tft.fillScreen(TFT_LIGHTGREY);
       tft.drawString("THE DOME", 40, 0, 2); //viewable on Screen 
       tft.drawXBitmap(0, 30, mit_608_dome, dome_608_width, dome_608_height, TFT_RED);
       state = 1;
     }
     if (state == 2){ // change to text and state to 3
       tft.setTextColor(TFT_RED, TFT_LIGHTGREY);
       tft.fillScreen(TFT_LIGHTGREY);
       tft.drawString("6.08 Lab ONE!!", 40, 0, 2);
       state = 3;
     } 
  }
  delay(3);
}
