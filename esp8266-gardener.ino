#include <AmazonIOTClient.h>
#include <ESP8266AWSImplementations.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include "rtcmem.h"
#include "keys.h"

// keys.cpp contains
// const char* awsRegion = "us-east-1";
// const char* awsDomain = "a14bdnvwk8zpaa.iot.us-east-1.amazonaws.com";
// const char* awsPath = "/things/esp8266-gardener/shadow";
// const char* awsKeyID = ;
// const char* awsSecKey = ;
// const char* ssid = ;
// const char* password = ;

const char* awsEndpoint = "amazonaws.com";

Esp8266HttpClient httpClient;
Esp8266DateTimeProvider dateTimeProvider;

AmazonIOTClient iotClient;
int totalValveOpenMS = 0;
const int sleepSeconds = 60;
const int maxVoltageMillis = 3300;
const int maxA2DValue = 556;
const int valvePin = 15;

ADC_MODE(ADC_VCC);

void setup() {
  pinMode(valvePin, OUTPUT);
  digitalWrite(valvePin, LOW);
  
  initSerial();

  connectToWiFi();

  configureIOTClient();

  if (!tryLoadTotalValveOpenMS())
  {
    startup();
  }
  else
  {
    processIoTShadow();
  }

  delay(100);

  ESP.deepSleep(sleepSeconds * 1000000);
}

void loop() {
}

void initSerial() {
  Serial.begin(115200);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }
}

void connectToWiFi() {
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
}

void configureIOTClient() {
  iotClient.setAWSRegion(awsRegion);
  iotClient.setAWSEndpoint(awsEndpoint);
  iotClient.setAWSDomain(awsDomain);
  iotClient.setAWSPath(awsPath);
  iotClient.setAWSKeyID(awsKeyID);
  iotClient.setAWSSecretKey(awsSecKey);
  iotClient.setHttpClient(&httpClient);
  iotClient.setDateTimeProvider(&dateTimeProvider);
}

void processIoTShadow() {
  ActionError actionError;
  char* json = iotClient.get_shadow(actionError);
  Serial.println(strlen(json));

  Serial.println(json);
  const size_t bufferSize = 600;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()){
    Serial.println("Error parsing json");
    return;
  } else {
    Serial.println("Successfully parsed json");
  }

  if (root.containsKey("state")){
    JsonObject& state = root["state"];
    if (state.containsKey("desired")){
      Serial.println("Shadow has desired state");
      JsonObject& desired = state["desired"];
      JsonObject& reported = state["reported"];

      // Check if battery level has changed by >= 5%
      if (reported.containsKey("bat")){
        int battery_mV = reported["bat"];
        int batteryLevel = getBatteryLevel();
        if (abs(battery_mV - batteryLevel)/(float)batteryLevel >= 0.05){
          sendState(false);
        }
      }
      
      if (desired.containsKey("totalValveOpenMS")) {
        int state_desired_totalValveOpenMS = desired["totalValveOpenMS"];
        int openFor = state_desired_totalValveOpenMS - totalValveOpenMS;
  
        if (openFor > 0) {
          Serial.print("Opening valve for ");
          Serial.println(openFor);
          digitalWrite(valvePin, HIGH);
          delay(openFor);
          Serial.println("Closing valve");
          digitalWrite(valvePin, LOW);
  
          totalValveOpenMS += openFor;
          saveTotalValveOpenMS();
          sendState(false);
        }
      }
    }
  }
}

// On deep sleep, every wake calls setup, not loop.
// Read from persistent RTC memory to see if this is the first time the program has started.
bool tryLoadTotalValveOpenMS()
{
  return rtc_read(&totalValveOpenMS, sizeof totalValveOpenMS);
}

void saveTotalValveOpenMS()
{
  rtc_write(&totalValveOpenMS, sizeof totalValveOpenMS);
}

void startup()
{
  Serial.println("STARTUP");
  rtc_write(&totalValveOpenMS, sizeof totalValveOpenMS);
  sendState(true);
}

void sendState(bool clearDesired)
{
  Serial.println("SEND STATE");

  StaticJsonBuffer<600> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& state = root.createNestedObject("state");
  JsonObject& reported = state.createNestedObject("reported");
  reported["totalValveOpenMS"] = totalValveOpenMS;
  reported["bat"] = getBatteryLevel();

  if (clearDesired){
    state["desired"] = RawJson("null");
  }

  String jsonString;
  root.printTo(jsonString);
  ActionError actionError;
  Serial.println("Updating shadow");
  // TODO: Optimistic locking.
  iotClient.update_shadow(jsonString.c_str(), actionError);
  Serial.println("Shadow updated");
}

int getBatteryLevel()
{
  return ESP.getVcc()*1.024;
}

