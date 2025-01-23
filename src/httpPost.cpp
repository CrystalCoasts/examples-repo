#define TINY_GSM_MODEM_SIM7000SSL
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

#include <Arduino.h>
#include <Wire.h>
#include <set>
#include <string>
#include "TinyGsmClient.h"
#include <Ticker.h>
#include <HttpClient.h>
#include <SSLClient.h>
#include <ArduinoJson.h>
#include <time.h>

#define mySerial2 Serial1
#define UART_BAUD           115200
#define PIN_DTR             25
#define PIN_TX              27
#define PIN_RX              26
#define PWR_PIN             4
#define LED_PIN             12

// set GSM PIN, if any
#define GSM_PIN ""
#define GSM_NL "\n"

// Your GPRS credentials, if any
const char apn[]  =  "m2mglobal"; //"iot.1nce.net";     //SET TO YOUR APN
const char gprsUser[] = "";
const char gprsPass[] = "";

// Define these in a suitable header file or at the top of your source file
const char* KEY_HUMIDITY = "humidity";
const char* KEY_TEMPERATURE = "temperature";
const char* KEY_TURBIDITY = "turbidity";
const char* KEY_SALINITY = "salinity";
const char* KEY_TDS = "tds";
const char* KEY_PH = "pH";
const char* KEY_OXYGEN_LEVEL = "oxygenLevel";
const char* KEY_MONTH = "Month";
const char* KEY_DAY = "Day";
const char* KEY_YEAR = "Year";
const char* KEY_HOUR = "Hour";
const char* KEY_MINUTE = "Minute";
const char* KEY_SECOND = "Second";
String RTC_TAG ="[RTC_TAG] ";


std::string sendData(String command);
void modemRestart();
void modemPowerOff();
void modemPowerOn();
struct tm getCurrentTime();
void sendPostRequest();
void sendPostRequest(String jsonPayload, const char* server, const char* resource);
void sendGetRequest();
String prepareJson();
uint8_t printLocalTime();
void updateSystemTime(const struct tm& newTime);


#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(mySerial2, Serial);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(mySerial2);
#endif

// Server details
// const char server[]   = "dlptest.com";
// const char resource[] = "/https-post/";
const char server[]   = "https://d17e66a7-c349-4d03-9453-cf90701e7aaa.mock.pstmn.io";
const char resource[] = "/post";
const int  port       = 443;
TinyGsmClientSecure client(modem);
HttpClient http(client, server, port);

int counter, lastIndex, numberOfPieces = 24;
String pieces[24], input;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello! ESP32-S3 AT command V1.1 Test");

  // Set LED OFF
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);

  //GSM Start
  mySerial2.begin(UART_BAUD,SERIAL_8N1, PIN_RX, PIN_TX);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  if (!modem.restart()) {
    Serial.println("Failed to restart modem, attempting to continue without restarting");
  }

}

