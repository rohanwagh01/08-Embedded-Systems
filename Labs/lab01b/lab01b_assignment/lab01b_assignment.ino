#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.

const uint8_t IDLE = 0; //example definition
const uint8_t DOWN = 1; //example...
const uint8_t UP = 2; //change if you want!
bool first_DOWN = false;
bool first_UP = false;
bool first_IDLE = false;
bool button_HELD = false;
//add more if you want!!!

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int GETTING_PERIOD = 2000; //periodicity of getting a number fact.
const int BUTTON_TIMEOUT = 1000; //button timeout in milliseconds
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response


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


const int BUTTON = 5; //pin connected to button 
uint8_t state;  //system state (feel free to use)
uint8_t num_count; //variable for storing the number of times the button has been pressed before timeout
unsigned long timer;  //used for storing millis() readings.

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
  pinMode(BUTTON, INPUT_PULLUP); //set input pin as an input!
  state = IDLE; //start system in IDLE state!
}

void loop(){
  number_fsm(digitalRead(BUTTON)); //Call our FSM every time through the loop.
}


/*------------------
 * number_fsm Function:
 * Use this to implement your finite state machine. It takes in an input (the reading from a switch), and can use
 * state as well as other variables to be your state machine.
 */

void number_fsm(uint8_t input){
  //your logic here!
  //use your inputs and the system's state to update state and perform actions
  //This function should be non-blocking, meaning there should be minimal while loops and other things in it!
  //Feel free to call do_http_GET from this function
  //state variable globally defined at top
  if (input) { // button unpressed
    if (state == DOWN) { // only change if the state is down
      state = UP;
      first_DOWN = true;
      button_HELD = false;
    }
  }
  else {
    state = DOWN;
    first_UP = true;
  }
  switch(state){
    case IDLE:
      //we want to reset the counters and other variables, this is the reset point
      if (first_IDLE == true) { // we want to update the first time
        //formulate GET request...first line:
        sprintf(request_buffer, "GET http://iesc-s3.mit.edu/esp32test/currenttime HTTP/1.1\r\n", num_count);
        strcat(request_buffer, "Host: iesc-s3.mit.edu\r\n"); //add more to the end
        strcat(request_buffer, "\r\n"); //add blank line!
        //submit to function that performs GET.  It will return output using response_buffer char array
       do_http_GET("iesc-s3.mit.edu", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
       Serial.println(response_buffer); //print to serial monitor
       tft.fillScreen(TFT_BLACK); //black out TFT Screen
       tft.setCursor(0, 0, 1);
       tft.println(response_buffer);
       //reset count
       num_count = 0;
       first_DOWN = true;
       first_IDLE = false;
      }
      break; //don't forget break statements
    case DOWN:
      //another state...etc...
      // wait for a small period and recheck
      if (first_DOWN == true) {
        timer = millis();
        first_DOWN = false;
      }
      if (millis()-timer > 10 && button_HELD == false) { // at least ten millisecond hold
        num_count = num_count + 1;
        button_HELD = true;
      }
      break;
    case UP:
      //another state!
      // start timer
      if (first_UP == true) {
        timer = millis();
        first_UP = false;
      }
      if (millis()-timer > BUTTON_TIMEOUT) {
        state = IDLE;
        first_IDLE = true;
      }
      break;
  }
  
}



/*----------------------------------
 * char_append Function:
 * Arguments:
 *    char* buff: pointer to character array which we will append a
 *    char c: 
 *    uint16_t buff_size: size of buffer buff
 *    
 * Return value: 
 *    boolean: True if character appended, False if not appended (indicating buffer full)
 */
uint8_t char_append(char* buff, char c, uint16_t buff_size) {
        int len = strlen(buff);
        if (len>buff_size) return false;
        buff[len] = c;
        buff[len+1] = '\0';
        return true;
}

/*----------------------------------
 * do_http_GET Function:
 * Arguments:
 *    char* host: null-terminated char-array containing host to connect to
 *    char* request: null-terminated char-arry containing properly formatted HTTP GET request
 *    char* response: char-array used as output for function to contain response
 *    uint16_t response_size: size of response buffer (in bytes)
 *    uint16_t response_timeout: duration we'll wait (in ms) for a response from server
 *    uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
 * Return value:
 *    void (none)
 */
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
