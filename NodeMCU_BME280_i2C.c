#include <Adafruit_BME280.h>
#include <Wire.h>
#define SDA D3  //实际接在NodeMCU D1
#define SDC D4  //实际接在NodeMCU D2

Adafruit_BME280 bme280;
bool bme280_CanRead = true;
unsigned long uptime = 0;
unsigned long last_send = 0;
const unsigned long sending_interval = 3000;

//判断280针脚地址
bool initBME280(char addr) {
  if (bme280.begin(addr)) {
    Serial.println("found");
    return true;
  } else {
    Serial.println("not found");
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.pins(SDC, SDA);
  Wire.begin(SDC, SDA);
  // BME280 initialisieren
  if (bme280_CanRead) {
    if (!initBME280(0x76) && !initBME280(0x77)) {
      Serial.println("Check BME280 wiring");
      bme280_CanRead = false;
    }
  }
  delay(2500);
}
void loop() {
  uptime = millis();
  if (last_send > uptime)
     last_send = 0;
  if (last_send == 0 || last_send + sending_interval < uptime){
    Serial.print("last:");
    Serial.println(last_send);
    Serial.print("uptime:");
    Serial.println(uptime);
  if (bme280_CanRead)
  {
    Serial.print("t->");
    Serial.println(bme280.readTemperature());
    Serial.print("h->");
    Serial.println(bme280.readHumidity());
    Serial.print("p->");
    Serial.println(bme280.readPressure());
  }
    last_send = uptime;
   }
}
