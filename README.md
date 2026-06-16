# 🔥 IoT-Based Fire Safety and Retrofit System

## 📋 Overview

This project presents an **IoT-based fire detection and automatic suppression system** specifically designed to retrofit existing condominium buildings in Ethiopia. The system provides real-time monitoring, automatic fire suppression, smoke exhaust, and mobile alerts — all at a low cost using readily available components.

---

## 🎯 Key Features

- 🔥 **Real-time fire detection** using IR flame sensor and MQ-2 smoke detector
- 🌡️ **Temperature and humidity monitoring** with DHT11 sensor
- 📱 **Remote monitoring** via Blynk IoT platform (mobile dashboard)
- 💧 **Automatic fire suppression** using water pump and solenoid valve
- 💨 **Automatic smoke exhaust** using L9110 fan module
- 🔊 **Audible alerts** with 5V buzzer
- 📲 **Push notifications** to mobile phone
- 🔕 **Silence buzzer** remotely from the app
- 🧠 **Smart logic** differentiates between fire-only, smoke-only, and fire+smoke scenarios
- 💰 **Low-cost custom sprinkler head** as alternative to expensive commercial options

---

## 🛠️ Hardware Components

| Component | Quantity | Function |
|-----------|----------|----------|
| ESP8266 (NodeMCU) | 1 | Microcontroller with Wi-Fi for IoT connectivity |
| PCF8574 I2C GPIO Expander | 1 | Provides additional GPIO pins for fan control |
| IR Flame Sensor | 1 | Detects infrared radiation from open flames |
| MQ-2 Smoke Detector | 1 | Detects smoke and combustible gases |
| DHT11 Temperature/Humidity Sensor | 1 | Monitors ambient conditions for rate-of-rise detection |
| L9110 Fan Module | 1 | Exhausts smoke to improve visibility |
| 5V Relay Module | 1 | Controls water pump and solenoid valve simultaneously |
| 12V Solenoid Valve | 1 | Controls water flow for fire suppression |
| 5-12V Water Pump | 1 | Pumps water to sprinkler head |
| Custom Sprinkler Head | 1 | Distributes water evenly (fabricated in-house) |
| Buzzer (5V) | 1 | Provides audible fire alert |
| Breadboard | 1 | For prototyping connections |
| 12V/2A DC Adapter | 1 | Powers solenoid valve and water pump |
| 5V USB Adapter | 1 | Powers ESP8266, sensors, PCF8574, and relay |

---


---

## 🔌 Circuit Connections

### ESP8266 Pin Connections

| Component | GPIO | NodeMCU Label | Power | Ground |
|-----------|------|---------------|-------|--------|
| IR Flame Sensor | GPIO4 | D2 | 3.3V/5V | GND |
| MQ-2 Smoke Detector | GPIO12 | D6 | 5V | GND |
| DHT11 | GPIO13 | D7 | 5V | GND |
| 5V Relay (Pump + Valve) | GPIO5 | D1 | 5V | GND |
| Buzzer | GPIO14 | D5 | 5V | GND |
| PCF8574 (SDA) | GPIO4 | D2 | 5V | GND |
| PCF8574 (SCL) | GPIO5 | D1 | 5V | GND |

### PCF8574 to L9110 Fan Module

| PCF8574 Pin | L9110 Pin | Function |
|-------------|-----------|----------|
| P0 | INA | Fan control input A |
| P1 | INB | Fan control input B |
| VCC | VCC | 5V Power |
| GND | GND | Ground |

### Relay Wiring (Water Pump + Solenoid Valve)

| Relay Terminal | Connected To |
|----------------|--------------|
| COM | 12V DC Power Supply (+) |
| NO | Water Pump (+) and Solenoid Valve (+) in parallel |
| GND | To 12V Supply Ground |

---

## 🧠 Smart Logic Table

| Situation | Pump | Fan | Buzzer |
|-----------|------|-----|--------|
| Fire only | ✅ ON | ❌ OFF | ✅ ON |
| Smoke only | ❌ OFF | ✅ ON | ✅ ON |
| Fire + Smoke | ✅ ON | ✅ ON | ✅ ON |
| All clear | ❌ OFF | ❌ OFF | ❌ OFF |

---

## 📱 Blynk Virtual Pins

| Virtual Pin | Type | Purpose |
|-------------|------|---------|
| V0 | Read | Temperature |
| V1 | Read | Humidity |
| V2 | Read | Flame status (1 = fire) |
| V3 | Read | Smoke status (1 = smoke) |
| V4 | Read | Pump/Sprinkler status label |
| V5 | Write | Silence buzzer button |
| V6 | Read | System status label |

---

## 💻 Code Structure

### Key Functions

```cpp
void sendSensorData() {
  // Read all sensors
  // Send data to Blynk
  // Apply logic for: Fire only → Pump ON, Fan OFF
  // Apply logic for: Smoke only → Pump OFF, Fan ON
  // Apply logic for: Fire + Smoke → Pump ON, Fan ON
  // Control buzzer
  // Send notifications
}

void setFan(bool state) {
  // I2C command to PCF8574 to control L9110 fan
}
```
## How the Fan is Controlled
### The fan is controlled via the PCF8574 GPIO expander over I2C:
```cpp
#include <Wire.h>
#define PCF8574_ADDR 0x20
#define FAN_INA 0
#define FAN_INB 1

void setFan(bool state) {
  Wire.beginTransmission(PCF8574_ADDR);
  if (state) {
    Wire.write((1 << FAN_INA) | (1 << FAN_INB)); // Fan ON
  } else {
    Wire.write(0x00); // Fan OFF
  }
  Wire.endTransmission();
}
```

