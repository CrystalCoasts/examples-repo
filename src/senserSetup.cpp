#include "Arduino.h"
#include "Cellular.h"
#include "rtc_handler.h"
#include "SPI.h"
#include "SD.h"
#include "time.h"
#include "FS.h"
#include "base64.h"

// ESP32 LilyGO T-SIM7000G SD Card Pins
#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
#define SD_TAG "[SD_CARD]"
#define SIM_TAG "[SIM_CARD]"


const char server[]   = "seawall.fiu.edu";//"https://128bdb57-9d10-4eb7-b3db-3aa86f885e1c.mock.pstmn.io"; //"https://seawall.fiu.edu";   //                                          //Domain where you will be sending/receiving data
const char resource[] =  "/sensorReads";   //"/post";                                                 //The path to where you are sending the data
const char  port[]       = "443";                                                                //https 443 port, could be port 80 for regular http

// status variables
bool cardMount = false;
bool isConnected = false;

// global variables
const char* JSON_DIR_PATH = "/jsonFiles";
const char* CSV_DIR_PATH = "/csvFiles";

tm timeinfo;

struct SensorData {
    float humidity= 0;
    bool humidityValid  = true;
    float temperature=0;
    bool temperatureValid= true;
    float turbidity=0;
    bool turbidityValid= true;
    float salinity=0;
    bool salinityValid= true;
    float tds=0;
    bool tdsValid= true;
    float ec=0;
    bool ecValid = true;
    float pH=0;
    bool pHValid= true;
    float oxygenLevel=0;
    bool oxygenLevelValid= true;
    unsigned int jsonLength;
}data;

String prepareJsonPayload(const SensorData& data); //forward declaration
String preparePhotoPayload(char* photo, int size);

const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const int LINE_WRAP = 76;      // Wrap Base64 output every 76 characters (standard)

/**
   * Encodes a block of 1, 2, or 3 bytes into Base64 characters
   * and writes them into the provided buffer.
   */
  void encodeBlockToBuffer(byte *in, int len, char *buffer, int *bufferPos) {
    byte out[4]; // Holds indices (0-63)
  
    // Calculate Base64 indices (same logic as before)
    out[0] = (len > 0) ? (in[0] >> 2) & 0x3F : 0;
    out[1] = (len > 0) ? ((in[0] << 4) & 0x30) : 0;
    if (len > 1) out[1] |= ((in[1] >> 4) & 0x0F);
    out[2] = (len > 1) ? ((in[1] << 2) & 0x3C) : 0;
    if (len > 2) out[2] |= ((in[2] >> 6) & 0x03);
    out[3] = (len > 2) ? (in[2] & 0x3F) : 0;
  
    // Write Base64 characters or padding to buffer
    if (len > 0) {
      buffer[*bufferPos] = base64Chars[out[0]];
      (*bufferPos)++;
      buffer[*bufferPos] = base64Chars[out[1]];
      (*bufferPos)++;
    }
  
    if (len > 1) {
      buffer[*bufferPos] = base64Chars[out[2]];
      (*bufferPos)++;
    } else if (len > 0) { // Only add padding if there was at least 1 byte
      buffer[*bufferPos] = '='; // Pad if len == 1
      (*bufferPos)++;
    }
  
    if (len > 2) {
      buffer[*bufferPos] = base64Chars[out[3]];
      (*bufferPos)++;
    } else if (len > 0) { // Only add padding if there was at least 1 byte
      buffer[*bufferPos] = '='; // Pad if len == 1 or len == 2
      (*bufferPos)++;
    }
  }

