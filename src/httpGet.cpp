// #define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

// #include <Arduino.h>
// #include <Wire.h>
// #include <set>
// #include <string>
// #include "TinyGsmClient.h"
// #include <Ticker.h>
// #include <ArduinoHttpClient.h>

// #define mySerial2 Serial1
// #define UART_BAUD           115200
// #define PIN_DTR             25
// #define PIN_TX              27
// #define PIN_RX              26
// #define PWR_PIN             4
// #define LED_PIN             12

// // set GSM PIN, if any
// #define GSM_PIN ""
// #define GSM_NL "\n"

// // Your GPRS credentials, if any
// const char apn[]  =  "m2mglobal"; //"iot.1nce.net";     //SET TO YOUR APN
// const char gprsUser[] = "";
// const char gprsPass[] = "";


// std::string sendData(String command) {
//   // Send the AT command
//   mySerial2.println(command);
//   //Serial.println(command);

//   // Variables to store the response
//   std::string buffer = "";
//   unsigned long startTime = millis();
//   const unsigned long timeout = 60000; // Timeout in milliseconds

//   // Read the response
//   while (millis() - startTime < timeout) {
//     while (mySerial2.available()) {
//       char c = mySerial2.read();
//       buffer += c;

//       // Check for termination keywords
//       if (buffer.find("OK") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
//         break;
//       }
//     }

//     // Break if we already found "OK" or "ERROR"
//     if (buffer.find("OK") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
//       break;
//     }
//   }

//   // Print the response to the serial monitor
//   Serial.println(buffer.c_str());
//   delay(2000);
//   return buffer;
// }

// void modemPowerOn(){
//   pinMode(PWR_PIN, OUTPUT);
//   digitalWrite(PWR_PIN, LOW);
//   delay(1000);
//   digitalWrite(PWR_PIN, HIGH);
// }

// void modemPowerOff(){
//   pinMode(PWR_PIN, OUTPUT);
//   digitalWrite(PWR_PIN, LOW);
//   delay(1500);
//   digitalWrite(PWR_PIN, HIGH);
// }


// void modemRestart(){
//   modemPowerOff();
//   delay(1000);
//   modemPowerOn();
// }

// #ifdef DUMP_AT_COMMANDS
//   #include <StreamDebugger.h>
//   StreamDebugger debugger(mySerial2, Serial);
//   TinyGsm modem(debugger);
// #else
//   TinyGsm modem(mySerial2);
// #endif

// // Server details
// const char server[]   = "https://script.google.com";
// const char resource[] = "/macros/s/AKfycbzxEqzPUWYte95OsO16OxFMccnLTK1wJgX7usyyW0iUxV8wgACagI7dmkPB3l5vabg/exec?path=Sheet1&action=read b";
// const int  port       = 443;
// TinyGsmClient client(modem);
// HttpClient http(client, server, port);

 


// int counter, lastIndex, numberOfPieces = 24;
// String pieces[24], input;

// void setup() {
//   // put your setup code here, to run once:
//   Serial.begin(115200);
//   Serial.print("Hello! ESP32-S3 AT command V1.1 Test");

//   // Set LED OFF
//   pinMode(LED_PIN, OUTPUT);
//   digitalWrite(LED_PIN, HIGH);

//   pinMode(PWR_PIN, OUTPUT);
//   digitalWrite(PWR_PIN, HIGH);
//   delay(300);
//   digitalWrite(PWR_PIN, LOW);

//   //GSM Start
//   mySerial2.begin(UART_BAUD,SERIAL_8N1, PIN_RX, PIN_TX);

//   // Restart takes quite some time
//   // To skip it, call init() instead of restart()
//   Serial.println("Initializing modem...");
//   if (!modem.restart()) {
//     Serial.println("Failed to restart modem, attempting to continue without restarting");
//   }

// }

// void loop() {
//     // Restart takes quite some time
//   // To skip it, call init() instead of restart()
//   Serial.println("Initializing modem...");
//   if (!modem.init()) {
//     Serial.println("Failed to restart modem, attempting to continue without restarting");
//   }

//   String name = modem.getModemName();
//   delay(500);
//   Serial.println("Modem Name: " + name);

//   String modemInfo = modem.getModemInfo();
//   delay(500);
//   Serial.println("Modem Info: " + modemInfo);

//   // Unlock your SIM card with a PIN if needed
//   if ( GSM_PIN && modem.getSimStatus() != 3 ) {
//       modem.simUnlock(GSM_PIN);
//   }
//   modem.sendAT("+CFUN=0 ");
//   if (modem.waitResponse(10000L) != 1) {
//     DBG(" +CFUN=0  false ");
//   }
//   delay(200);

//   /*
//     2 Automatic
//     13 GSM only
//     38 LTE only
//     51 GSM and LTE only
//   * * * */
//   String res;
//   // CHANGE NETWORK MODE, IF NEEDED
//   res = modem.setNetworkMode(2);
//   if (res != "1") {
//     DBG("setNetworkMode  false ");
//     return ;
//   }
//   delay(200);

//   /*
//     1 CAT-M
//     2 NB-Iot
//     3 CAT-M and NB-IoT
//   * * */
//   // CHANGE PREFERRED MODE, IF NEEDED
//   res = modem.setPreferredMode(1);
//   if (res != "1") {
//     DBG("setPreferredMode  false ");
//     return ;
//   }
//   delay(200);

