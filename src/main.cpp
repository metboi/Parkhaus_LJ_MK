#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <M5Core2.h>
#include <PubSubClient.h>
#include <WiFi.h>

void mqtt_callback(char* topic, byte* payload, unsigned int length);

Adafruit_VL53L0X tof;
VL53L0X_RangingMeasurementData_t measure;

bool isTimerActive = false;
unsigned long timerStartTime = 0;
const unsigned long timerDuration = 30000; // 30 seconds
bool hasTimerExpired = false;

bool isParkFree = true;
String isReceivedFree1;
String isReceivedFree2;
String combined;

// MQTT Broker configuration
const char* mqttServer = "cloud.tbz.ch";
const int mqttPort = 1883;
const char* mqttClientId = "";
const char* mqttTopic1 = "Parkhaus/ParkPlatz-unten";
const char* mqttTopic2 = "Parkhaus/ParkPlatz-oben";

// WiFi credentials
const char * ssid = "LERNKUBE";
const char * password = "l3rnk4b3";

String chatIdStr = "5187049756";

char remainingTimeMessage[50];


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, mqttTopic1) == 0) {
    char receivedData[length + 1];
    memcpy(receivedData, payload, length);
    receivedData[length] = '\0';

    isReceivedFree1 = receivedData;
  
  }
   if (strcmp(topic, mqttTopic2) == 0) {
    char receivedData[length + 1];
    memcpy(receivedData, payload, length);
    receivedData[length] = '\0';

    isReceivedFree2 = receivedData;
  
  }
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(mqttClientId)) {
      Serial.println("connected");
      mqttClient.subscribe(mqttTopic1);
      mqttClient.subscribe(mqttTopic2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  M5.begin();
  M5.Lcd.begin();
  M5.Lcd.setTextFont(4);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE);
  Wire.begin();

  if (!tof.begin()) {
    M5.Lcd.println("Failed to initialize VL53L0X sensor!");
    while (1);
  }

  setupWiFi();
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  tof.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    uint16_t distance = measure.RangeMilliMeter;

    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0);
    if (distance < 100) {
      if (!isTimerActive) {
        timerStartTime = millis();
        isTimerActive = true;
      }

      unsigned long currentTime = millis();
      unsigned long elapsedTime = currentTime - timerStartTime;
      if (elapsedTime >= timerDuration) {
        isTimerActive = false;
        isParkFree = false;
        hasTimerExpired = true;
      } else {
        unsigned long remainingTime = timerDuration - elapsedTime;

       // mqttClient.publish("Parkhaus/ParkPlatz-unten/Ubere Zeit", rmT );
        isParkFree = false;
      }
    } else {
      isTimerActive = false;
      hasTimerExpired = false;
      isParkFree = true;
      
  }
  if (isParkFree){
    mqttClient.publish(mqttTopic2, "Frei");
  }
  else{
    mqttClient.publish(mqttTopic2, "Besetzt");
  }

  if (isReceivedFree1 == "Frei" && isReceivedFree2 == "Frei") {
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.println("<-- 1");
  
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("---------------");

    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.println("1 -->");
  } else if (isReceivedFree1 == "Frei" && isReceivedFree2 == "Besetzt") {
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.println("<-- 0");
  
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("---------------");

    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.println("1 -->");
  } else if (isReceivedFree1 == "Besetzt" && isReceivedFree2 == "Frei") {
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.println("<-- 1");
  
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("---------------");

    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.println("0 -->");
  } else if (isReceivedFree1 == "Besetzt" && isReceivedFree2 == "Besetzt") {
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.println("<-- 0");
  
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("---------------");

    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.println("0 -->");
  }


  callback;
  delay(200);
  }
}
