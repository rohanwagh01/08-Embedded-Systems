#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include<math.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

//Init Global Variables
//Button State Machine
const uint8_t IDLE = 0;
const uint8_t DOWN = 1;
const uint8_t UP = 2;
uint8_t state = IDLE;
uint32_t idle_time = 0;
bool init_seq = true;

//Screen Toggling
const int8_t DIG = 1;
const int8_t AN = -1;
int8_t toggle = DIG;
int values[10];
int center_x = 64;
int center_y = 80;

//Time setup
uint8_t seconds = 0;
uint8_t hours = 0;
uint8_t minutes = 0;
uint32_t get_request_timer = 0; 
uint32_t update_timer = 0; //a timer in millilseconds to update seconds

//Network Setup
char network[] = "MIT";
char password[] = "";
uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41};
//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int GETTING_PERIOD = 2000; //periodicity of getting a number fact.
const int BUTTON_TIMEOUT = 1000; //button timeout in milliseconds
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

//General Setup
const uint8_t BUTTON1 = 5; //pin connected to button
MPU6050 imu; //imu object called, appropriately, imu


void setup(){
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  Serial.begin(115200); //begin serial comms


  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t* cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++) {
        Serial.print(*cc, HEX);
        if (k != 5) Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100); //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(network, password);
  //if using channel/mac specification for crowded bands use the following:
  //WiFi.begin(network, password, channel, bssid);
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count<6) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n",WiFi.localIP()[3],WiFi.localIP()[2],
                                            WiFi.localIP()[1],WiFi.localIP()[0], 
                                          WiFi.macAddress().c_str() ,WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  pinMode(BUTTON1, INPUT_PULLUP); //set input pin as an input!
  idle_time = millis();
}

void loop() {
  uint8_t button1 = digitalRead(BUTTON1);
  if (millis()-update_timer > 1000) { // more than a second has passed, ok for while since it terminates quickly
    seconds++;
    tft.fillScreen(TFT_BLACK); //fill background
    update_timer = millis();
  }
  //now check if minutes or hours needs to be updated
  while (seconds >= 60) {
    minutes++;
    seconds -= 60;
  }
  while (minutes >= 60) {
    hours++;
    minutes -= 60;
  }
  while (hours >= 24) { //if over 24 back to 0
    hours -= 24;
  }
  if (millis()-get_request_timer > 60000 || init_seq == true) {//once every minute
    //reset the hours, minutes, seconds from the HTTP GEt request
    //initialize 
    init_seq = false;
    //formulate GET request...first line:
    sprintf(request_buffer, "GET http://iesc-s3.mit.edu/esp32test/currenttime HTTP/1.1\r\n");
    strcat(request_buffer, "Host: iesc-s3.mit.edu\r\n"); //add more to the end
    strcat(request_buffer, "\r\n"); //add blank line!
    //submit to function that performs GET.  It will return output using response_buffer char array
    do_http_GET("iesc-s3.mit.edu", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    Serial.println(response_buffer); //print to serial monitor
    //split string into the proper values
    int_extractor(response_buffer,values,':');
    hours = values[0];
    minutes = values[1];
    seconds = values[2];
    get_request_timer = millis();
    update_timer = millis();
  }
  button_checker(button1);
  lcd_update(hours,minutes,seconds);
}

//extractor
void int_extractor(char* data_array, int* output_values, char delimiter){
  char * pch;
  char space_token[5];
  sprintf(space_token,"%c",' ');
  pch = strtok(data_array,space_token);
  int count = 0;
  char token[5];
  sprintf(token,"%c",delimiter);
  while (pch != NULL)
  {
    pch = strtok(NULL, token);
    if (pch == NULL) {
      break;
    }
    output_values[count] = atoi(pch);
    count++;
  }
}

//button function
void button_checker(uint8_t button1) {
  //checks button and updatess state and toggle accordingly
  switch (state) {
    case IDLE:
      if (button1 == 1) { //Pressed
        state = DOWN;
      }
      break;
    case DOWN:
      if (button1 != 1) { //lifted
        state = UP;
        idle_time = millis();
      }
      break;
    case UP:
      if (millis() - idle_time > 200) { //buffer time passed
        state = IDLE;
        tft.fillScreen(TFT_BLACK); //fill background
        toggle = -toggle;
      }
      break;
  }
}

//lcd function
void lcd_update(uint8_t h,uint8_t m,uint8_t s) {
  //check with toggle and updates the time
  if (toggle == DIG) {
    tft.setTextSize(1);
    tft.setCursor(0,0,1);
    tft.println("Digital Clock:   ");
    tft.setTextSize(2.5);
    tft.println();
    tft.println();
    if (h > 12) {
      tft.print(h-12);
    }
    else {
      tft.print(h);
    }
    tft.print(":");
    tft.print(m);
    tft.print(":");
    tft.print(s);
    if (h > 12) {
      tft.print(" PM   ");
    }
    else {
      tft.print(" AM   ");
    }
  }
  else {
    tft.setTextSize(1);
    tft.setCursor(0,0,1);
    tft.println("Analog Clock:   ");
    if (h>12) {
      draw_line(30, (double)(h-12 + (m + s/60)/60)/12);
    }
    else {
      draw_line(28, (double)(h + (m + s/60)/60)/12);
    }
    draw_line(44, (double)(m + s/60)/60);
    draw_line(55, (double)s/60);
  }
}

//analog
void draw_line(uint8_t radius, float time_ratio) {
  double theta = time_ratio*2*M_PI;
  int d_x = (int)(sin(theta)*radius);
  int d_y = -(int)(cos(theta)*radius);
  tft.drawLine(center_x, center_y, center_x + d_x, center_y + d_y, TFT_GREEN);
}


//GET Request

void do_http_GET(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}        
