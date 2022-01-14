#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include<math.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

WiFiClientSecure client; //global WiFiClient Secure object
WiFiClient client2; //global WiFiClient Secure object

//TODO
//-Add selection with arbitrary values
//-Add GET options and parsing

//Init Global Variables
//Button1 State Machine
const uint8_t IDLE = 0;
const uint8_t DOWN = 1;
const uint8_t UP = 2;
uint8_t state = IDLE;
uint8_t state2 = IDLE;
uint32_t idle_time = 0;
uint32_t idle2_time = 0;
bool init_seq = true;

//Screen Toggling
const int8_t LIST = 1;
const int8_t MENU = -1;
int8_t toggle = LIST;
int counter = 0;
char one[100];
char two[100];
char three[100];
char four[100];
char five[100];
char six[100];
char seven[100];
char temp[100];

/* CONSTANTS */
//Prefix to POST request:
const char PREFIX[] = "{\"wifiAccessPoints\": ["; //beginning of json body
const char SUFFIX[] = "]}"; //suffix to POST request
const char API_KEY[] = "AIzaSyCwyynsePu7xijUYTOgR7NdVqxH2FAG9DQ"; //don't change this and don't share this


//Network Setup
char*  SERVER = "googleapis.com";  // Server URL

const int MAX_APS = 5;
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
const uint16_t JSON_BODY_SIZE = 3000;
char request[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
char json_body[JSON_BODY_SIZE];

//General Setup
const uint8_t BUTTON1 = 5; //pin connected to button
const uint8_t BUTTON2 = 19; //pin connected to button


void setup() {
  // put your setup code here, to run once:
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
  pinMode(BUTTON2, INPUT_PULLUP); //set input pin as an input!
  idle_time = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t button1 = digitalRead(BUTTON1);
  uint8_t button2 = digitalRead(BUTTON2);
  button1_checker(button1);
  button2_checker(button2);
  lcd_update();
}

//lcd function
void lcd_update() {
  //check with toggle and updates the time
  if (toggle == LIST) {
    tft.setCursor(0,0,1);
    tft.println(one);
    tft.println(two);
    tft.println(three);
    tft.println(four);
    tft.println(five);
    tft.println(six);
    tft.println(seven);
  }
  else {
    tft.setCursor(0,0,1);
    tft.print("Current temperature");
    should_I(1);
    tft.print("Current time");
    should_I(2);
    tft.print("Current date");
    should_I(3);
    tft.print("Current visibility");
    should_I(4);
    tft.print("Current humidity");
    should_I(5);
    tft.print("Current pressure");
    should_I(6);
    tft.print("exit");
    should_I(0);
  }
}

void should_I(int num) {
  if (counter == num){
    tft.println("*");
  }
  else {
    tft.println(" ");
  }
}

//button function
void button1_checker(uint8_t button1) {
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
        Serial.println(toggle);
        if (toggle == LIST) { // reset the counter
          if (counter != 0) {
            Serial.println("here");
            sprintf(seven, "%s", six);
            sprintf(six, "%s", five);
            sprintf(five, "%s", four);
            sprintf(four, "%s", three);
            sprintf(three, "%s", two);
            sprintf(two, "%s", one);
            wifi_update();
          }
          counter = 0;
        }
      }
      break;
  }
}

void button2_checker(uint8_t button2) {
  //checks button and updatess the counter accordingly
  switch (state2) {
    case IDLE:
      if (button2 == 1) { //Pressed
        state2 = DOWN;
      }
      break;
    case DOWN:
      if (button2 != 1) { //lifted
        state2 = UP;
        idle2_time = millis();
      }
      break;
    case UP:
      if (millis() - idle2_time > 100) { //buffer time passed
        state2 = IDLE;
        if (toggle == MENU){ //we can start shifting through
          counter++;
          if (counter > 6) {
            counter = 0;
          }
        }
      }
      break;
  }
}

