#include <Servo.h>

Servo dryServo;  // Servo 1 = DRY flap
Servo wetServo;  // Servo 2 = WET flap

const int moistureSensorPin = A0;
const int irSensorPin = 13;

const int dryServoPin = 9;
const int wetServoPin = 10;

int dryCal = 1000;  // Dry calibration value
int wetCal = 500;   // Wet calibration value

unsigned long moveDuration = 3000;
unsigned long lastActionTime = 0;

bool isDryServoMoving = false;
bool isWetServoMoving = false;
bool systemActive = false;

unsigned long irTriggerTime = 0;
bool waitingForMoistureCheck = false;
const unsigned long delayAfterIR = 7000;  // Wait 7 sec after motion

// Converts raw value into percent
int getMoisturePercent(int value) {
  return constrain(map(value, wetCal, dryCal, 100, 0), 0, 100);
}

void openDryFlap() {
  dryServo.attach(dryServoPin);
  dryServo.write(40);  // OPEN
  wetServo.attach(wetServoPin);
  wetServo.write(180); // CLOSE
  lastActionTime = millis();
  isDryServoMoving = true;
  Serial.println("→ DRY flap OPENED");
}

void closeDryFlap() {
  dryServo.write(180);  // PRONE
  delay(500);
  dryServo.detach();
  isDryServoMoving = false;
  Serial.println("→ DRY flap CLOSED");
}

void openWetFlap() {
  wetServo.attach(wetServoPin);
  wetServo.write(40);  // OPEN
  dryServo.attach(dryServoPin);
  dryServo.write(180); // CLOSE
  lastActionTime = millis();
  isWetServoMoving = true;
  Serial.println("→ WET flap OPENED");
}

void closeWetFlap() {
  wetServo.write(180);  // PRONE
  delay(500);
  wetServo.detach();
  isWetServoMoving = false;
  Serial.println("→ WET flap CLOSED");
}

void adminConsole() {
  Serial.println("=== ADMIN MODE ===");
  Serial.println("Commands: CAL / TEST / INFO / START / VERSION / EXIT");

  while (true) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      input.toUpperCase();

      if (input == "CAL") {
        Serial.print("Place WET material... ");
        delay(2000);
        wetCal = analogRead(moistureSensorPin);
        Serial.println("WetCal = " + String(wetCal));

        Serial.print("Place DRY material... ");
        delay(2000);
        dryCal = analogRead(moistureSensorPin);
        Serial.println("DryCal = " + String(dryCal));
      } else if (input == "TEST") {
        Serial.println("Testing DRY flap...");
        dryServo.attach(dryServoPin);
        dryServo.write(40);
        delay(1000);
        dryServo.write(180);
        delay(1000);
        dryServo.detach();

        Serial.println("Testing WET flap...");
        wetServo.attach(wetServoPin);
        wetServo.write(40);
        delay(1000);
        wetServo.write(180);
        delay(1000);
        wetServo.detach();

        Serial.println("Test complete.");
      } else if (input == "INFO") {
        Serial.println("DryCal = " + String(dryCal));
        Serial.println("WetCal = " + String(wetCal));
      } else if (input == "START") {
        systemActive = true;
        Serial.println("→ SYSTEM STARTED");
        break;
      } else if (input == "VERSION") {
        Serial.println("DUOBIN V2.5 | Startup Command Added");
      } else if (input == "EXIT") {
        Serial.println("Exiting Admin Mode.");
        break;
      } else {
        Serial.println("Invalid command.");
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(irSensorPin, INPUT);
  pinMode(moistureSensorPin, INPUT);

  dryServo.attach(dryServoPin);
  dryServo.write(180);  // Default position
  dryServo.detach();

  wetServo.attach(wetServoPin);
  wetServo.write(180);  // Default position
  wetServo.detach();

  Serial.println("DUOBIN SYSTEM READY");
  Serial.println("Type LOGIN to enter Admin Console.");
}

void loop() {
  if (!systemActive) {
    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      if (cmd == "LOGIN") adminConsole();
    }
    return;
  }

  int irState = digitalRead(irSensorPin);
  Serial.print("IR State: ");
  Serial.println(irState);  // Debug check

  if (irState == LOW && !waitingForMoistureCheck && !isDryServoMoving && !isWetServoMoving) {
    irTriggerTime = millis();
    waitingForMoistureCheck = true;
    Serial.println("→ IR TRIGGERED! Waiting 7 seconds...");
  }

  if (waitingForMoistureCheck && millis() - irTriggerTime >= delayAfterIR) {
    int moistureVal = analogRead(moistureSensorPin);
    int moisturePercent = getMoisturePercent(moistureVal);

    Serial.print("Moisture: ");
    Serial.print(moisturePercent);
    Serial.println("%");

    if (moisturePercent > 60) {
      openDryFlap();
    } else if (moisturePercent < 40) {
      openWetFlap();
    } else {
      Serial.println("→ Moisture unclear, no flap moved.");
    }
    waitingForMoistureCheck = false;
  }

  if (isDryServoMoving && millis() - lastActionTime >= moveDuration) {
    closeDryFlap();
  }

  if (isWetServoMoving && millis() - lastActionTime >= moveDuration) {
    closeWetFlap();
  }

  delay(100);
}
