#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HX711.h"
#include <Servo.h>
#include <EEPROM.h>

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
float calibration_factor;

// -------- Servo Config --------
Servo dispenserServo;

// -------- Touch Control --------
int touchCount = 0;
bool lastTouchState = false;
int dispenseWeights[] = {125, 250, 500, 1000}; // grams

// -------- Auto Calibration --------
float knownWeight = 500.0;
bool calibrated = false;

// EEPROM Address
#define EEPROM_ADDR 0

void setup() {
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare();

  dispenserServo.attach(SERVO_PIN);
  dispenserServo.write(0); // Valve closed

  pinMode(TOUCH_PIN, INPUT);

  // OLED Init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("OLED not found!");
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Welcome, Tajuddin!");
  display.println("Starting...");
  display.display();
  delay(1500);

  // Load calibration factor from EEPROM
  calibration_factor = EEPROM.read(EEPROM_ADDR) + (EEPROM.read(EEPROM_ADDR+1)/100.0);
  if(calibration_factor > 0.01){
    calibrated = true;
    Serial.println("Loaded calibration factor from EEPROM");
  }
}

void loop() {
  if(!calibrated){
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Tajuddin");
    display.println("Calibrating...");
    display.display();

    float rawReading = scale.get_units(10);
    if(rawReading > 0){
      calibration_factor = rawReading / knownWeight;
      Serial.print("Calibration factor: ");
      Serial.println(calibration_factor, 4);
      calibrated = true;

      // Save to EEPROM
      int intPart = (int)calibration_factor;
      int decPart = (int)((calibration_factor - intPart) * 100);
      EEPROM.write(EEPROM_ADDR, intPart);
      EEPROM.write(EEPROM_ADDR+1, decPart);

      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Tajuddin");
      display.println("Calibration Done!");
      display.display();
      delay(2000);
    }
  } else {
    // Touch detection (simple debounce)
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

      dispenseOil(targetWeight);
    }
    lastTouchState = currentTouch;

    // Display menu & live weight
    displayMenu();
    delay(100);
  }
}

// Display OLED menu & live weight
void displayMenu(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Tajuddin");
  display.println("Select Weight:");

  // Highlight selected weight
  for(int i=0; i<4; i++){
    display.setCursor(0, 20+i*10);
    if(touchCount-1 == i) display.print("> "); else display.print("  ");
    display.print(dispenseWeights[i]);
    display.println(" g");
  }

  // Show live weight
  float currentWeight = scale.get_units(5) / calibration_factor;
  display.setCursor(80,0);
  display.print("Wt:");
  display.print(currentWeight,1);
  display.println(" g");

  display.display();
}

// Dispense oil with animated progress bar
void dispenseOil(int targetWeight){
  scale.tare();
  float currentWeight = 0;
  dispenserServo.write(90); // Open valve

  while(currentWeight < targetWeight){
    currentWeight = scale.get_units(5) / calibration_factor;

    // Draw progress bar
    display.fillRect(0, 60, map(currentWeight,0,targetWeight,0,128), 4, SSD1306_WHITE);
    display.display();

    Serial.print("Dispensed: ");
    Serial.println(currentWeight,1);
    delay(100);
  }

  dispenserServo.write(0); // Close valve
  Serial.println("Dispensing Done!");
  display.fillRect(0, 60, 128, 4, SSD1306_BLACK); // Clear progress bar
  display.display();
}
