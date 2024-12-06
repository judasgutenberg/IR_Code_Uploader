
/*
 * ESP8266 Remote Control IR code capturer
 * Uploads IR codes to our database in the cloud for future use controlling devices
 *  by Gus Mueller, Dec 4 2024, based loosely on https://github.com/crankyoldgit/IRremoteESP8266/blob/master/examples/IRrecvDemo/IRrecvDemo.ino
 */

#define RAWBUF 4200 
 
 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>

#include "config.h"

IRsend irsend(ir_pin);
IRrecv irrecv(ir_pin);

decode_results results;
String ipAddress;
boolean donePrinting = false;

void setup() {
  Serial.begin(115200);
  wiFiConnect();
  //irsend.sendNEC(0xF7C03F, 32, 4);
  irrecv.enableIRIn();  // Start the receiver
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(ir_pin);
}


void loop() {

  String irData = "";
  if (irrecv.decode(&results)) {
    Serial.println(resultToHexidecimal(&results)); // Show decoded code
    Serial.println("Raw Signal:");
    for (int i = 0; i < results.rawlen; i++) {
      Serial.print(results.rawbuf[i] * kRawTick);
      Serial.print(" ");
      irData += (String)(results.rawbuf[i] * kRawTick) + ",";
    }
    sendIrData(irData);
    Serial.println();
    irrecv.resume();
    donePrinting  = false;
  } else if (irrecv.decode(&results)  && false) {
    // print() & println() can't handle printing long longs. (uint64_t)
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();  // Receive the next value
    donePrinting  = false;
  } else if (!donePrinting){
    donePrinting = true;
    Serial.println("-------------------------");
    Serial.println("");
  } else {
    donePrinting = true;
  }
  delay(100);
}


//SEND DATA TO A REMOTE SERVER TO STORE IN A DATABASE----------------------------------------------------
void sendIrData(String datastring) {
  WiFiClient clientGet;
  const int httpGetPort = 80;
  String url;
  String mode = "saveIrData";
  //most of the time we want to getDeviceData, not saveData. the former picks up remote control activity. the latter sends sensor data
  
  url =  (String)url_get + "?storagePassword=" + (String)storage_password + "&mode=" + mode + "&data=" + datastring;
  Serial.println("\r>>> Connecting to host: ");
  //Serial.println(host_get);
  int attempts = 0;
  while(!clientGet.connect(host_get, httpGetPort) && attempts < connection_retry_number) {
    attempts++;
    delay(200);
  }
  Serial.print("Connection attempts:  ");
  Serial.print(attempts);
  Serial.println();
  if (attempts >= connection_retry_number) {
    Serial.print("Connection failed");
 
    Serial.print(host_get);
    Serial.println();
  } else {
 
     Serial.println(url);
     clientGet.println("GET " + url + " HTTP/1.1");
     clientGet.print("Host: ");
     clientGet.println(host_get);
     clientGet.println("User-Agent: ESP8266/1.0");
     clientGet.println("Accept-Encoding: identity");
     clientGet.println("Connection: close\r\n\r\n");
     unsigned long timeoutP = millis();
     while (clientGet.available() == 0) {
       if (millis() - timeoutP > 10000) {
        //let's try a simpler connection and if that fails, then reboot moxee
        //clientGet.stop();
        if(clientGet.connect(host_get, httpGetPort)){
         //timeOffset = timeOffset + timeSkewAmount; //in case two probes are stepping on each other, make this one skew a 20 seconds from where it tried to upload data
         clientGet.println("GET / HTTP/1.1");
         clientGet.print("Host: ");
         clientGet.println(host_get);
         clientGet.println("User-Agent: ESP8266/1.0");
         clientGet.println("Accept-Encoding: identity");
         clientGet.println("Connection: close\r\n\r\n");
        }//if (clientGet.connect(
        //clientGet.stop();
        return;
       } //if( millis() -  
     }
    delay(1); //see if this improved data reception. OMG IT TOTALLY WORKED!!!
    bool receivedData = false;
    bool receivedDataJson = false;
    while(clientGet.available()){
      String retLine = clientGet.readStringUntil('\n');
    }
  } //if (attempts >= connection_retry_number)....else....    
  clientGet.stop();
}
 
void wiFiConnect() {
  WiFi.persistent(false); //hopefully keeps my flash from being corrupted, see: https://rayshobby.net/wordpress/esp8266-reboot-cycled-caused-by-flash-memory-corruption-fixed/
  WiFi.begin(wifi_ssid, wifi_password);    
  Serial.println();
  // Wait for connection
  int wiFiSeconds = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    wiFiSeconds++;
    if(wiFiSeconds > 80) {
      Serial.println("WiFi taking too long, rebooting Moxee");
      //rebootMoxee();
      wiFiSeconds = 0; //if you don't do this, you'll be stuck in a rebooting loop if WiFi fails once
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wifi_ssid);
  Serial.print("IP address: ");
  ipAddress =  WiFi.localIP().toString();
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}
