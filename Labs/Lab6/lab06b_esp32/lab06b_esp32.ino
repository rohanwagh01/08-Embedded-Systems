#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <BMP280_DEV.h>

BMP280_DEV bmp280(21, 22);       // Instantiate (create) a BMP280 object and set-up for I2C on ESP32's pins 21 and 22


const char USER[] = "rwagh"; //CHANGE YOUR USER VARIABLE!!!
const char POST_URL[] = "POST http://608dev-2.net/sandbox/sc/rwagh/lab06/server_6.py HTTP/1.1\r\n"; //CHANGE THIS TO YOUR TEAM'S URL
char network[] = "MIT";
char password[] = "";


/* Having network issues since there are 50 MIT and MIT_GUEST networks?. Do the following:
    When the access points are printed out at the start, find a particularly strong one that you're targeting.
    Let's say it is an MIT one and it has the following entry:
   . 4: MIT, Ch:1 (-51dBm)  4:95:E6:AE:DB:41
   Do the following...set the variable channel below to be the channel shown (1 in this example)
   and then copy the MAC address into the byte array below like shown.  Note the values are rendered in hexadecimal
   That is specified by putting a leading 0x in front of the number. We need to specify six pairs of hex values so:
   a 4 turns into a 0x04 (put a leading 0 if only one printed)
   a 95 becomes a 0x95, etc...
   see starting values below that match the example above. Change for your use:
   Finally where you connect to the network, comment out
     WiFi.begin(network, password);
   and uncomment out:
     WiFi.begin(network, password, channel, bssid);
   This will allow you target a specific router rather than a random one!
*/
uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.


const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;
uint32_t posting_timer = 0;
float x, y, z; //variables for grabbing x,y,and z values

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //ms to wait between posting step


const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response



void setup() {
  Serial.begin(115200); //begin serial comms
  while (!Serial);
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
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  bmp280.begin(0x77);                             //initialize bmp280 module
  bmp280.setTimeStandby(TIME_STANDBY_2000MS);     // Set the standby time to 1 sec
  bmp280.startNormalConversion();                 // Start BMP280 continuous conversion in NORMAL_MODE
  posting_timer = millis();
}


void loop() {
  float temperature; //variable for temperature
  float pressure;    //variable for pressure
  float altitude;    //variable for alititude, but tbh, this is pretty unreliable without calibration
  if (bmp280.getMeasurements(temperature, pressure, altitude)) {
    if (millis() - posting_timer > POSTING_PERIOD) {
      posting_timer = millis();
      Serial.printf("temp: %f *C\t\tpressure: %f hPa\t\taltitute: %f m\n", temperature, pressure, altitude);
      char body[100]; //for body
      sprintf(body, "user=%s&temperature=%f&pressure=%f", USER, temperature, pressure); //generate body, posting temp, humidity to server
      int body_len = strlen(body); //calculate body length (for header reporting)
      sprintf(request_buffer, POST_URL);
      strcat(request_buffer, "Host: 608dev-2.net\r\n");
      strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
      strcat(request_buffer, "\r\n"); //new line from header to body
      strcat(request_buffer, body); //body
      strcat(request_buffer, "\r\n"); //new line
      Serial.println(request_buffer);
      do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      Serial.println(response_buffer); //viewable in Serial Terminal
    }
  }
}