void loop() {
    // Restart takes quite some time
  // To skip it, call init() instead of restart()



  Serial.println("Initializing modem...");
  if (!modem.init()) {    //dont need
    Serial.println("Failed to restart modem, attempting to continue without restarting");
  }

  String name = modem.getModemName();
  delay(500);
  Serial.println("Modem Name: " + name);

  String modemInfo = modem.getModemInfo();
  delay(500);
  Serial.println("Modem Info: " + modemInfo);

  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
      modem.simUnlock(GSM_PIN);
  }
  // modem.sendAT("+CFUN=0 ");
  // if (modem.waitResponse(10000L) != 1) {
  //   DBG(" +CFUN=0  false ");
  // }
  // delay(200);
  sendData("AT+CFUN=0");
  delay(200);
  /*
    2 Automatic
    13 GSM only
    38 LTE only
    51 GSM and LTE only
  * * * */
  String res;
  // CHANGE NETWORK MODE, IF NEEDED
  res = modem.setNetworkMode(2);
  if (res != "1") {
    DBG("setNetworkMode  false ");
    return ;
  }
  delay(200);
  
  //sendData("AT++CNMP=2"); //set network mode to automatic


  /*
    1 CAT-M
    2 NB-Iot
    3 CAT-M and NB-IoT
  * * */
  // CHANGE PREFERRED MODE, IF NEEDED
  res = modem.setPreferredMode(1);
  if (res != "1") {
    DBG("setPreferredMode  false ");
    return ;
  }
  delay(200);

  //sendData("AT+CMNB=1");  //changes mode to CAT-M

  /*AT+CBANDCFG=<mode>,<band>[,<band>…]
   * <mode> "CAT-M"   "NB-IOT"
   * <band>  The value of <band> must is in the band list of getting from  AT+CBANDCFG=?
   * For example, my SIM card carrier "NB-iot" supports B8.  I will configure +CBANDCFG= "Nb-iot ",8
   */
  /* modem.sendAT("+CBANDCFG=\"NB-IOT\",8 ");*/
  
  /* if (modem.waitResponse(10000L) != 1) {
       DBG(" +CBANDCFG=\"NB-IOT\" ");
   }*/
  delay(200);
  //modem.sendAT("+CFUN=1 ");
  sendData("AT+CFUN=1");
  delay(200);
  // if (modem.waitResponse(10000L) != 1) {
  //   DBG(" +CFUN=1  false ");
  // }
  // delay(200);

  mySerial2.println("AT+CGDCONT?");
  delay(500);
  if (mySerial2.available()) {
    input = mySerial2.readString();
    for (int i = 0; i < input.length(); i++) {
      if (input.substring(i, i + 1) == "\n") {
        pieces[counter] = input.substring(lastIndex, i);
        lastIndex = i + 1;
        counter++;
       }
        if (i == input.length() - 1) {
          pieces[counter] = input.substring(lastIndex, i);
        }
      }
      // Reset for reuse
      input = "";
      counter = 0;
      lastIndex = 0;

      for ( int y = 0; y < numberOfPieces; y++) {
        for ( int x = 0; x < pieces[y].length(); x++) {
          char c = pieces[y][x];  //gets one byte from buffer
          if (c == ',') {
            if (input.indexOf(": ") >= 0) {
              String data = input.substring((input.indexOf(": ") + 1));
              if ( data.toInt() > 0 && data.toInt() < 25) {
                modem.sendAT("+CGDCONT=" + String(data.toInt()) + ",\"IP\",\"" + String(apn) + "\",\"0.0.0.0\",0,0,0,0");
              }
              input = "";
              break;
            }
          // Reset for reuse
          input = "";
         } else {
          input += c;
         }
      }
    }
  } else {
    Serial.println("Failed to get PDP!");
  }

  Serial.println("\n\n\nWaiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) {
    Serial.println("Network connected");
  }
  
  // --------TESTING GPRS--------
  Serial.println("\n---Starting GPRS TEST---\n");
  Serial.println("Connecting to: " + String(apn));
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    delay(10000);
    return;
  }

  Serial.print("GPRS status: ");
  if (modem.isGprsConnected()) {
    Serial.println("connected");
  } else {
    Serial.println("not connected");
  }

  String ccid = modem.getSimCCID();
  Serial.println("CCID: " + ccid);

  String imei = modem.getIMEI();
  Serial.println("IMEI: " + imei);

  String cop = modem.getOperator();
  Serial.println("Operator: " + cop);

  IPAddress local = modem.localIP();
  Serial.println("Local IP: " + String(local));

  int csq = modem.getSignalQuality();
  Serial.println("Signal quality: " + String(csq));

  mySerial2.println("AT+CPSI?");     //Get connection type and band
  delay(500);
  if (mySerial2.available()) {
    String r = mySerial2.readString();
    Serial.println(r);
  }

  sendData("AT+SHSSL?");
  
  // CLK STUFF
  sendData("AT+CLTS=1"); // set time zone to local/auto
  String time = sendData("AT+CCLK?").c_str();
  int startTime = time.charAt('"');


