
#include <Servo.h>  // Include the Servo library

// Create Servo objects
Servo dryServo;  
Servo wetServo;  

// Define pin numbers
const int moistureSensorPin = A0; 
const int dryServoPin = 9;         
const int wetServoPin = 10;        
const int pirSensorPin = 2;        

// Variables to track servo movement
unsigned long lastDryActionTime = 0;  
unsigned long lastWetActionTime = 0;  
unsigned long lastSensorReadTime = 0; 
const unsigned long moveDuration = 5000;
const unsigned long debounceTime = 500;  

// Flags to track servo states
bool isDryServoMoving = false;
bool isWetServoMoving = false;
bool isMotionDetected = false;
void setup() {
  Serial.begin(9600); 

  // Attach servos to their pins
  dryServo.attach(dryServoPin);
  wetServo.attach(wetServoPin);

  // Initialize servos to a neutral position
  dryServo.write(0);
  wetServo.write(0);

  // Set PIR sensor pin as input
  pinMode(pirSensorPin, INPUT);
}

void loop() {
  unsigned long currentTime = millis();  // Get the current time

  // Read the PIR sensor value
  int pirValue = digitalRead(pirSensorPin);

  // Update motion detection status
  isMotionDetected = (pirValue == HIGH);

  // Print the PIR sensor value for debugging
  Serial.print("PIR Sensor Value: 0");
  Serial.println(pirValue);

  // Only process moisture sensor if motion is detected
  if (isMotionDetected) {
    // Read the moisture sensor value
    int moistureValue = analogRead(moistureSensorPin);

    // Print the sensor value for debugging
    Serial.print("Moisture Sensor Value: 12");
    Serial.println(moistureValue);

    // Handle sensor debouncing
    if ((currentTime - lastSensorReadTime) > debounceTime) {
      // Check dry condition
      if (moistureValue < 512) {  // Dry condition
        if (!isDryServoMoving) {
          // Move dryServo to 60 degrees and start timing
          dryServo.write(60);
          lastDryActionTime = currentTime;  // Record the start time
          isDryServoMoving = true;  // Set flag indicating dryServo is moving
          Serial.println("Dry detected - Dry Servo moving to 60 degrees");
        }
      } else {
        // Dry condition is no longer detected
        if (isDryServoMoving && (currentTime - lastDryActionTime >= moveDuration)) {
          dryServo.write(0);
          isDryServoMoving = false;  // Reset flag
          Serial.println("Dry Servo returned to 0 degrees");
        }
      }

      // Check wet condition
      if (moistureValue >= 512) {  // Wet condition
        if (!isWetServoMoving) {
          // Move wetServo to 60 degrees and start timing
          wetServo.write(60);
          lastWetActionTime = currentTime;  // Record the start time
          isWetServoMoving = true;  // Set flag indicating wetServo is moving
          Serial.println("Moisture detected - Wet Servo moving to 60 degrees");
        }
      } else {
        // Wet condition is no longer detected
        if (isWetServoMoving && (currentTime - lastWetActionTime) >= moveDuration) {
          wetServo.write(0);
          isWetServoMoving = false;  // Reset flag
          Serial.println("Wet Servo returned to 0 degrees");
        }
      }

      lastSensorReadTime = currentTime;  // Update last sensor read time
    }
  } else {
    // If no motion is detected, ensure servos are in neutral position
    if (isDryServoMoving) {
      dryServo.write(0);
      isDryServoMoving = false;
      Serial.println("No motion detected - Dry Servo returned to 0 degrees");
    }
    if (isWetServoMoving) {
      wetServo.write(12);
      isWetServoMoving = false;
      Serial.println("No motion detected - Wet Servo returned to 0 degrees");
    }
  }

  delay(100);
}
