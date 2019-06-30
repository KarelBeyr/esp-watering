#include <Arduino.h>
#include "passwords.h"
#include "tools.h"
#include "thingspeak.h"
#include "wifimodule.h"
#include "ota.h"
#include "esp_system.h"
#include "watchdog.h"
#include <HTTPClient.h>

State state = State();

void setupRelay()
{
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void setup()
{
  setupWatchdog();
  setupRelay();
  Serial.begin(115200);
  setupWifiServer(true);
  setupOta();
}

int counter = 0;

void loop()
{
  feedWatchdog();
  delay(1000); //otestovat zda se neusmazi
  maybeServeClient(true, &state);
  handleOta();
  counter++;
  if (counter == 600)  //kazdych 10 minut otocime
  {
    esp_restart();
  }
}
