#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <M5Core2.h>

Adafruit_VL53L0X tof;
VL53L0X_RangingMeasurementData_t measure;

bool isTimerActive = false;
unsigned long timerStartTime = 0;
const unsigned long timerDuration = 30000; // 30 seconds
bool hasTimerExpired = false;
bool isParkFree = true;

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
}

void loop() {
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
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.println("Zeit ueberschritten");
        isParkFree = false;
        hasTimerExpired = true;
      } else {
        unsigned long remainingTime = timerDuration - elapsedTime;
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.println("Besetzt ");
        M5.Lcd.println("Uebrige Zeit: ");
        M5.Lcd.println(remainingTime / 1000);
        isParkFree = false;
      }
    } else {
      isTimerActive = false;
      hasTimerExpired = false;
      isParkFree = true;
      M5.Lcd.setTextColor(TFT_GREEN);
      M5.Lcd.println("Parkplatz frei");
      M5.Lcd.println("Maximale ");
      M5.Lcd.println("Parkzeit: ");
      M5.Lcd.println("30 min");
  }

  delay(1000);
  }
}
