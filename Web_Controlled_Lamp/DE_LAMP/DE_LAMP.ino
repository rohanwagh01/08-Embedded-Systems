#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <ArduinoJson.h>


char network[] = "MIT";
char password[] = "";

uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //ms to wait between posting step


const uint16_t OUT_BUFFER_SIZE = 200; //size of buffer to hold HTTP response
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

uint32_t get_update_timer;

//led control
uint8_t RED = 33;
uint8_t GREEN = 27;
uint8_t BLUE = 32;

//PWM Channels
uint8_t RED_pmw = 0;
uint8_t GREEN_pmw = 2;
uint8_t BLUE_pmw = 1;


void setup() {
  Serial.begin(115200); //for debugging if needed.
  WiFi.begin(network, password); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  get_update_timer = millis();
  ledcSetup(RED_pmw, 200, 8);
  ledcSetup(GREEN_pmw, 200, 8);
  ledcSetup(BLUE_pmw, 200, 8);
  ledcAttachPin(RED, RED_pmw);
  ledcAttachPin(GREEN, GREEN_pmw);
  ledcAttachPin(BLUE, BLUE_pmw);
}

void loop() {
  if (millis()-get_update_timer > 3000){
    get_update_timer = millis();
    char request_buffer[200];
    sprintf(request_buffer, "GET /sandbox/sc/rwagh/dex05/lamp_server_side_code.py HTTP/1.1\r\n");
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer, "\r\n"); //new line from header to body
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    //NOW check if the coor has changed and if we need to change the LED
    Serial.println(response_buffer); //viewable in Serial Terminal
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response_buffer);
    int red_val = doc["red"];
    int green_val = doc["green"];
    int blue_val = doc["blue"];
    Serial.println(red_val);
    Serial.println(green_val);
    Serial.println(blue_val);
    ledcWrite(RED_pmw, red_val);
    ledcWrite(GREEN_pmw, green_val);
    ledcWrite(BLUE_pmw, blue_val);
  }
}