struct tm timeinfo = getCurrentTime();
updateSystemTime(timeinfo);

//HTTPS POST
sendPostRequest(prepareJson(), server, resource);
//sendPostRequest();


// Shutdown
  
  Serial.println(("Server disconnected"));
  
  Serial.println("\n---End of GPRS TEST---\n");

  modem.gprsDisconnect();
  if (!modem.isGprsConnected()) {
    Serial.println("GPRS disconnected");
  } else {
    Serial.println("GPRS disconnect: Failed.");
  }
  while(1)  {
     modem.maintain();
  }

}

void sendGetRequest() {
  sendData("AT+CSSLCFG=\"sslversion\",1,3");
  sendData("AT+CCLK?");
  sendData("AT+SHSSL=1,\"\"");
  sendData("AT+SHCONF=\"BODYLEN\",1024");
  sendData("AT+SHCONF=\"HEADERLEN\",350");
  sendData("AT+SHCONF=\"URL\",\"https://httpbin.org\"");
  if(sendData("AT+SHCONN").find("ERROR") !=std::string::npos)  {
    Serial.println("Error found! Could not connnect!");
    return;
  }
  sendData("AT+SHSTATE?");
  sendData("AT+SHCHEAD");
  //sendData("AT+SHAHEAD=\"X-SSL-ENABLE\", \"1\"");
  sendData("AT+SHAHEAD=\"Content-Type\", \"application/json\"");
  sendData("AT+SHAHEAD=\"User-Agent\",\"curl/7.47.0\"");
  sendData("AT+SHAHEAD=\"Cache-control\", \"no-cache\"");
  sendData("AT+SHAHEAD=\"Connection\", \"keep- alive\"");
  sendData("AT+SHAHEAD=\"Accept\", \"application/json\"");
  sendData("AT+SHREQ=\"/get\", 1");
  sendData("AT+SHREAD=0, 300");
  sendData("AT+SHDISC");
}


void sendPostRequest() {
  Serial.println("Sending POST request...");

  sendData("AT+CSSLCFG=\"sslversion\",1,3");
  sendData("AT+CSSLCFG=\"sni\",1,\"https://d17e66a7-c349-4d03-9453-cf90701e7aaa.mock.pstmn.io\"");
  sendData("AT+CCLK?");
  sendData("AT+SHSSL=1,\"\"");
  sendData("AT+SHCONF=\"BODYLEN\",1024");
  sendData("AT+SHCONF=\"HEADERLEN\",350");
  sendData("AT+SHCONF=\"URL\",\"https://d17e66a7-c349-4d03-9453-cf90701e7aaa.mock.pstmn.io\"");

  if(sendData("AT+SHCONN").find("ERROR") != std::string::npos)  {
    Serial.println("Error found! Could not connnect!");
    return;
  }
  sendData("AT+SHSTATE?");
  sendData("AT+SHCHEAD");
  sendData("AT+SHAHEAD=\"Content-Type\", \"application/json\"");

  sendData("AT+SHAHEAD=\"User-Agent\",\"curl/7.47.0\"");
  sendData("AT+SHAHEAD=\"Cache-control\", \"no-cache\"");
  sendData("AT+SHAHEAD=\"Connection\", \"keep-alive\"");
  sendData("AT+SHAHEAD=\"Accept\", \"*/*\"");    //experimental
  sendData("AT+SHBOD=\"{\\\"success\\\": \\\"true\\\"}\",19");
  sendData("AT+SHBOD?");
  sendData("AT+SHREQ=\"/post\", 3");
  sendData("AT+SHREAD=0, 6");
  sendData("AT+SHDISC");
}

