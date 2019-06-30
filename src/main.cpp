#include <Arduino.h>
#include "debugUtils.h"
#include "wifimodule.h"
#include "ota.h"
#include "esp_system.h"
#include "watchdog.h"
#include <HTTPClient.h>

const byte pinNumber = 2;

void setupRelay()
{
  pinMode(pinNumber, OUTPUT);
  digitalWrite(pinNumber, LOW);
}

void setup()
{
  setupWatchdog(90);
  setupRelay();
  DEBUG_SERIAL_START(115200);
  setupWifiServer();
  setupOta();
}

int counter = 0;

void loop()
{
  feedWatchdog();
  delay(1000); //otestovat zda se neusmazi
  maybeServeClient(pinNumber);
  handleOta();
  counter++;
  if (counter == 600)  //kazdych 10 minut otocime
  {
    esp_restart();
  }
}
