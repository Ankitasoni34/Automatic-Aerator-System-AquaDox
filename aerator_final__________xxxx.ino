#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

// Pin definitions
#define OXYGEN_SENSOR_PIN A0    
#define PH_SENSOR_PIN A1        
#define RELAY_PIN 3             
#define PH_RELAY_PIN 4          
#define ONE_WIRE_BUS 2          
#define RX_PIN 10               
#define TX_PIN 11               

// Constants
const float oxygenVoltageReference = 5.0; 
const float oxygenMaxVoltage = 4.5;       
const float oxygenMaxConcentration = 25.0;
const float phThreshold = 7.0;           
const int oxygenThreshold = 15;          


const float voltageAtpH7 = 2.5; 
const float voltageAtpH4 = 1.5; 
const float slope = (4.0 - 7.0) / (voltageAtpH4 - voltageAtpH7); 
const float intercept = 7.0 - (slope * voltageAtpH7); 

// Initialize components
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
SoftwareSerial bluetooth(RX_PIN, TX_PIN);

// Variables
float temperature = 0.0;
float oxygenConcentration = 0.0;
float pHValue = 0.0;

void setup() {
  // Serial communication
  Serial.begin(9600);
  bluetooth.begin(9600); // Default HC-05 baud rate

  // Initialize sensors and relays
  sensors.begin();
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PH_RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);    
  digitalWrite(PH_RELAY_PIN, LOW); 

  // Startup messages
  Serial.println("Automated Aerator System Initializing...");
  bluetooth.println("Automated Aerator System Initialized");
}

void loop() {
  // ---- DS18B20 Temperature Reading ----
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);

  // Check for valid temperature reading
  if (temperature == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Temperature sensor disconnected");
    bluetooth.println("Error: Temperature sensor disconnected");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    bluetooth.print("Temperature: ");
    bluetooth.print(temperature);
    bluetooth.println(" °C");
  }

  // ---- ASAIR AOF1010 Oxygen Sensor Reading ----
  int oxygenRaw = analogRead(OXYGEN_SENSOR_PIN);
  float oxygenVoltage = (oxygenRaw / 1023.0) * oxygenVoltageReference;
  oxygenConcentration = (oxygenVoltage / oxygenMaxVoltage) * oxygenMaxConcentration;

  Serial.print("Oxygen Concentration: ");
  Serial.print(oxygenConcentration);
  Serial.println(" %");
  bluetooth.print("Oxygen Concentration: ");
  bluetooth.print(oxygenConcentration);
  bluetooth.println(" %");

  // ---- Crowtail pH Sensor Reading ----
  int phRaw = analogRead(PH_SENSOR_PIN);
  float phVoltage = (phRaw / 1023.0) * 5.0; // Convert raw ADC to voltage
  pHValue = slope * phVoltage + intercept + 1; // Calculate pH using calibrated values

  Serial.print("pH Value: ");
  Serial.println(pHValue);
  bluetooth.print("pH Value: ");
  bluetooth.println(pHValue);

  // ---- pH Threshold Control ----
  if (pHValue > phThreshold) {
    digitalWrite(PH_RELAY_PIN, HIGH); // Activate pH relay
    Serial.println("Warning: pH value exceeded 7. Relay activated.");
    bluetooth.println("Warning: pH value exceeded 7. Relay activated.");
  } else {
    digitalWrite(PH_RELAY_PIN, LOW); // Deactivate pH relay
    Serial.println("pH value is within acceptable range.");
    bluetooth.println("pH value is within acceptable range.");
  }

  // ---- Relay Control Logic for Oxygen ----
  if (oxygenConcentration < oxygenThreshold) {
    digitalWrite(RELAY_PIN, LOW); // Turn ON the relay (start aerator)
    Serial.println("Relay ON: Oxygen level low");
    bluetooth.println("Relay ON: Oxygen level low");
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // Turn OFF the relay (stop aerator)
    Serial.println("Relay OFF: Oxygen level sufficient");
    bluetooth.println("Relay OFF: Oxygen level sufficient");
  }

  // ---- Delay for Stable Readings ----
  delay(1000);
}
