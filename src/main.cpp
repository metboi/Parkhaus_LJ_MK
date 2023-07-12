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


// MQTT Broker configuration
const char* mqttServer = "cloud.tbz.ch";
const int mqttPort = 1883;
const char* mqttClientId = "";
const char* mqttTopic = "Parkhaus/ParkPlatz-unten";

// WiFi credentials
const char * ssid = "Wlan@Krasniqi";
const char * password = "Kras1818@Au8804";

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
  if (strcmp(topic, mqttTopic) == 0) {
    char receivedData[length + 1];
    memcpy(receivedData, payload, length);
    receivedData[length] = '\0';

    isReceivedFree1 = receivedData;
  
  }
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(mqttClientId)) {
      Serial.println("connected");
      mqttClient.subscribe(mqttTopic);
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
  M5.Lcd.setTextSize(1);
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
        isParkFree = false;
      }
    } else {
      isTimerActive = false;
      hasTimerExpired = false;
      isParkFree = true;
      
  }
  if (isParkFree){
    mqttClient.publish(mqttTopic, "Frei");
  }
  else{
    mqttClient.publish(mqttTopic, "Besetzt");
  }


  if (isReceivedFree1 == "Frei"){
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.println("Parkplatz unten frei");
    callback;
  }
  else if (isReceivedFree1 == "Besetzt"){
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.println("Parkplatz unten Besetzt");
    callback;
  }

  delay(200);
  }
}
