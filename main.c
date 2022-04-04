#define PIN_A      2
#define PIN_B      4
#define DEBONCE_TO 150
#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Replace with your network credentials
const char* ssid     = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// REPLACE with your Domain name and URL path or IP address with path
const char* serverName = "http://adresse_ip/post-esp-data.php";

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "tPmAT5Ab3j7F9";

//original
volatile bool turnedCW = false;
volatile bool turnedCCW = false;

unsigned long debounceTime = 0;

bool lastWasCW = false;
bool lastWasCCW = false;
int Pos = 0; 
int State;
int LastState;  
const float pi = 31.4;
const float R = 40;
const int N = 40;
int distance = 0;
void checkEncoder() {
  if ((!turnedCW)&&(!turnedCCW)) {
    int pinA = digitalRead(PIN_A);
    delayMicroseconds(1500);
    int pinB = digitalRead(PIN_B);
    if (pinA == pinB){
      if (lastWasCW) {
        turnedCW = true;
      } else {
        turnedCCW = true;
      }      
    } else {
      if (lastWasCCW) {
        turnedCCW = true;
      } else {
        turnedCW = true;
      }
    }    
  }
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) { 
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.begin(115200);
    
    pinMode(PIN_A, INPUT_PULLUP);
    pinMode(PIN_B, INPUT_PULLUP);
    //  pinMode(PIN_BUTTON, INPUT_PULLUP);
    attachInterrupt(PIN_B, checkEncoder, CHANGE);
    Serial.println("Reading from encoder: ");
}

void loop() {
    static int Pos = 0;
    //if (!digitalRead(PIN_BUTTON)) {
    //Pos = 0;
    // Serial.print("Reset: ");
    // Serial.println(distance);
    //}
    if (turnedCW) {
        Pos++;
        distance = ((2*pi*R)/N) * Pos ;
        Serial.print("turnedCW:  ");
        Serial.println(distance);
        Serial.println("Mm");
        turnedCW = false;
        lastWasCW = true;
        debounceTime = millis();
    }
    if (turnedCCW) {
        Pos--;
        distance = ((2*pi*R)/N) * Pos ;
        Serial.print("turnedCCW:   ");
        Serial.println(distance );
        Serial.println("Mm");
        turnedCCW = false;
        lastWasCCW = true;
        debounceTime = millis();
    }
    if ((millis()-debounceTime) > DEBONCE_TO) {
        lastWasCW = false;
        lastWasCCW = false;
    }
    if(WiFi.status()== WL_CONNECTED){
        WiFiClient client;
        HTTPClient http;
        
        // Your Domain name with URL path or IP address with path
        http.begin(client, serverName);
        
        // Specify content-type header
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        
        // Prepare your HTTP POST request data
        String httpRequestData = "api_key=" + apiKeyValue + "&distance=" + distance+"";
        Serial.print("httpRequestData: ");
        Serial.println(httpRequestData);
        
        // You can comment the httpRequestData variable above
        // then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
        //String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&location=Office&value1=24.75&value2=49.54&value3=1005.14";

        // Send HTTP POST request
        int httpResponseCode = http.POST(httpRequestData);
        
        // If you need an HTTP request with a content type: text/plain
        //http.addHeader("Content-Type", "text/plain");
        //int httpResponseCode = http.POST("Hello, World!");
        
        // If you need an HTTP request with a content type: application/json, use the following:
        //http.addHeader("Content-Type", "application/json");
        //int httpResponseCode = http.POST("{\"value1\":\"19\",\"value2\":\"67\",\"value3\":\"78\"}");
            
        if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        }
        else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        }
        // Free resources
        http.end();
    }
}