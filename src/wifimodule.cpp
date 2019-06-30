#include <WiFi.h>
#include <Arduino.h>
#include "debugUtils.h"
#include "wifimodule.h"
#include "thingspeak.h"
#include "pass.h"

//http://tomeko.net/online_tools/cpp_text_escape.php?lang=en
const char *webPages[] = {
"",
"<!doctype html>\n"
"<html lang=\"en\">\n"
"  <head>\n"
"    <meta charset=\"utf-8\">\n"
"    <title>Watering control</title>\n"
"  </head>\n"
"  <body>\n"
"    <form action=\"/water5\" method=\"post\"> <input type=\"submit\" value=\"5s\"></form>\n"
"    <form action=\"/water30\" method=\"post\"> <input type=\"submit\" value=\"30s\"></form>\n"
"    <form action=\"/water60\" method=\"post\"> <input type=\"submit\" value=\"60s\"></form>\n"
"    <br />\n"
"    <p>Built at: \n"
__DATE__
"    @ \n"
__TIME__
"    </p>\n"
"  </body>\n"
"</html>",
"Watering finished",
};

WiFiServer httpServer(80);

void setupWifiServer()
{
    DEBUG_PRINTLN("");
    DEBUG_PRINT("Connecting to ");
    DEBUG_PRINTLN(WifiSsid);

    WiFi.begin(WifiSsid, WifiPassword);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        DEBUG_PRINT(".");
    }

    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("WiFi connected.");
    DEBUG_PRINTLN("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());

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

void water(int seconds)
{
    DEBUG_PRINTLN("Watering start");
    digitalWrite(2, HIGH);
    delay(seconds * 1000);
    digitalWrite(2, LOW);
    DEBUG_PRINTLN("Watering end");
    maybeLogTelemetryToThingspeak(WriteAPIKeyTFA, 30, "0", String(seconds));
}

void printPageToClient(WiFiClient client, int pageType)
{
  String str = webPages[pageType];
  client.print(str); // the content of the HTTP response follows the header:
}

void maybeServeClient()
{
    bool res = false;
    WiFiClient client = httpServer.available(); // listen for incoming clients
    if (client)
    { // if you get a client,
        DEBUG_PRINTLN("New Client.");
        int pageType = 0;
        String currentLine = ""; // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                if (c == '\n')
                { // if the byte is a newline character
                    DEBUG_PRINTLN(currentLine);
                    if (currentLine.startsWith("GET "))
                        pageType = 1;
                    if (currentLine.startsWith("POST /water60"))
                    {
                        pageType = 2;
                        water(60);
                    }
                    if (currentLine.startsWith("POST /water30"))
                    {
                        pageType = 2;
                        water(30);
                    }
                    if (currentLine.startsWith("POST /water5"))
                    {
                        pageType = 2;
                        water(5);
                    }
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
                        printPageToClient(client, pageType);

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
        DEBUG_PRINTLN("Client Disconnected.");
    }
}