char* readFileToBase64(File dataFile) {
    if (!dataFile) {
      return NULL;
    }
  
    long fileSize = dataFile.size();
    if (fileSize == 0) {
      Serial.println("Warning: File is empty.");
      // Return an empty, null-terminated string
      char* emptyString = (char*)malloc(1);
      if (emptyString) {
        emptyString[0] = '\0';
      }
      return emptyString; // Remember to free this too!
    }
  
    // Calculate required buffer size for Base64 string
    // Each 3 bytes of input become 4 bytes of output.
    // Add space for padding and the null terminator.
    // Use long long for calculation to avoid overflow with large file sizes
    long long requiredSize = ((fileSize + 2LL) / 3LL) * 4LL + 1LL; // +1 for '\0'
  
    // --- Memory Allocation ---
    Serial.print("Attempting to allocate ");
    Serial.print((long)requiredSize); // Cast to long for printing if it fits
    Serial.println(" bytes for Base64 string...");
  
    // Check if requiredSize exceeds reasonable limits for the specific Arduino board
    // This is a rough check, actual available memory might be less
    // For ATmega328 (Uno/Nano) - 2KB SRAM total. Be very conservative.
    #if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)
      const unsigned long MAX_ALLOCATION = 1000; // Example limit: 1KB
    #else
      // For ESP32/ESP8266/Due etc. - adjust as needed, much higher limit possible
      const unsigned long MAX_ALLOCATION = 160000; // Example limit: 64KB
    #endif
  
    if (requiredSize <= 0 || requiredSize > MAX_ALLOCATION) {
        Serial.print("Error: Required allocation size (");
        Serial.print((long)requiredSize);
        Serial.print(") exceeds limit (");
        Serial.print(MAX_ALLOCATION);
        Serial.println(") or is invalid.");
        return NULL;
    }
  
  
    char *base64Buffer = (char *)malloc(requiredSize * sizeof(char));
  
    if (base64Buffer == NULL) {
      Serial.println("Error: malloc failed! Not enough memory?");
      return NULL; // Allocation failed
    }
    Serial.println("Memory allocated successfully.");
    // --- End Memory Allocation ---
  
  
    byte inputBuffer[3];
    int bytesRead;
    int bufferPosition = 0; // Current writing position in base64Buffer
  
    // Ensure file pointer is at the beginning (if it wasn't already)
    dataFile.seek(0);
  
    while (dataFile.available() > 0) {
      bytesRead = dataFile.read(inputBuffer, 3);
      if (bytesRead > 0) {
        encodeBlockToBuffer(inputBuffer, bytesRead, base64Buffer, &bufferPosition);
      }
    }
  
    // Null-terminate the resulting string
    base64Buffer[bufferPosition] = '\0';
  
    return base64Buffer;
  }
  
char* base64String = ""; // Pointer to hold the Base64 string

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

    // sim.serverConnect(server, resource);
    // sim.setJsonHeader(); //sets up a premade header for https post *working*
    // sim.sendGetRequest(server, resource); //sends get request to server

    String directoryPath = "/photos";
    File dir;
    String encoded = "";

    if (!SD.exists(directoryPath)) {
      SD.mkdir(directoryPath);
      Serial.println("Made directory for photos!");
    }
    
    dir = SD.open(directoryPath);
    if (!dir || !dir.isDirectory()) {
      Serial.println("Couldn't open directory.");
      return;
    }
    
    File photoFile;
;
      if(photoFile = dir.openNextFile()) {
        Serial.print("Reading file: ");
        Serial.println(photoFile.name());

        uint8_t buffer[3];
        size_t bytesRead = 0;
        
        // Calculate the required size for the Base64 string
        size_t fileSize = photoFile.size();
        size_t base64Size = (fileSize * 4) / 3 + 1;  // Add 1 for null terminator

        Serial.println("Encoding file to Base64 string in memory...");

        // --- Allocate memory and encode ---
        base64String = readFileToBase64(photoFile);
        // --- Encoding finished ---
      
        // Close the file *after* reading is complete
        photoFile.close();
        Serial.println("File closed.");
      
        if (base64String != NULL) {
          Serial.println("Base64 encoding successful. String stored in memory.");
      
          // --- HERE YOU WOULD USE THE base64String ---
          // For example, build and send your POST request
          Serial.println("--- BEGIN BASE64 STRING (from memory) ---");
          // Print the string (optional, for verification - can be long!)
          // Be careful printing very long strings, it might overwhelm Serial buffer or IDE
          // Consider printing in chunks if needed for debugging.
          Serial.println(base64String);
          Serial.println("--- END BASE64 STRING ---");
      
          Serial.print("Simulating POST request with string at memory address: 0x");
          Serial.println((unsigned long)base64String, HEX);
          Serial.println("...");
          Serial.println("POST request simulation complete.");
          // --- End of usage ---
      
      
          // --- CRITICAL: Free the allocated memory ---
          // Serial.println("Freeing allocated memory...");
          // free(base64String);
          // base64String = NULL; // Good practice to avoid dangling pointers
          // Serial.println("Memory freed.");
      
        } else {
          Serial.println("Base64 encoding failed (likely out of memory).");
        }
      
        Serial.println("Process complete.");
      }else{
        Serial.println("No files found in directory.");
      }

}