String prepareJson()  {
  StaticJsonDocument<256> doc;
  doc[KEY_HUMIDITY] = String(100);
  doc[KEY_TEMPERATURE] = String(78);
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  jsonPayload.replace("\"", "\\\"");
  jsonPayload = "\"" + jsonPayload + "\""; 
  
  //jsonPayload = "\"{\\\"humidity\\\": \\\"100\\\"}\"";    //WORKS!!!
  // , \\\"temperature\\\": \\\"78\\\"
  return jsonPayload;
}

void sendPostRequest(String jsonPayload, const char* server, const char* resource) {
  Serial.println("Sending POST request...");

  sendData("AT+CSSLCFG=\"sslversion\",1,3");
  sendData("AT+CSSLCFG=\"sni\",1," + String(server));
  sendData("AT+CCLK?");
  sendData("AT+SHSSL=1,\"\"");
  sendData("AT+SHCONF=\"BODYLEN\",1024");
  sendData("AT+SHCONF=\"HEADERLEN\",350");
  sendData("AT+SHCONF=\"URL\"," + String(server));
  if(sendData("AT+SHCONN").find("ERROR") != std::string::npos)  {
    Serial.println("Error found! Could not connnect!");
    return;
  }
  sendData("AT+SHSTATE?");
  sendData("AT+SHCHEAD");
  sendData("AT+SHAHEAD=\"Content-Type\", \"application/json\"");

  sendData("AT+SHAHEAD=\"User-Agent\",\"curl/7.47.0\"");
  sendData("AT+SHAHEAD=\"Cache-control\", \"no-cache\"");
  sendData("AT+SHAHEAD=\"Connection\", \"keep-alive\"");
  sendData("AT+SHAHEAD=\"Accept\", \"*/*\"");    //experimental
  Serial.println(jsonPayload);

  int jsonBits = 0;
  for(char c: jsonPayload)  {
    if(c==':')  {
      jsonBits++;
    }
  }
  int jsonLength = jsonPayload.length() -2 -(jsonBits*4);

  sendData("AT+SHBOD=" + jsonPayload + "," + String(jsonLength));
  sendData("AT+SHBOD?");
  sendData("AT+SHREQ=" + String(resource) + ", 3");
  sendData("AT+SHREAD=0, 7");
  sendData("AT+SHDISC");

}

struct tm getCurrentTime() {
    struct tm timeinfo = {0};  // Initialize struct to avoid garbage values

    std::string clk = sendData("AT+CCLK?");
    
    // Extract the quoted time string
    int start = clk.find('"');  // First double quote
    int end = clk.find_last_of('"');  // Last double quote
    if (start == std::string::npos || end == std::string::npos) {
        Serial.println("Error: Could not find time string in response!");
        //return timeinfo; // Return empty struct
        return timeinfo;
    }
    clk = clk.substr(start + 1, end - start - 1);  // Extract time string

    // Parse date and time
    char delimiter = '/';
    int year = 2000 + std::stoi(clk.substr(0, clk.find(delimiter)));  // Convert year properly
    clk.erase(0, clk.find(delimiter) + 1);
    int month = std::stoi(clk.substr(0, clk.find(delimiter)));
    clk.erase(0, clk.find(delimiter) + 1);
    
    delimiter = ',';
    int day = std::stoi(clk.substr(0, clk.find(delimiter)));
    clk.erase(0, clk.find(delimiter) + 1);
    
    delimiter = ':';
    int hour = std::stoi(clk.substr(0, clk.find(delimiter)));
    clk.erase(0, clk.find(delimiter) + 1);
    int minute = std::stoi(clk.substr(0, clk.find(delimiter)));
    clk.erase(0, clk.find(delimiter) + 1);

    delimiter = '-';
    int second = std::stoi(clk.substr(0, clk.find(delimiter)));
    
    // Populate struct tm
    timeinfo.tm_year = year - 1900;  // Years since 1900
    timeinfo.tm_mon = month - 1;  // Months are 0-based
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;

