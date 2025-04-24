#include "Arduino.h"
#include "Cellular.h"
#include "rtc_handler.h"
#include "SPI.h"
#include "SD.h"
#include "time.h"

// ESP32 LilyGO T-SIM7000G SD Card Pins
#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
#define SD_TAG "[SD_CARD]"
#define SIM_TAG "[SIM_CARD]"


const char server[]   = "https://seawall.fiu.edu";   //                                          //Domain where you will be sending/receiving data
const char resource[] = "/";                                                    //The path to where you are sending the data
const char  port[]       = "443";                                                                //https 443 port, could be port 80 for regular http

// status variables
bool cardMount = false;
bool isConnected = false;

// global variables
const char* JSON_DIR_PATH = "/jsonFiles";
const char* CSV_DIR_PATH = "/csvFiles";

tm timeinfo;

struct SensorData {
    float humidity;
    bool humidityValid  = true;
    float temperature;
    bool temperatureValid= true;
    float turbidity;
    bool turbidityValid= true;
    float salinity;
    bool salinityValid= true;
    float tds;
    bool tdsValid= true;
    float ec;
    bool ecValid = true;
    float pH ;
    bool pHValid= true;
    float oxygenLevel;
    bool oxygenLevelValid= true;
    unsigned int jsonLength;
};

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Hello! This is the ESP32 Cellular Database Setup script. ");

    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);        //SD config

    if(!SD.begin(SD_CS, SPI, 4000000))    { 
        Serial.println("SD Mount failed!");
    }else{
        Serial.println("SD Mount successful!");
    }

    sim.begin();
    if(sim.isGprsConnected())  {
        String msg = SIM_TAG + String ("Cellular connected");
        Serial.println(msg);
        isConnected = true;
    }
    getCurrentTime(timeinfo);   //get current time from cellular network
    Serial.println("Setup done");
    timeinfo = get_current_time();    //gets current time

    sim.serverConnect(server, resource);
    sim.setJsonHeader(); //sets up a premade header for https post *working*
    sim.sendGetRequest(server, resource); //sends get request to server
}

void loop() {
    
    Serial.println("infinite loop");
    delay(1000);



}

template<typename T>
const char* getTypeName() {
    return "unknown";
}
template<>
const char* getTypeName<int>() {
    return "int";
}

template<>
const char* getTypeName<float>() {
    return "float";
}

template<>
const char* getTypeName<char>() {
    return "char";
}

String prepareSensorSpecs(const SensorData& data)   {
    StaticJsonDocument<256> doc;
    doc["type"] = "ENUM('" + String(getTypeName<decltype(data.humidity)>()) +"', '" + String(getTypeName<decltype(data.temperature)>()) +
        "', '" + String(getTypeName<decltype(data.turbidity)>()) + "', '" + String(getTypeName<decltype(data.salinity)>()) +
        "', '" + String(getTypeName<decltype(data.tds)>()) + "', '" + String(getTypeName<decltype(data.ec)>()) + "', '" + 
        String(getTypeName<decltype(data.pH)>()) + "', '" + String(getTypeName<decltype(data.oxygenLevel)>()) + "', '" +
        String(getTypeName<decltype(timeinfo.tm_year)>()) + "', '" + String(getTypeName<decltype(timeinfo.tm_mon)>()) + "', '" +
        String(getTypeName<decltype(timeinfo.tm_mday)>()) + "', '" + String(getTypeName<decltype(timeinfo.tm_hour)>()) + "', '" +
        String(getTypeName<decltype(timeinfo.tm_min)>()) + "', '" + String(getTypeName<decltype(timeinfo.tm_sec)>()) + "')";
    doc["version"] = "1";

    //need to hardcode labels because apparently at runtime there is no way to get label names :P
    doc["readValLabel"] = "(humidity, temperature, turbidity, salinity, tds, ec, pH, oxygenLevel, year, month, day, hour, minute, second)";

    String rv;
    serializeJson(doc, rv);
    return rv;
}

String prepareSensorArraySpecs(const SensorData& data)   {
    StaticJsonDocument<256> doc;
    doc["type"] = "ENUM('" + String(getTypeName<decltype(data.humidity)>()) +"', '" + String(getTypeName<decltype(data.temperature)>()) +
        "', '" + String(getTypeName<decltype(data.turbidity)>()) + "', '" + String(getTypeName<decltype(data.salinity)>()) +
        "', '" + String(getTypeName<decltype(data.tds)>()) + "', '" + String(getTypeName<decltype(data.ec)>()) + "', '" + 
        String(getTypeName<decltype(data.pH)>()) + "', '" + String(getTypeName<decltype(data.oxygenLevel)>()) + "')";
    doc["version"] = "1";

    String rv;
    serializeJson(doc, rv);
    return rv;
}

