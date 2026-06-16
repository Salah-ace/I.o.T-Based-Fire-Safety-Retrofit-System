#define BLYNK_TEMPLATE_ID "TMPL2MH9WLR43"
#define BLYNK_TEMPLATE_NAME "Fire Detection System"
#define BLYNK_AUTH_TOKEN "tiMkKVNVP_VMLQVQBQB4EFT1WWMq6MfC"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>

// Wi-Fi Credentials
char ssid[] = "Inf";
char pass[] = "12131213";

// PCF8574 I2C Address
#define PCF8574_ADDR 0x20

// ESP8266 Pin Definitions
#define FLAME_SENSOR  4   // GPIO4 (D2)
#define SMOKE_SENSOR  12  // GPIO12 (D6)
#define BUZZER        14  // GPIO14 (D5)
#define PUMP_RELAY    5   // GPIO5 (D1)
#define DHTPIN        13  // GPIO13 (D7)
#define DHTTYPE       DHT11

// PCF8574 Fan Control Pins
#define FAN_INA 0
#define FAN_INB 1

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool fireDetected    = false;
bool smokeDetected   = false;
bool buzzerSilenced  = false;

void setFan(bool state) {
  Wire.beginTransmission(PCF8574_ADDR);
  if (state) {
    Wire.write((1 << FAN_INA) | (1 << FAN_INB));
  } else {
    Wire.write(0x00);
  }
  Wire.endTransmission();
}

BLYNK_WRITE(V5) {
  int buttonState = param.asInt();
  if (buttonState == 1) {
    buzzerSilenced = true;
    digitalWrite(BUZZER, LOW);
    Blynk.virtualWrite(V6, "⚠️ Buzzer Silenced - Still Monitoring");
  } else {
    buzzerSilenced = false;
  }
}

void sendSensorData() {
  int flame = digitalRead(FLAME_SENSOR);
  int smoke = digitalRead(SMOKE_SENSOR);
  
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  
  bool fireDetectedNow  = (flame == LOW);
  bool smokeDetectedNow = (smoke == LOW);
  
  // Send to Blynk
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, hum);
  Blynk.virtualWrite(V2, fireDetectedNow ? 1 : 0);
  Blynk.virtualWrite(V3, smokeDetectedNow ? 1 : 0);
  Blynk.virtualWrite(V7, smokeDetectedNow ? 1 : 0);
  
  // Buzzer Logic
  if (fireDetectedNow || smokeDetectedNow) {
    if (!buzzerSilenced) {
      digitalWrite(BUZZER, HIGH);
    }
  } else {
    digitalWrite(BUZZER, LOW);
    if (buzzerSilenced) {
      buzzerSilenced = false;
      Blynk.virtualWrite(V5, 0);
    }
  }
  
  // Fan Logic (Smoke only OR Fire + Smoke)
  if (smokeDetectedNow) {
    setFan(true);
  } else {
    setFan(false);
  }
  
  // Pump Logic (Fire only)
  if (fireDetectedNow) {
    digitalWrite(PUMP_RELAY, HIGH);
    Blynk.virtualWrite(V4, "🔥 FIRE DETECTED - SPRINKLER ON");
    Blynk.virtualWrite(V6, "🔥 FIRE ALARM ACTIVE");
    
    if (!fireDetected) {
      Blynk.logEvent("fire_alert", "🔥 Fire detected! Sprinkler activated!");
      fireDetected = true;
    }
  } else {
    digitalWrite(PUMP_RELAY, LOW);
    fireDetected = false;
    
    if (smokeDetectedNow) {
      Blynk.virtualWrite(V4, "💨 SMOKE DETECTED - SPRINKLER OFF");
      Blynk.virtualWrite(V6, "💨 SMOKE ALARM - FAN ON");
      
      if (!smokeDetected) {
        Blynk.logEvent("smoke_alert", "💨 Smoke detected! Fan activated.");
        smokeDetected = true;
      }
    } else {
      smokeDetected = false;
      Blynk.virtualWrite(V4, "✅ SYSTEM SAFE");
      Blynk.virtualWrite(V6, "✅ System Safe - Monitoring");
    }
  }
}

void setup() {
  Serial.begin(9600);
  
  Wire.begin(4, 5);
  
  pinMode(FLAME_SENSOR, INPUT_PULLUP);
  pinMode(SMOKE_SENSOR,  INPUT_PULLUP);
  pinMode(BUZZER,        OUTPUT);
  pinMode(PUMP_RELAY,    OUTPUT);
  
  digitalWrite(BUZZER,     LOW);
  digitalWrite(PUMP_RELAY, LOW);
  setFan(false);
  
  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}
