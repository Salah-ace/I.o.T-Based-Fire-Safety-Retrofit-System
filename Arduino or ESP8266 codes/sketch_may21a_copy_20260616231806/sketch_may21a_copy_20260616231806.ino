#define BLYNK_TEMPLATE_ID "" //add blynk template ID
#define BLYNK_TEMPLATE_NAME "Fire Detection System"
#define BLYNK_AUTH_TOKEN "" //add authentication token

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

char ssid[] = "";  //Device name
char pass[] = "";  // Device hotspot password

// Sensors & Outputs
#define FLAME_SENSOR  4 //add 3 or 2
#define SMOKE_SENSOR  12
#define BUZZER        14
#define PUMP          5
#define DHTPIN        13
#define DHTTYPE       DHT11

// Blynk Virtual Pins
// V0 = Temperature
// V1 = Humidity
// V2 = Flame status
// V3 = Smoke status
// V4 = Pump status label
// V5 = Silence buzzer button  <-- NEW
// V6 = System status label    <-- NEW

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool fireDetected    = false;
bool smokeDetected   = false;
bool buzzerSilenced  = false;  // NEW — tracks if user silenced the buzzer

// ─────────────────────────────────────────────
// Called when user taps the Silence button (V5)
// ─────────────────────────────────────────────
BLYNK_WRITE(V5) {
  int buttonState = param.asInt();

  if (buttonState == 1) {
    buzzerSilenced = true;
    digitalWrite(BUZZER, LOW);  // Stop buzzer immediately
    Blynk.virtualWrite(V6, "⚠️ Buzzer Silenced - Still Monitoring");
    Serial.println("Buzzer silenced via Blynk app.");
  } else {
    buzzerSilenced = false;
    Serial.println("Silence reset.");
  }
}

// ─────────────────────────────────────────────
// Main sensor loop — runs every 2 seconds
// ─────────────────────────────────────────────
void sendSensorData() {

  int flame = digitalRead(FLAME_SENSOR);
  int smoke = digitalRead(SMOKE_SENSOR);

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  bool fireDetectedNow  = (flame == LOW);
  bool smokeDetectedNow = (smoke == LOW);

  // Send sensor readings to Blynk
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, hum);
  Blynk.virtualWrite(V2, fireDetectedNow  ? 1 : 0);
  Blynk.virtualWrite(V3, smokeDetectedNow ? 1 : 0);

  // ── BUZZER LOGIC ──────────────────────────
  if (fireDetectedNow || smokeDetectedNow) {
    if (!buzzerSilenced) {
      digitalWrite(BUZZER, HIGH);  // Sound buzzer only if not silenced
    }
    // If silenced, buzzer stays off but everything else still runs
  } else {
    // All clear — reset buzzer and silence flag
    digitalWrite(BUZZER, LOW);

    if (buzzerSilenced) {
      buzzerSilenced = false;             // Auto-reset silence for next event
      Blynk.virtualWrite(V5, 0);         // Reset button in app
    }
  }

  // ── PUMP LOGIC (fire only) ─────────────────
  if (fireDetectedNow) {
    digitalWrite(PUMP, HIGH);
    Blynk.virtualWrite(V4, "🔥 FIRE DETECTED - PUMP ON");
    Blynk.virtualWrite(V6, "🔥 FIRE ALARM ACTIVE");
    Serial.println("FIRE DETECTED -> Buzzer ON, Pump ON");

    if (!fireDetected) {  // Send notification only once per event
      Blynk.logEvent("fire_alert", "🔥 Fire detected! Pump activated! Open app to silence buzzer.");
      fireDetected = true;
    }

  } else {
    digitalWrite(PUMP, LOW);
    fireDetected = false;

    if (smokeDetectedNow) {
      Blynk.virtualWrite(V4, "💨 SMOKE DETECTED - PUMP OFF");
      Blynk.virtualWrite(V6, buzzerSilenced ? "⚠️ Buzzer Silenced - Still Monitoring" : "💨 SMOKE ALARM ACTIVE");

      if (!smokeDetected) {  // Send notification only once per smoke event
        Blynk.logEvent("fire_alert", "💨 Smoke detected! Check your environment.");
        smokeDetected = true;
      }

      Serial.println("SMOKE DETECTED -> Buzzer ON, Pump OFF");

    } else {
      // Fully clear
      smokeDetected = false;
      Blynk.virtualWrite(V4, "✅ PUMP OFF");
      Blynk.virtualWrite(V6, "✅ System Safe");
      Serial.println("System Safe");
    }
  }

  // Serial Monitor
  Serial.print("Temp: ");     Serial.print(temp);
  Serial.print(" C  Hum: ");  Serial.print(hum);
  Serial.print(" %  Flame: "); Serial.print(flame);
  Serial.print("  Smoke: ");  Serial.println(smoke);
}

void setup() {
  Serial.begin(9600);

  pinMode(FLAME_SENSOR, INPUT_PULLUP);
  pinMode(SMOKE_SENSOR,  INPUT_PULLUP);
  pinMode(BUZZER,        OUTPUT);
  pinMode(PUMP,          OUTPUT);

  digitalWrite(BUZZER, LOW);
  digitalWrite(PUMP,   LOW);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}
