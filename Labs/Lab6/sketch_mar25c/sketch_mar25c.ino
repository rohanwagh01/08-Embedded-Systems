#include <BMP280_DEV.h>

BMP280_DEV bmp280(21, 22);       // Instantiate (create) a BMP280 object and set-up for I2C on ESP32's pins 21 and 22


float temperature; //variable for temperature
float pressure;    //variable for pressure
float altitude;    //variable for alititude, but tbh, this is pretty unreliable without calibration

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("starting");
  bmp280.begin(0x77);                             //initialize module
  bmp280.setTimeStandby(TIME_STANDBY_2000MS);     // Set the standby time to 1 sec
  bmp280.startNormalConversion();                 // Start BMP280 continuous conversion in NORMAL_MODE
  Serial.println("setup done");
}

void loop() {
  if (bmp280.getMeasurements(temperature, pressure, altitude)) {
    Serial.printf("temp: %f *C\t\tpressure: %f hPa\t\taltitute: %f m\n", temperature, pressure, altitude);
  }
}
