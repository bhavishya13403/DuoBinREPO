#include <Servo.h>

Servo dryServo;  
Servo wetServo;  

const int moistureSensorPin = A0; 
const int dryServoPin = 9;         
const int wetServoPin = 10;        
const int pirSensorPin = 2;        

unsigned long lastDryActionTime = 0;  
unsigned long lastWetActionTime = 0;  
unsigned long lastSensorReadTime = 0; 

const unsigned long moveDuration = 5000;
const unsigned long debounceTime = 500;  

bool isDryServoMoving = false;
bool isWetServoMoving = false;
bool isMotionDetected = false;

int dryCal = 0;
int wetCal = 1023;

void waitForUser(const char* message) {
  Serial.println(message);
  while (!Serial.available()) {
    delay(100);
  }
  while (Serial.available()) Serial.read(); // Clear buffer
}

void calibrateMoistureSensor() {
  waitForUser("Place sensor in DRY AIR and press any key...");
  dryCal = analogRead(moistureSensorPin);
  Serial.print("Dry calibrated value: ");
  Serial.println(dryCal);

  waitForUser("Now place sensor in WATER and press any key...");
  wetCal = analogRead(moistureSensorPin);
  Serial.print("Wet calibrated value: ");
  Serial.println(wetCal);

  if (wetCal <= dryCal) {
    Serial.println("Error: Wet value should be higher than dry value. Using default calibration.");
    dryCal = 0;
    wetCal = 1023;
  } else {
    Serial.println("Calibration complete.");
  }
}

int getMoisturePercent(int value) {
  return constrain(map(value, dryCal, wetCal, 0, 100), 0, 100);
}

void setup() {
  Serial.begin(9600); 
  Serial.println("System Booting...");

  dryServo.attach(dryServoPin);
  wetServo.attach(wetServoPin);
  dryServo.write(0);
  wetServo.write(0);
  pinMode(pirSensorPin, INPUT);

  calibrateMoistureSensor();

  Serial.println("System ready.");
}

void loop() {
  unsigned long currentTime = millis();  
  int pirValue = digitalRead(pirSensorPin);
  isMotionDetected = (pirValue == HIGH);

  Serial.print("PIR: ");
  Serial.print(pirValue);

  int rawMoisture = analogRead(moistureSensorPin);
  int percentMoisture = getMoisturePercent(rawMoisture);

  Serial.print(" | Raw Moisture: ");
  Serial.print(rawMoisture);
  Serial.print(" | % Moisture: ");
  Serial.println(percentMoisture);

  if (isMotionDetected) {
    if ((currentTime - lastSensorReadTime) > debounceTime) {
      if (percentMoisture < 50 && !isDryServoMoving) {
        dryServo.write(60);
        lastDryActionTime = currentTime;
        isDryServoMoving = true;
        Serial.println("Dry detected - Dry Servo ON");
      } else if (isDryServoMoving && (currentTime - lastDryActionTime >= moveDuration)) {
        dryServo.write(0);
        isDryServoMoving = false;
        Serial.println("Dry Servo OFF");
      }

      if (percentMoisture >= 50 && !isWetServoMoving) {
        wetServo.write(60);
        lastWetActionTime = currentTime;
        isWetServoMoving = true;
        Serial.println("Wet detected - Wet Servo ON");
      } else if (isWetServoMoving && (currentTime - lastWetActionTime) >= moveDuration) {
        wetServo.write(0);
        isWetServoMoving = false;
        Serial.println("Wet Servo OFF");
      }

      lastSensorReadTime = currentTime;
    }
  } else {
    if (isDryServoMoving) {
      dryServo.write(0);
      isDryServoMoving = false;
      Serial.println("No motion - Dry Servo OFF");
    }
    if (isWetServoMoving) {
      wetServo.write(0);
      isWetServoMoving = false;
      Serial.println("No motion - Wet Servo OFF");
    }
  }

  delay(100);
}

