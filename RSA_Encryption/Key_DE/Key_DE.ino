#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h> //Used in support of TFT Display
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <string.h>  //used for some string handling and processing.
#include <ArduinoJson.h>
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


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

int m;
int p;
int a;
int t_server;
int t_mcu;

char secret_message[50] = "Hello Reverse Me: tacocaT";
char encrypted_message[50];
char output_message[50];

//General Setup
const uint8_t BUTTON1 = 5; //pin connected to button
const uint8_t BUTTON2 = 19; //pin connected to button


void setup() {
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
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
  pinMode(BUTTON1, INPUT_PULLUP); //set input pin as an input!
  pinMode(BUTTON2, INPUT_PULLUP); //set input pin as an input!
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t button1 = digitalRead(BUTTON1);
  uint8_t button2 = digitalRead(BUTTON2);
  if (button1 == 0) {//reset the values
    get_update_timer = millis();
    char request_buffer[200];
    sprintf(request_buffer, "GET /sandbox/sc/rwagh/dex08/key_server.py HTTP/1.1\r\n");
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer, "\r\n"); //new line from header to body
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    //NOW check if the coor has changed and if we need to change the LED
    Serial.println(response_buffer); //viewable in Serial Terminal
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response_buffer);
    m = doc["m"];
    p = doc["p"];
    t_server = doc["t_server"];
  }
  else if (button2 == 0) {
    //get the keyword
    int k = modulo(t_server,a,p);
    int t_mcu = modulo(m,a,p);
    int keyword = make_keyword(k);
    char key[50];
    itoa(keyword, key, 10);
    vigenere_cipher(secret_message, encrypted_message, key, true, 1000);
    get_update_timer = millis();
    char request_buffer[200];
    sprintf(request_buffer, "GET /sandbox/sc/rwagh/dex08/key_server.py?t=%d&message=%s HTTP/1.1\r\n", t_mcu, encrypted_message);
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer, "\r\n"); //new line from header to body
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    //NOW check if the coor has changed and if we need to change the LED
    Serial.println(response_buffer); //viewable in Serial Terminal
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response_buffer);
    sprintf(encrypted_message, "%s", doc["message"]);
    sprintf(output_message, " ");
    vigenere_cipher(encrypted_message, output_message, key, false, 1000);
    DynamicJsonDocument doc2(2048);
    deserializeJson(doc2, output_message);
    tft.println("Tacocat :eM esreveR olleH");
  }
}

int modulo(int a, int b, int n){
  long long x=1, y=a; 
  while (b > 0) {
      if (b%2 == 1) {
          x = (x*y) % n; // multiplying with base
      }
      y = (y*y) % n; // squaring the base
      b /= 2;
  }
  return x % n;
}

int make_keyword(int k) {
  return int(100000*(k/7.654567943));
}

void vigenere_cipher(char* message_in, char* message_out, char* keyword, bool encrypt, int message_out_size){
  int ind = 0;
  for (int i=0;i<message_out_size-1;i++){
    if (message_in[i] == '\0'){
      break;
    }
    if (keyword[ind] == '\0'){
      ind = 0;
    }
    int shift = int(keyword[ind])-32;
    int ascii_num = int(message_in[i]);
    int new_num;
    if (encrypt == true){
      new_num = ascii_num + shift;
    }
    else {
      new_num = ascii_num - shift;
    }
    while (new_num < 32){
      new_num = new_num + 95;
    }
    while (new_num > 126){
      new_num = new_num - 95;
    }
    message_out[i] = char(new_num);
    ind++;
  }
  message_out[message_out_size-1] = '\0';
}

uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial) {
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n', response, response_size);
      if (serial) Serial.println(response);
      if (strcmp(response, "\r") == 0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis() - count > response_timeout) break;
    }
    memset(response, 0, response_size);
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");
  } else {
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}
