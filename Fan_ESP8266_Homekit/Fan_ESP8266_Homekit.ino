#include <EEPROM.h>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"

#define PIN_SWITCH 2
#define EEPROM_SIZE 250
#define LOG_D(fmt, ...) printf_P(PSTR(fmt "\n"), ##__VA_ARGS__);

void setup() {
  //Read from memory
  EEPROM.begin(EEPROM_SIZE);
  bool fanOn = EEPROM.read(205);
  bool on = fanOn;
  
  //Connect to wifi
  Serial.begin(115200);
  wifi_connect();

  //Print and change state
  digitalWrite(PIN_SWITCH, fanOn ? LOW : HIGH);
  Serial.println("Fan on: ");
  Serial.println(fanOn);
  //Start homekit setup
  my_homekit_setup();
}

void loop() {
  my_homekit_loop();
  delay(10);
}

//==============================
// HomeKit setup and loop
//==============================

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;

static uint32_t next_heap_millis = 0;



//Called when the switch value is changed by iOS Home APP
void cha_switch_on_setter(const homekit_value_t value) {
  bool on = value.bool_value;
  cha_switch_on.value.bool_value = on;  //sync the value
  LOG_D("Switch: %s", on ? "ON" : "OFF");
  digitalWrite(PIN_SWITCH, on ? LOW : HIGH);
  
  //Write to Memory
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(205, on);
  EEPROM.commit();
  
  //Print state
  Serial.println("Write memory: ");
  Serial.println(on);
}

void my_homekit_setup() {
  pinMode(PIN_SWITCH, OUTPUT);

  cha_switch_on.setter = cha_switch_on_setter;
  arduino_homekit_setup(&config);
}

void my_homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_heap_millis) {
    // show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
}