void wifi_update() {
  int offset = sprintf(json_body, "%s", PREFIX);
  int n = WiFi.scanNetworks(); //run a new scan. could also modify to use original scan from setup so quicker (though older info)
  Serial.println("scan done");
  int max_aps = max(min(MAX_APS, n), 1);
  for (int i = 0; i < max_aps; ++i) { //for each valid access point
    uint8_t* mac = WiFi.BSSID(i); //get the MAC Address
    offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE-offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); //generate the query
    if(i!=max_aps-1){
      offset +=sprintf(json_body+offset,",");//add comma between entries except trailing.
    }
  }   
  sprintf(json_body + offset, "%s", SUFFIX);
  Serial.println(json_body);
  int len = strlen(json_body);
  // Make a HTTP request:
  Serial.println("SENDING REQUEST");
  request[0] = '\0'; //set 0th byte to null
  offset = 0; //reset offset variable for sprintf-ing
  offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
  offset += sprintf(request + offset, "Host: googleapis.com\r\n");
  offset += sprintf(request + offset, "Content-Type: application/json\r\n");
  offset += sprintf(request + offset, "cache-control: no-cache\r\n");
  offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
  offset += sprintf(request + offset, "%s\r\n", json_body);
  do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  Serial.println("-----------");
  Serial.println(response);
  Serial.println("-----------");
  char * ptr;
  char opbr[5];
  sprintf(opbr,"%c",'{');
  char clbr[5];
  sprintf(clbr,"%c",'}');
  ptr = strtok (response,opbr);
  ptr = strtok (NULL,clbr);
  char json[100];
  sprintf(json,"{%s}", ptr);
  Serial.println(json);
  DynamicJsonDocument doc2(2048);
  deserializeJson(doc2, json);
  double latitude = doc2["location"]["lat"];
  double longitude = doc2["location"]["lng"];
  // Make the second HTTP request:
  Serial.println("SENDING REQUEST");
  request[0] = '\0'; //set 0th byte to null
  offset = 0; //reset offset variable for sprintf-ing
  offset += sprintf(request + offset, "GET /data/2.5/weather?lat=%f2&lon=%f&appid=258b59b4f9ff2a473f5779da1daab855 HTTP/1.1\r\n", latitude, longitude);
  offset += sprintf(request + offset, "Host: api.openweathermap.org\r\n");
  offset += sprintf(request + offset, "\r\n");
  do_http_request("api.openweathermap.org", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  Serial.println("-----------");
  Serial.println(response);
  Serial.println("-----------");
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, response);
  if (counter == 1) {
    double temperature = doc["main"]["temp"];
    temperature = (temperature - 273.15)*(9/5)+32;
    sprintf(temp, "The temp is %f deg F", temperature);
  }
  if (counter == 2) {
    int times = doc["dt"];
    int shift = doc["timezone"];
    times = times + shift;
    times = times % 86400;
    //now time is time since 12:00 am
    int hours = 0;
    int mins = 0;
    while(times >= 3600) {
      hours++;
      times = times - 3600;
    }
    while(times >= 60) {
      mins++;
      times = times - 60;
    }
    sprintf(temp, "The time is %d:%d:%d", hours,mins,times);
  }
  if (counter == 3) {
    int times = doc["dt"];
    int shift = doc["timezone"];
    times = times + shift;
    int year = 2021;
    times = times - 1609477200;
    char month[5];
    int day = 1;
    while (times >= 31536000) {
      year++;
      times = times - 31536000;
    }
    //check month
    if (times >= 2678400) { //is in january
      times = times - 2678400;
      if (times >= 2419200) { //is it february
        times = times - 2419200;
        if (times >= 2678400) { //is it march
          times = times - 2678400;
          if (times >= 2592000) {
            times = times - 2592000;
            if (times >= 2678400) {
              times = times - 2678400;
              if (times >= 2592000) {
                times = times - 2592000;
                if (times >= 2678400) {
                  times = times - 2678400;
                  if (times >= 2678400) {
                    times = times - 2678400;
                    if (times >= 2592000) {
                      times = times - 2592000;
                      if (times >= 2678400) {
                        times = times - 2678400;
                        if (times >= 2592000) {
                          times = times - 2592000;
                          sprintf(month, "Dec");
                        }
                        else {
                          sprintf(month, "Nov");
                        }
                      }
                      else {
                        sprintf(month, "Oct");
                      }
                    }
                    else {
                      sprintf(month, "Sep");
                    }
                  }
                  else {
                    sprintf(month, "Aug");
                  }
                }
                else {
                  sprintf(month, "July");
                }
              }
              else {
                sprintf(month, "June");
              }
            }
            else {
              sprintf(month, "May");
            }
          }
          else {
            sprintf(month, "Apr");
          }
        }
        else {
          sprintf(month, "Mar");
        }
      }
      else {
        sprintf(month, "Feb");
      }
    }
    else {
      sprintf(month, "Jan");
    }
    //check day, starts at 2 to account for first case and final case
    while (times > 86400) {
      day++;
      times = times - 86400;
    }
    sprintf(temp, "The date is %s, %d, %d", month, day, year);
  }
  if (counter == 4) {
    int visibility = doc["weather"]["visibility"];
    sprintf(temp, "The visibility is %d", visibility);
  }
  if (counter == 5) {
    int humidity = doc["main"]["humidity"];
    sprintf(temp, "The humidity is %d percent", humidity);
  }
  if (counter == 6) {
    double pressure = doc["main"]["pressure"];
    sprintf(temp, "The pressure is %f hPa", pressure);
  }
  sprintf(one, "%s", temp);
}