void loop() {
    
    while(Serial.available())   {
        // if(Serial.read() == '1')   {
        //     sim.serverConnect(server,resource); //connects to server again to send data
        //     sim.setJsonHeader(); //sets up a premade header for https post *working*
        //     sim.sendPostRequest(prepareJsonPayload(data), resource); //sends post request to server with json payload
        // }
        if(Serial.read() == '1')   {
            sim.serverConnect("128bdb57-9d10-4eb7-b3db-3aa86f885e1c.mock.pstmn.io","/post"); //connects to server again to send data
            sim.setJsonHeaderPhoto(); //sets up a premade header for https post *working*
            sim.sendPhotoPost(preparePhotoPayload(base64String, sizeof(base64String)), "/post"); //sends post request to server with json payload
            sim.sendData("AT+SHDISC"); //disconnects from server after sending data
            free(base64String); //frees memory from base64String
            Serial.println("Memory freed.");
            //sim.sendPostRequest(prepareSensorSpecs(data), resource); //sends post request to server with json payload
        }
    }
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

// String prepareJPG() {

// }

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

String preparePhotoPayload(char* photo, int size) {
    StaticJsonDocument<160000> doc;
    doc["photo"] = photo; // Assuming photo is a Base64 encoded string

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    Serial.println("json processed");
    return jsonPayload;
}


String prepareJsonPayload(const SensorData& data) {     
    const tm& timeinfo = get_current_time();    //gets current time
    StaticJsonDocument<1024> doc;

    //Creates readInfo array
    doc["sensorArrayID"] = "1";    //creates empty array called readInfo
    JsonArray jsArr = doc.createNestedArray("readInfo");        //creates huge array called readInfo

    //creates humidity sensor object
    JsonObject humDoc = jsArr.createNestedObject();     //stores object in array for humidity
    humDoc["id"] = "2";
    humDoc["type"] = "float";
    humDoc["readVal"] = String(data.humidity,3);

    //creates a temperature sensor object
    JsonObject tempDoc = jsArr.createNestedObject();    //object for temp
    tempDoc["id"] = "3";
    tempDoc["type"] = "float";
    tempDoc["readVal"] = String(data.temperature,3);

    //creates a turbidity sensor object
    JsonObject turbDoc = jsArr.createNestedObject();   //object for turbidity
    turbDoc["id"] = "4";
    turbDoc["type"] = "float";
    turbDoc["readVal"] = String(data.turbidity, 3);

    //creates a salinity sensor object
    JsonObject salDoc = jsArr.createNestedObject();     //object for salinity
    salDoc["id"] = "5";
    salDoc["type"] = "float";
    salDoc["readVal"] = String(data.salinity,3);

    //creates a conductivity sensor object
    JsonObject ecDoc = jsArr.createNestedObject();      //object for EC
    ecDoc["id"] = "7";
    ecDoc["type"] = "float";
    ecDoc["readVal"] = String(data.ec,3);

    //creates a tds object
    JsonObject tdsDoc = jsArr.createNestedObject();     //object for TDS
    tdsDoc["id"] = "6";
    tdsDoc["type"] = "float";
    tdsDoc["readVal"] = String(data.tds,3);

    //creates a ph object
    JsonObject phDoc = jsArr.createNestedObject();      //object for pH
    phDoc["id"] = "8";
    phDoc["type"] = "float";
    phDoc["readVal"] = String(data.pH, 3);

    //creates a dissolved oxygen object
    JsonObject doDoc = jsArr.createNestedObject();      //object for DO
    doDoc["id"] = "9";
    doDoc["type"] = "float";
    doDoc["readVal"] = String(data.oxygenLevel, 3);

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

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    return jsonPayload;
}

