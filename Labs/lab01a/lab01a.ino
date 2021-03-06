#include<string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "dome_image.h"
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
const uint32_t PAUSE = 1000;
uint32_t timer;
uint8_t draw_state;
const uint8_t NUM_DRAW_STATES = 5;

void setup(void) {
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  Serial.begin(115200);
}

void loop() {
  draw_stuff();
}


/*--------------------
 * draw_stuff Function:
 * 
 * Arguments:
 *    None
 * Return Values:
 *    None
 * Five-state State machine that cycles through various drawings on TFT screen.
 * Uses global variables draw_state (to represent state), and timer, to keep track of transition times
 * Only draws image on change of state!
 * State 0: Simple message!
 * State 1: Draw a one-bit monochrome image of MIT Dome and a title in MIT Colors
 * State 2: Draw Black background with a filled and empty circle of various sizes
 * State 3: Draw yellow background with two rectangles
 * State 4: Draw a bunch of lines green lines on a white backdrop using a for loop
 */

void draw_stuff(){
  if (millis()-timer > PAUSE){ //has PAUSE milliseconds elapsed since last change?
    timer = millis(); //store time of switch for future comparison
    draw_state +=1; //increment draw state
    draw_state %= NUM_DRAW_STATES;  //mod state (keep within 0 to NUM_DRAW_STATES -1
    if (draw_state==0){
      tft.fillScreen(TFT_BLACK); 
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("6.08 Test Patterns",0,0,1);
    }else if(draw_state==1){
      tft.setTextColor(TFT_RED, TFT_LIGHTGREY);
      tft.fillScreen(TFT_LIGHTGREY);
      tft.drawString("THE DOME", 40, 0, 2); //viewable on Screen 
      tft.drawXBitmap(0, 30, mit_608_dome, dome_608_width, dome_608_height, TFT_RED);
    }else if (draw_state==2){
      tft.fillScreen(TFT_BLACK);
      tft.drawCircle(5,10,3,TFT_GREEN);
      tft.fillCircle(64,80,30,TFT_PURPLE);
    }else if (draw_state==3){
      tft.fillScreen(TFT_YELLOW);
      tft.drawRect(10,10,5,30,TFT_BLUE);
      tft.fillRect(50,50,20,20,TFT_RED);
    }else if (draw_state==4){
      tft.fillScreen(TFT_WHITE);
      for (int i=0; i<160; i+=4){
        tft.drawLine(0, 0, 128, i, TFT_GREEN);
      }
    }
  }
}
