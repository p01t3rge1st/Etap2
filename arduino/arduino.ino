#include "PMS.h"
#include <Wire.h>
#include <DHT.h>
#include "SGP30.h"
#include <Adafruit_SCD30.h>

#define LOG_PERIOD 10000
#define Geiger_Pin 3
#define DHT11_PIN 6
#define DHT22_PIN 7
#define DEBUG_ENABLED 1
#define CHECK_TIME 0.1

HardwareSerial& pmsSerial = Serial1;
PMS pms(pmsSerial);
PMS::DATA data;
SGP30 SGP;
Adafruit_SCD30 scd30;
DHT dht11(DHT11_PIN, DHT11);
DHT dht22(DHT22_PIN, DHT22);
bool debugMode;
int counts;


int pm1, pm25, pm100;
float humidity1, temperature1, humidity2, temperature2;
int ethanol, hydrogen, eCO2, CO2, TVOC, temperature_co2, humidity_co2;

bool read_success;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("AEPD - Air/Electromagnetic Particle Detector ver 1.0.0");

  Wire.begin();
  SGP.begin();
  scd30.begin();
  scd30.setMeasurementInterval(5);
  dht11.begin();
  dht22.begin();
  
  pinMode(Geiger_Pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(Geiger_Pin), impulse, FALLING);
}

void loop() {
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;
  
  if (currentMillis - previousMillis > LOG_PERIOD) {
    previousMillis = currentMillis;


    counts = 0;
  }

  if (Serial.available() > 0) {
    handleSerialCommand();
  }
  
  delay(5000);  // Delay before next cycle
  readSCD30SensorData();
  readSensors();
}

void readSensors() {
  humidity1 = dht11.readHumidity();
  temperature1 = dht11.readTemperature();
  humidity2 = dht22.readHumidity();
  temperature2 = dht22.readTemperature();
  
  SGP.measure(true);
  ethanol = SGP.getEthanol();
  hydrogen = SGP.getH2();
  eCO2 = SGP.getCO2();
  TVOC = SGP.getTVOC();

  pmsSerial.begin(9600);
  delay(500);

  for (int j = 0; j < 100; j++) {
    if (pms.read(data)) {
      read_success = true;
      pm1 = data.PM_AE_UG_1_0;
      pm25 = data.PM_AE_UG_2_5;
      pm100 = data.PM_AE_UG_10_0;
      break;
    } else {
      read_success = false;
    }
  }

  pmsSerial.end();
  
  if (read_success) {
    Serial.println("--[new_line]--");
    Serial.print("PM 1.0 (ug/m3): ");
    Serial.println(pm1);
    Serial.print("PM 2.5 (ug/m3): ");
    Serial.println(pm25);
    Serial.print("PM 10.0 (ug/m3): ");
    Serial.println(pm100);
    Serial.print("CO2 Level: ");
    Serial.println(CO2);
    Serial.print("TVOC Level: ");
    Serial.println(TVOC);
    Serial.print("Temperature: ");
    Serial.println(temperature_co2);
    Serial.print("Humidity: ");
    Serial.println(humidity_co2);
    Serial.print("Radiation: " );
    Serial.print(counts*6);
    Serial.println( " CPM");
    Serial.print("Radiation dose per h: " );
    Serial.print(counts*0.0378);
    Serial.println( " uSv");
    Serial.println("--[end_line]--");

    tone(11, 1000, 10);  // Indicator for new data
    
  }
}

void readSCD30SensorData() {
  if (scd30.dataReady()) {
    if (!scd30.read()) {
      return;  // Skip if no data available
    }
    
    temperature_co2 = scd30.temperature;
    humidity_co2 = scd30.relative_humidity;
    CO2 = scd30.CO2;
  }
}

void handleSerialCommand() {
  String command = Serial.readStringUntil('\n');
  
  if (command.equals("debug on")) {
    debugMode = true;
  } else if (command.equals("debug off")) {
    debugMode = false;
  }
}

void impulse() {
  counts++;
}