String prepareBoxes()   {
    StaticJsonDocument<256> doc;
    doc["locationID"] = "ENUM('int')";
    doc["nickname"] = "data_box1";
    doc["boxPublic"] = "1";
    doc["deployDate"] = String(timeinfo.tm_year) + ":" + String(timeinfo.tm_mon) + ":" + String(timeinfo.tm_mday);

    String rv;
    serializeJson(doc, rv);
    return rv;
}

String declareSensorArray(const SensorData& data) {
    StaticJsonDocument<256> doc;
    doc["sensor$array_specificationID"] = "INT";
    doc["boxID"] = "INT";

    String rv;
    serializeJson(doc, rv);
    return rv;
}

String declareSensor()  {
    StaticJsonDocument<256> doc;
    doc["sensor_specificationID"] = "INT";
    doc["serialNumber"] = "VARCHAR(100)";
    doc["sensor$arrayID"] = "INT";

    String rv;
    serializeJson(doc, rv);
    return rv;
}


String prepareJsonPayload(const SensorData& data) {     
    const tm& timeinfo = get_current_time();    //gets current time
    StaticJsonDocument<1024> doc;

    //Creates readInfo array
    doc["SensorArrayID"] = "";    //creates empty array called readInfo
    JsonArray jsArr = doc.createNestedArray("readInfo");        //creates huge array called readInfo

    //creates humidity sensor object
    JsonObject humDoc = jsArr.createNestedObject();     //stores object in array for humidity
    humDoc["id"] = "";
    humDoc["type"] = "float";
    humDoc["value"] = String(data.humidity,3);

    //creates a temperature sensor object
    JsonObject tempDoc = jsArr.createNestedObject();    //object for temp
    tempDoc["id"] = "pending";
    tempDoc["type"] = "float";
    tempDoc["value"] = String(data.temperature,3);

    //creates a turbidity sensor object
    JsonObject turbDoc = jsArr.createNestedObject();   //object for turbidity
    turbDoc["id"] = "pending";
    turbDoc["type"] = "float";
    turbDoc["value"] = String(data.turbidity, 3);

    //creates a salinity sensor object
    JsonObject salDoc = jsArr.createNestedObject();     //object for salinity
    salDoc["id"] = "pending";
    salDoc["type"] = "float";
    salDoc["value"] = String(data.salinity,3);

    //creates a conductivity sensor object
    JsonObject ecDoc = jsArr.createNestedObject();      //object for EC
    ecDoc["id"] = "pending";
    ecDoc["type"] = "float";
    ecDoc["value"] = String(data.ec,3);

    //creates a tds object
    JsonObject tdsDoc = jsArr.createNestedObject();     //object for TDS
    tdsDoc["id"] = "pending";
    tdsDoc["type"] = "float";
    tdsDoc["value"] = String(data.tds,3);

    //creates a ph object
    JsonObject phDoc = jsArr.createNestedObject();      //object for pH
    phDoc["id"] = "pending";
    phDoc["type"] = "float";
    phDoc["value"] = String(data.pH, 3);

    //creates a dissolved oxygen object
    JsonObject doDoc = jsArr.createNestedObject();      //object for DO
    doDoc["id"] = "pending";
    doDoc["type"] = "float";
    doDoc["value"] = String(data.oxygenLevel, 3);

    // Add date object
    JsonObject dateObj = doc.createNestedObject("date");    //object for date
    dateObj["year"] = String(timeinfo.tm_year);
    dateObj["month"] = String(timeinfo.tm_mon + 1);
    dateObj["day"] = String(timeinfo.tm_mday);
    dateObj["hour"] = String(timeinfo.tm_hour);
    dateObj["minute"] = String(timeinfo.tm_min);
    dateObj["second"] = String(timeinfo.tm_sec);

    // Add arrayInfo object
    JsonObject arrayInfo = doc.createNestedObject("arrayInfo");     //object for sensor array ID
    arrayInfo["id"] = "1";
    arrayInfo["passphrase"] = "randomText";

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    return jsonPayload;
}

