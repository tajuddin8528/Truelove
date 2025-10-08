#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HX711.h"
#include <Servo.h>

// -------- Pin Configuration --------
#define LOADCELL_DOUT_PIN  2
#define LOADCELL_SCK_PIN   3
#define TOUCH_PIN          4
#define SERVO_PIN          5

// -------- Display Config --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- HX711 Config --------
HX711 scale;
float calibration_factor; // Auto calculated

// -------- Servo Config --------
Servo dispenserServo;

// -------- Touch Control --------
int touchCount = 0;
bool lastTouchState = false;
int dispenseWeights[] = {125, 250, 500, 1000}; // grams

// -------- Auto Calibration --------
float knownWeight = 500.0; // अपने reference weight डालें
bool calibrated = false;

void setup() {
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare();

  dispenserServo.attach(SERVO_PIN);
  dispenserServo.write(0); // Valve closed

  pinMode(TOUCH_PIN, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("OLED not found!");
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Calibrating...");
  display.display();
}

void loop() {
  if(!calibrated){
    float rawReading = scale.get_units(10);
    calibration_factor = rawReading / knownWeight;
    Serial.print("Calibration factor: ");
    Serial.println(calibration_factor, 4);
    calibrated = true;
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Calibration Done!");
    display.display();
    delay(2000);
  } else {
    bool currentTouch = digitalRead(TOUCH_PIN);
    if(currentTouch && !lastTouchState){
      touchCount++;
      if(touchCount > 4) touchCount = 1;
      int targetWeight = dispenseWeights[touchCount - 1];
      Serial.print("Touch ");
      Serial.print(touchCount);
      Serial.print(" - Dispensing ");
      Serial.print(targetWeight);
      Serial.println(" g");

      dispenseOil(targetWeight); // Single-step dispense
    }
    lastTouchState = currentTouch;

    // Display live weight
    float weight = scale.get_units(10) / calibration_factor;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print(weight,1);
    display.println(" g");

    display.setTextSize(1);
    display.setCursor(0,40);
    display.println("Touch to dispense:");
    display.print(dispenseWeights[0]);
    display.print(" / ");
    display.print(dispenseWeights[1]);
    display.print(" / ");
    display.print(dispenseWeights[2]);
    display.print(" / ");
    display.print(dispenseWeights[3]);
    display.println(" g");
    display.display();

    delay(100);
  }
}

void dispenseOil(int targetGrams){
  float startWeight = scale.get_units(10) / calibration_factor;
  float targetWeight = startWeight + targetGrams;

  dispenserServo.write(90); // Open tap
  delay(100);

  // Wait until target weight is reached
  while((scale.get_units(5) / calibration_factor) < targetWeight){
    // Nothing, just wait
    delay(10);
  }

  dispenserServo.write(0); // Close tap
  delay(500);
  Serial.print("Dispensed ");
  Serial.print(targetGrams);
  Serial.println(" g");
}
