#include <Arduino.h>
#include "wifimodule.h"
#include "ota.h"
#include "esp_system.h"
#include "watchdog.h"
#include <HTTPClient.h>

void setupRelay()
{
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

void setup()
{
  setupWatchdog(90);
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
  maybeServeClient(true);
  handleOta();
  counter++;
  if (counter == 600)  //kazdych 10 minut otocime
  {
    esp_restart();
  }
}
