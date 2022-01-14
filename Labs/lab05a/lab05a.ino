#include <SPI.h>
#include <TFT_eSPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();

//we'll use a different pin for each PWM so we can run them side-by-side
const uint8_t PIN1 = 26; //pin we use for software PWM
const uint8_t PIN2 = 25; //pin we use for hardware PWM

const uint32_t PWM_CHANNEL = 0; //hardware pwm channel used in secon part


class PWM_608
{
  public:
    int pin; //digital pin class controls
    int period; //period of PWM signal (in milliseconds)
    int on_amt; //duration of "on" state of PWM (in milliseconds)
    int invert; //if invert==1, the "HIGH" value of the PWM signal should be 0 and the "LOW" value should be 1
    //DECLARE  three class methods below:
    PWM_608(int op, float frequency, int invert_in); //constructor op = output pin, frequency=freq of pwm signal in Hz, invert_in, if output should be inverted
    void set_duty_cycle(float duty_cycle); //sets duty cycle to be value between 0 and 100
    void update(); //updates state of system based on millis() and duty cycle
};

//DEFINE the three class methods below:
PWM_608::PWM_608(int op, float frequency, int invert_in){
  pin = op;
  period = 1000/frequency;
  invert = invert_in;
  on_amt = 0;
}

void PWM_608::update(){
  if (millis()%period >= floor(on_amt)) { //should be off
    if (invert == 1) {
      digitalWrite(pin,1);
    }
    else{
      digitalWrite(pin,0);
    }
  }
  else {//should be on
    if (invert == 1) {
      digitalWrite(pin,0);
    }
    else{
      digitalWrite(pin,1);
    }
  }
    
    
}
void PWM_608::set_duty_cycle(float duty_cycle){//sets duty cycle to be value between 0 and 100
  if (duty_cycle > 100.0) {
    duty_cycle = 100.0;
  }
  if (duty_cycle < 0.0) {
    duty_cycle = 0.0;
  }
  //duty cycle is 'percent' of the period we want on
  on_amt = (duty_cycle/100)*period;
}

PWM_608 backlight(PIN1, 50, 1); //create instance of PWM to control backlight on pin 1, operating at 50 Hz

void setup() {
  Serial.begin(115200); // Set up serial port

  //For Lab05a second part, you will uncomment the next two lines
  ledcSetup(PWM_CHANNEL, 50, 12);//create pwm channel, @50 Hz, with 12 bits of precision
  ledcAttachPin(25, PWM_CHANNEL); //link pwm channel to IO pin 25

  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_WHITE); //fill background
  tft.setTextColor(TFT_BLACK, TFT_WHITE); //set color for font
  pinMode(PIN1, OUTPUT); //controlling TFT with our PWM controller (first part of lab)
  pinMode(PIN2, OUTPUT); //controlling TFT with hardware PWM (second part of lab)
  backlight.set_duty_cycle(30); //initialize the software PWM to be at 30%

  tft.setCursor(0, 0, 2); //set cursor, font size 1
  tft.printf("Period: %d ms  \nOn amount: %d ms   ", backlight.period, backlight.on_amt); //print
}

void loop() {
  if (Serial.available()) {
    char buffer[20];
    Serial.readBytes(buffer, 20);
    int value = atoi(buffer);
    Serial.printf("updating to %d% \n", value);
    backlight.set_duty_cycle(value);
    tft.setCursor(0, 0, 2); //set cursor, font size 1
    tft.printf("Period: %d ms  \nOn amount: %d ms   ", backlight.period, backlight.on_amt); //print
    //------------
    //In last part of lab05a:
    //Add line(s) here to implement a Hardware PWM on pin 25 here (keep your sofrware PWM running as well)
    //remember the hardware PWM considers HIGH to be 1 and LOW to be 0. That may not work when driving a P-channel FET like in lab05a
    //------------
    //with resolution 12, the max is at 4095
    
    if (value > 100.0) {
      value = 100.0;
    }
    if (value < 0.0) {
      value = 0.0;
    }
    ledcWrite(PWM_CHANNEL, 4096 - value*40.96);
  }
  random_delay(); //cause a random and relatively rare pause of a few milliseconds
  backlight.update(); //call backlight.update() as fast as possible on our software PWM object
}

//This function is used to simulate some other task with an unpredictable delay
//it can stand in for a sensor measurement, or an http GET or http POST
//we could do this for real, but since we're not really using WiFi today we're avoiding it so
//things compile faster :)
void random_delay() {
  int random_val = random(0, 10000); //get a random number between 0 and 100
  if (random_val > 9998) delay(random(0, 4));
}