//   /*AT+CBANDCFG=<mode>,<band>[,<band>…]
//    * <mode> "CAT-M"   "NB-IOT"
//    * <band>  The value of <band> must is in the band list of getting from  AT+CBANDCFG=?
//    * For example, my SIM card carrier "NB-iot" supports B8.  I will configure +CBANDCFG= "Nb-iot ",8
//    */
//   /* modem.sendAT("+CBANDCFG=\"NB-IOT\",8 ");*/
  
//   /* if (modem.waitResponse(10000L) != 1) {
//        DBG(" +CBANDCFG=\"NB-IOT\" ");
//    }*/
//   delay(200);
//   modem.sendAT("+CFUN=1 ");
//   if (modem.waitResponse(10000L) != 1) {
//     DBG(" +CFUN=1  false ");
//   }
//   delay(200);

//   mySerial2.println("AT+CGDCONT?");
//   delay(500);
//   if (mySerial2.available()) {
//     input = mySerial2.readString();
//     for (int i = 0; i < input.length(); i++) {
//       if (input.substring(i, i + 1) == "\n") {
//         pieces[counter] = input.substring(lastIndex, i);
//         lastIndex = i + 1;
//         counter++;
//        }
//         if (i == input.length() - 1) {
//           pieces[counter] = input.substring(lastIndex, i);
//         }
//       }
//       // Reset for reuse
//       input = "";
//       counter = 0;
//       lastIndex = 0;

//       for ( int y = 0; y < numberOfPieces; y++) {
//         for ( int x = 0; x < pieces[y].length(); x++) {
//           char c = pieces[y][x];  //gets one byte from buffer
//           if (c == ',') {
//             if (input.indexOf(": ") >= 0) {
//               String data = input.substring((input.indexOf(": ") + 1));
//               if ( data.toInt() > 0 && data.toInt() < 25) {
//                 modem.sendAT("+CGDCONT=" + String(data.toInt()) + ",\"IP\",\"" + String(apn) + "\",\"0.0.0.0\",0,0,0,0");
//               }
//               input = "";
//               break;
//             }
//           // Reset for reuse
//           input = "";
//          } else {
//           input += c;
//          }
//       }
//     }
//   } else {
//     Serial.println("Failed to get PDP!");
//   }

//   Serial.println("\n\n\nWaiting for network...");
//   if (!modem.waitForNetwork()) {
//     delay(10000);
//     return;
//   }

//   if (modem.isNetworkConnected()) {
//     Serial.println("Network connected");
//   }
  
//   // --------TESTING GPRS--------
//   Serial.println("\n---Starting GPRS TEST---\n");
//   Serial.println("Connecting to: " + String(apn));
//   if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
//     delay(10000);
//     return;
//   }

//   Serial.print("GPRS status: ");
//   if (modem.isGprsConnected()) {
//     Serial.println("connected");
//   } else {
//     Serial.println("not connected");
//   }

//   String ccid = modem.getSimCCID();
//   Serial.println("CCID: " + ccid);

//   String imei = modem.getIMEI();
//   Serial.println("IMEI: " + imei);

//   String cop = modem.getOperator();
//   Serial.println("Operator: " + cop);

//   IPAddress local = modem.localIP();
//   Serial.println("Local IP: " + String(local));

//   int csq = modem.getSignalQuality();
//   Serial.println("Signal quality: " + String(csq));

//   mySerial2.println("AT+CPSI?");     //Get connection type and band
//   delay(500);
//   if (mySerial2.available()) {
//     String r = mySerial2.readString();
//     Serial.println(r);
//   }

//   Serial.print(F("Performing HTTP GET request... "));
//   int err = http.get(resource);
//   if (err != 0) {
//     Serial.println(F("failed to connect"));
//     delay(10000);
//     return;
//   }

//   int status = http.responseStatusCode();
//   Serial.print(F("Response status code: "));
//   Serial.println(status);
//   if (!status) {
//     delay(10000);
//     return;
//   }

//   Serial.println(F("Response Headers:"));
//   while (http.headerAvailable()) {
//     String headerName  = http.readHeaderName();
//     String headerValue = http.readHeaderValue();
//     Serial.println("    " + headerName + " : " + headerValue);
//   }

//   int length = http.contentLength();
//   if (length >= 0) {
//     Serial.print(F("Content length is: "));
//     Serial.println(length);
//   }
//   if (http.isResponseChunked()) {
//     Serial.println(F("The response is chunked"));
//   }

//   String body = http.responseBody();
//   Serial.println(F("Response:"));
//   Serial.println(body);

//   Serial.print(F("Body length is: "));
//   Serial.println(body.length());

//   // Shutdown

//   http.stop();
//   Serial.println(F("Server disconnected"));

//   // Serial.println("Performing HTTP GET request... ");
//   // http.get(server);
//   // while (http.available()) {
//   //   char c = http.read();
//   //   Serial.print(c);
//   // }
//   // http.stop();
//   // Serial.flush();
//   // delay(5000);

//   // Shutdown
  
//   Serial.println(("Server disconnected"));
  
//   Serial.println("\n---End of GPRS TEST---\n");

//   modem.gprsDisconnect();
//   if (!modem.isGprsConnected()) {
//     Serial.println("GPRS disconnected");
//   } else {
//     Serial.println("GPRS disconnect: Failed.");
//   }
//   while(1)  {
//      modem.maintain();
//   }

// }