    Serial.printf("Parsed Time: %04d/%02d/%02d %02d:%02d:%02d\n", 
                  year, month, day, hour, minute, second);

    return timeinfo;
}

void updateSystemTime(const struct tm& newTime) {
    String msg;
    // Convert tm struct to time_t
    time_t t = mktime((struct tm *)&newTime);
    
    // Set system time
    timeval tv = { t, 0 };
    settimeofday(&tv, NULL); // Update the system time with new time

    // Logging for debugging
    msg = RTC_TAG + "System time updated successfully.";
    Serial.println(msg);
    // For further verification, you might want to print the new time
    printLocalTime();
}


// struct tm getCurrrentTime() {
//     String msg;
//     struct tm timeinfo;
//     std::string clk = sendData("AT+CCLK?");
//     int start = clk.find('"');  // First double quote
//     int end = clk.find_last_of('"');  // Last double quote
//     if (start == std::string::npos || end == std::string::npos) {
//         Serial.println("Error: Could not find time string in response!");
//         return timeinfo; // Return empty struct
//     }
//     clk = clk.substr(start + 1, end - start - 1);  // Extract time string
//     char delimiter = '/';
//     clk = clk.substr(start+1,end-1);
//     int year = std::stoi(clk.substr(0, clk.find(delimiter)+1));
//     clk.erase(0, clk.find(delimiter));
//     int month = std::stoi(clk.substr(0,clk.find(delimiter)+1));
//     clk.erase(0, clk.find(delimiter));

//     delimiter = ',';
//     int day = std::stoi(clk.substr(0, clk.find(delimiter)+1));
//     clk.erase(0, clk.find(delimiter));

//     delimiter = ':';
//     int hour = std::stoi(clk.substr(0, clk.find(delimiter)+1));
//     clk.erase(0, clk.find(delimiter));
//     int minute = std::stoi(clk.substr(0, clk.find(delimiter)+1));
//     clk.erase(0, clk.find(delimiter));

//     delimiter = '-';
//     int second = std::stoi(clk.substr(0, clk.find(delimiter)+1));
//     clk.erase(0, clk.find(delimiter));

//     Serial.printf("Time: %d/%d/%d %d:%d:%d\n", year, month, day, hour, minute, second);
//     return timeinfo;
// }

uint8_t printLocalTime() {
    String msg;
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        msg =  RTC_TAG + " Time not set yet.";
        Serial.print(msg);
        return 1;
    }
    char timeStr[80]; // Ensure the buffer is large enough to hold the resulting string
    strftime(timeStr, sizeof(timeStr), "%A, %B %d, %Y %H:%M:%S", &timeinfo);
    msg =  RTC_TAG + String(timeStr);
    Serial.println(msg);
    // Serial.println(&timeinfo, "%A, %B %d, %Y %H:%M:%S");
    return 0;
}

void modemPowerOn(){
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(1000);
  digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff(){
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(1500);
  digitalWrite(PWR_PIN, HIGH);
}


void modemRestart(){
  modemPowerOff();
  delay(1000);
  modemPowerOn();
}

std::string sendData(String command) {
  // Send the AT command
  mySerial2.println(command);
  Serial.println(command);

  // Variables to store the response
  std::string buffer = "";
  unsigned long startTime = millis();
  const unsigned long timeout = 60000; // Timeout in milliseconds

  // Read the response
  while (millis() - startTime < timeout) {
    while (mySerial2.available()) {
      char c = mySerial2.read();
      buffer += c;

      // Check for termination keywords
      if (buffer.find("OK") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
        break;
      }
    }
    // Break if we already found "OK" or "ERROR"
    if (buffer.find("OK") != std::string::npos || buffer.find("ERROR") != std::string::npos) {
      break;
    }
  }
  // Print the response to the serial monitor
  Serial.println(buffer.c_str());
  delay(2000);
  return buffer;
}
