/*|----------------------------------------------------------|*/
/*|Connection sketch to eduroam network (WPA/WPA2) Enteprise |*/
/*|Suitable for almost any ESP32 microcontroller with WiFi   |*/
/*|Raspberry or Arduino WiFi CAN'T USE THIS LIBRARY!!!       |*/
/*|Based on: https://github.com/martinius96/ESP32-eduroam    |*/
/*           https://github.com/martinius96/ESP32-eduroam    |*/
/*|Edited by: Eduardo Wachter (ewwachter)                    |*/
/*|Compilation under 2.0.3 Arduino Core and higher worked    |*/
/*|Compilation can be done only using STABLE releases        |*/
/*|Dev releases WILL NOT WORK. (Check your Ard. Core .json)  |*/
/*|WiFi.begin() have more parameters for PEAP connection     |*/
/*|----------------------------------------------------------|*/

//WITHOUT certificate option connection is WORKING (if there is exception set on RADIUS server that will let connect devices without certificate)
//It is DEPRECATED function and standardly turned off, so it must be turned on by your eduroam management at university / organisation


#include <WiFi.h> //Wifi library
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks

#include <credentials.h>
//credentials file should  include those:
// //Identity for user with password related to his realm (organization)
// //Available option of anonymous identity for federation of RADIUS servers or 1st Domain RADIUS servers
// #define EAP_ANONYMOUS_IDENTITY "anonymous@warwick.ac.uk" //anonymous@example.com, or you can use also nickname@example.com
// #define EAP_IDENTITY "username@warwick.ac.uk" //your full university username e.g. u1234567@warwick.ac.uk
// #define EAP_PASSWORD "password" //password for eduroam account
// #define EAP_USERNAME "username@warwick.ac.uk" // the Username is the same as the Identity in most eduroam networks.

//SSID NAME
const char* ssid = "eduroam";

const char *ntpServer = "pool.ntp.org";
const char *timezone = "GMT0BST,M3.5.0/1,M10.5.0";

uint8_t counter = 0;
unsigned long previousMillisWiFi = 0;
char timeStringBuff[50];
void printLocalTime(bool printToSerial = false)
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.print(F("NTP sync failed "));
    return;
  }
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  if(printToSerial) Serial.println(timeStringBuff);
}

void setup() {
	byte error = 0;
  Serial.begin(115200);
  delay(10000);
  Serial.print(F("Connecting to network: "));
  Serial.println(ssid);
  WiFi.disconnect(true);  //disconnect from WiFi to set new WiFi connection

  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD); // without CERTIFICATE, RADIUS server EXCEPTION "for old devices" required

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("");
  Serial.println(F("WiFi is connected!"));
  Serial.println(F("IP address set: "));
  Serial.println(WiFi.localIP()); //print LAN IP

  //init and get the time
  configTime(0, 0, ntpServer);
  setenv("TZ", timezone, 1);
  time_t now;
  Serial.print("Obtaining NTP time: ");
  printLocalTime();
  while (now < 1510592825)
  {
    Serial.print(F("."));
    delay(500);
    time(&now);
  }
  Serial.print(F(" success!\nGot Time: "));
  printLocalTime(true);
  Serial.print(F("NTP time received!\n"));
}

void loop(){

  unsigned long currentMillis = millis();

  if (WiFi.status() == WL_CONNECTED)
  {              //if we are connected to Eduroam network
    counter = 0; //reset counter
    if (currentMillis - previousMillisWiFi >= 15 * 1000)
    {
      printLocalTime(true);
      previousMillisWiFi = currentMillis;
      Serial.print(F("Wifi is still connected with IP: "));
      Serial.println(WiFi.localIP()); //inform user about his IP address
    }
  }
  else if (WiFi.status() != WL_CONNECTED)
  { //if we lost connection, retry
    WiFi.begin(ssid);

    Serial.printf("Connecting to WiFi: %s ", ssid);
    while (WiFi.status() != WL_CONNECTED)
    { //during lost connection, print dots
      delay(500);
      Serial.print(F("."));
      counter++;
      if (counter >= 60)
      { //30 seconds timeout - reset board
        ESP.restart();
      }
    }
    Serial.println(F(" connected!"));
  }
}