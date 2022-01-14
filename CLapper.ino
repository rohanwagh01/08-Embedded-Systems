#include <SPI.h>
#include <TFT_eSPI.h>

//Setup:
TFT_eSPI tft = TFT_eSPI();
const int AUDIO_IN = A0; //pin where microphone is connected
float values[100]; //used for remembering an arbitrary number of old previous values
int indx = 0;
int order = 20;
const int IDLES = 0;
const int CLAP_ONE_WAIT = 1;
const int WAIT_FOR_CLAP_TWO = 2;
const int CLAP_TWO_WAIT = 3;
int state = IDLES;

uint32_t clap_timer = 0;
uint32_t time_between = 0;
int CLAP_TIME = 150;
int screen = -1; //off
uint32_t just_changed_timer = 0;
int COOLDOWN = 500;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);               // Set up serial port
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  delay(100); //wait a bit (100 ms)
  for (int i=0; i<50; i++){ 
    values[i] = 0.0;
  }
  just_changed_timer = millis()+500;
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = 0;
  value = analogRead(AUDIO_IN);
  int average = int(averaging_filter(value, values, order, &indx));
  //scaling to provide smooth values
  average = average - 1400.0;
  if (average < 350) { //within threshold
    average = 0;
  }
  else {
    average = 1;
  }
  
  if (millis()-just_changed_timer > 500) {//prevents spamming on off on off
    switch (state) {
      case IDLES:
        if (average == 1) {
          state = CLAP_ONE_WAIT;
          clap_timer = millis();
          time_between = millis();
        }
        break;
      case CLAP_ONE_WAIT:
        if (millis()-clap_timer > CLAP_TIME) {
          if (average == 1){
            state = IDLES;
          }
          else {
            state = WAIT_FOR_CLAP_TWO;
          }
        }
        break;
      case WAIT_FOR_CLAP_TWO:
        if (millis()-time_between > 1000) { //second has passed without clap
          state = IDLES;
        }
        if (average == 1) {
          state = CLAP_TWO_WAIT;
          clap_timer = millis();
        }
        break;
      case CLAP_TWO_WAIT:
        if (millis()-clap_timer > CLAP_TIME) {
          if (average == 1){
            state = WAIT_FOR_CLAP_TWO;
          }
          else {
            if (millis()-time_between > 250) {
              screen = -screen;
              if (screen == 1){
                tft.fillScreen(TFT_GREEN);
              }
              else{
                tft.fillScreen(TFT_BLACK);
              }
              state = IDLES;
            }
            else {//clap before 0.25 sec
              state = IDLES;
            }
          }
        }
        break;
    }
  }
  Serial.println(screen);
}


float averaging_filter(float input, float* stored_values, int order, int* index) {
    // replace the input and update index
    if(*index>49) {
      *index = *index-50;
    }
    stored_values[*index] = input;
    // now go backwards from the index 
    float multi = 1/((float)(1+order));
    float sum = 0.0;
    for(int i = *index; *index-i < order+1; i--) {
      if (i < 0){
        sum += multi*(stored_values[50+i]);
      }
      else {
        sum += multi*(stored_values[i]);
      }
    }
    *index = *index + 1;
    return sum;
}
