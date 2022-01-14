#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

const int UPDATE_PERIOD = 20;
const uint8_t PIN_1 = 5; //button 1
const uint8_t PIN_2 = 0; //button 2
uint32_t primary_timer;
float sample_rate = 2000; //Hz
float sample_period = (int)(1e6 / sample_rate);

const uint8_t BAR_WIDTH = 30;
const uint8_t BAR_X = 50;
const uint8_t SCREEN_YMAX = 159;
int render_counter;
uint16_t raw_reading;
uint16_t scaled_reading_for_lcd;
uint16_t old_scaled_reading_for_lcd;
float scaled_reading_for_serial;

void setup() {
  Serial.begin(115200);               // Set up serial port
  pinMode(PIN_1, INPUT_PULLUP);
  primary_timer = micros();
  render_counter = 0;
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set col
}

void loop() {
  while (!digitalRead(PIN_1)) {
    render_counter++;
    if (render_counter % UPDATE_PERIOD == 0) {
      raw_reading = analogRead(A0);
      scaled_reading_for_lcd = raw_reading * 159 / 4096; //MUST CHANGE
      Serial.println(scaled_reading_for_lcd);
      if (old_scaled_reading_for_lcd > scaled_reading_for_lcd) {
        tft.fillRect(BAR_X, SCREEN_YMAX - old_scaled_reading_for_lcd, BAR_WIDTH, (old_scaled_reading_for_lcd - scaled_reading_for_lcd), TFT_BLACK);
      }
      scaled_reading_for_serial = raw_reading * 3.3 / 4096;
      Serial.println(scaled_reading_for_serial);
      tft.fillRect(BAR_X, SCREEN_YMAX - scaled_reading_for_lcd, BAR_WIDTH, scaled_reading_for_lcd, TFT_GREEN);
      old_scaled_reading_for_lcd = scaled_reading_for_lcd; //remember, remember the scaled_reading_for_lcd 
    }
    while (micros() > primary_timer && micros() - primary_timer < sample_period); //prevents rollover glitch every 71 minutes...not super needed
    primary_timer = micros();
  }
}
