#include <WiFi.h>
#include <Arduino.h>
#include "wifimodule.h"
#include "thingspeak.h"
#include "pass.h"

//http://tomeko.net/online_tools/cpp_text_escape.php?lang=en
const char *webPages[] = {
"",
"<script>\n"
"var freq;\n"
"var duty;\n"
"\n"
"function redraw() {\n"
"  var freqRaw = document.getElementById('freqInputId').value;\n"
"  freq = Math.pow(10, Math.floor(freqRaw/2)) * ((freqRaw % 2) * 2 + 1);\n"
"  duty = document.getElementById('dutyInputId').value;\n"
"  document.getElementById('dutySpanId').innerHTML = duty;\n"
"  document.getElementById('freqSpanId').innerHTML = freq.toPrecision(1);\n"
"}\n"
"\n"
"function sendData() {\n"
"  redraw();\n"
"  var url = '/data/' + duty + '/' + freq + '/';\n"
"  console.log(url);\n"
"  var xmlHttp = new XMLHttpRequest();\n"
"  xmlHttp.onreadystatechange = function() { \n"
"      if (xmlHttp.readyState == 4 && xmlHttp.status == 200)\n"
"          pending=false;\n"
"  }\n"
"  xmlHttp.open('PUT', url, true); // true for asynchronous \n"
"  xmlHttp.send(null);\n"
"}\n"
"</script>\n"
"<input type='range' min='0' max='15' value='currentFreq' oninput='sendData()' onchange='sendData()' id='freqInputId'>Switching frequency: <span id='freqSpanId'>?</span> Hz<br /><br />\n"
"<input type='range' min='0' max='100' value='currentDuty' oninput='sendData()' onchange='sendData()' id='dutyInputId'>Manual duty: <span id='dutySpanId'>?</span>%<br /><br />\n"
"<script>\n"
"  redraw();\n"
"</script>\n"
"",
"Build date: "
__DATE__
" build time: "
__TIME__
};

WiFiServer httpServer(80);

void setupWifiServer(bool logToSerial)
{
    if (logToSerial)
    {
        Serial.println("");
        Serial.print("Connecting to ");
        Serial.println(WifiSsid);
    }
    WiFi.begin(WifiSsid, WifiPassword);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        if (logToSerial)
            Serial.print(".");
    }
    if (logToSerial)
    {
        Serial.println("");
        Serial.println("WiFi connected.");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
    httpServer.begin();
    IPAddress ip = WiFi.localIP();
    //logMessageToThingspeak(String(ip[2]) + "." + String(ip[3]));
    maybeLogTelemetryToThingspeak(WriteAPIKeyTFA, 30, "1", "0");
}

String getParameterAfterSlash(String line, int nr)
{
    int idx = line.indexOf(' ');
    for (int i = 1; i <= nr; i++)
    {
        idx = line.indexOf('/', idx + 1);
    }
    int endIdx = line.indexOf('/', idx + 1);
    return line.substring(idx + 1, endIdx);
}

void water(int seconds, bool logToSerial)
{
    if (logToSerial)
        Serial.println("Watering start");
    digitalWrite(2, HIGH);
    delay(seconds * 1000);
    digitalWrite(2, LOW);
    if (logToSerial)
        Serial.println("Watering end");
    maybeLogTelemetryToThingspeak(WriteAPIKeyTFA, 30, "0", String(seconds));
}

void printPageToClient(WiFiClient client, int pageType, State *state)
{
  String str = webPages[pageType];
  if (pageType == 1)
  {
    int freqBase = round(log10(state->freq)) * 2 + ((state->freq - 1) % 3) / 2;
    str.replace("currentFreq", String(freqBase));
    str.replace("currentDuty", String(state->duty));
  }
  client.print(str); // the content of the HTTP response follows the header:
}

void maybeServeClient(bool logToSerial, State *state)
{
    bool res = false;
    WiFiClient client = httpServer.available(); // listen for incoming clients
    if (client)
    { // if you get a client,
        if (logToSerial)
            Serial.println("New Client.");
        int pageType = 0;
        String currentLine = ""; // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                if (c == '\n')
                { // if the byte is a newline character
                    if (logToSerial)
                        Serial.println(currentLine);
                    if (currentLine.startsWith("GET "))
                        pageType = 1;
                    if (currentLine.startsWith("GET /water30"))
                        water(30, logToSerial);
                    if (currentLine.startsWith("GET /water5"))
                        water(5, logToSerial);
                    if (currentLine.startsWith("GET /reboot"))
                        esp_restart_noos();
                    if (currentLine.startsWith("GET /bluescreen"))
                    {
                        while(true) {
                          //lockup to test watchdog
                        }
                    }
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        client.println("HTTP/1.1 200 OK");        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        client.println("Content-type:text/html"); // and a content-type so the client knows what's coming, then a blank line:
                        client.println();
                        printPageToClient(client, pageType, state);

                        client.println();                 // The HTTP response ends with another blank line:
                        break;                            // break out of the while loop:
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        client.stop(); // close the connection:
        if (logToSerial)
        {
            Serial.println("Client Disconnected.");
        }
    }
}
