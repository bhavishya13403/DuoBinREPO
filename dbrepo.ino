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
unsigned long lastInfoTime = 0;

const unsigned long moveDuration = 5000;
const unsigned long debounceTime = 500;
const unsigned long autoInfoInterval = 3000;

bool isDryServoMoving = false;
bool isWetServoMoving = false;
bool isMotionDetected = false;
bool adminMode = true;
bool systemReady = false;
bool showVersion = true;
bool autoInfo = false;
bool awaitingAdminLogin = false;
bool rudeDesiMode = false;

int dryCal = 0;
int wetCal = 1023;

#define VERSION "Duobin v1.0.0"
#define ADMIN_PASSWORD "1234"

int infoCount = 0;
long totalMoisture = 0;
long totalPIR = 0;

void waitForUser(const char *message) {
  Serial.println(message);
  while (!Serial.available()) {
    delay(100);
  }
  while (Serial.available()) Serial.read();
}

void calibrateMoistureSensor() {
  waitForUser("Place sensor in DRY AIR and press ENTER:");
  dryCal = analogRead(moistureSensorPin);
  Serial.print("Dry calibrated value: ");
  Serial.println(dryCal);

  waitForUser("Now place sensor in WATER and press ENTER:");
  wetCal = analogRead(moistureSensorPin);
  Serial.print("Wet calibrated value: ");
  Serial.println(wetCal);

  if (wetCal <= dryCal) {
    Serial.println("Error: Wet must be higher than Dry! Using defaults.");
    dryCal = 0;
    wetCal = 1023;
  } else {
    Serial.println("Calibration complete.");
  }
}

int getMoisturePercent(int value) {
  return constrain(map(value, dryCal, wetCal, 0, 100), 0, 100);
}

void displayHeader() {
  const char* lines[] = {
    "   ____        _     _       ",
    "  |  _ \\  ___ | |__ (_) ___  ",
    "  | | | |/ _ \\| '_ \\| |/ _ \\ ",
    "  | |_| | (_) | | | | | (_) |",
    "  |____/ \\___/|_| |_|_|\\___/ ",
    "         Waste Sorter v1.0.0"
  };
  for (int i = 0; i < 6; i++) {
    Serial.println(lines[i]);
    delay(150);
  }
  Serial.println("============================");
  if (showVersion) {
    Serial.print("         ");
    Serial.println(VERSION);
  }
  Serial.println("");
}

void printInfo(bool isAuto = false) {
  int raw = analogRead(moistureSensorPin);
  int percent = getMoisturePercent(raw);
  int pir = digitalRead(pirSensorPin);

  Serial.print(isAuto ? "[Auto] " : "[Info] ");
  Serial.print("Moisture (raw): ");
  Serial.print(raw);
  Serial.print(" | %: ");
  Serial.print(percent);
  Serial.print(" | PIR: ");
  Serial.println(pir);

  totalMoisture += percent;
  totalPIR += pir;
  infoCount++;
}

void printAverages() {
  if (infoCount == 0) {
    Serial.println("No data to average.");
    return;
  }
  Serial.println("--- Average Report ---");
  Serial.print("Avg Moisture %: ");
  Serial.println(totalMoisture / infoCount);
  Serial.print("Avg Motion % (0/1): ");
  Serial.println((float)totalPIR / infoCount);
  Serial.println("----------------------");
}

void runVanityDB() {
  const char* db[] = {
    "╔══════════════════════╗",
    "║      D  U  O  B  I   ║",
    "║     Smart Bin v1.0   ║",
    "╚══════════════════════╝"
  };
  for (int i = 0; i < 4; i++) {
    Serial.println(db[i]);
    delay(200);
  }
}

void adminConsole() {
  displayHeader();
  Serial.println("Admin Mode Active.");
  Serial.println("Type CAL / TEST / INFO / AVG / AUTO / VERSION / DESI / DB / EXIT");

  while (adminMode) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      input.toUpperCase();

      if (input == "CAL") {
        calibrateMoistureSensor();
      } else if (input == "TEST") {
        Serial.println("Testing Dry Servo...");
        dryServo.write(60);
        delay(1000);
        dryServo.write(0);

        Serial.println("Testing Wet Servo...");
        wetServo.write(60);
        delay(1000);
        wetServo.write(0);

        Serial.println("Moisture Value: " + String(analogRead(moistureSensorPin)));
        Serial.println("PIR State: " + String(digitalRead(pirSensorPin)));

        Serial.println("Servo Test Complete.");
      } else if (input == "INFO") {
        printInfo();
      } else if (input == "AVG") {
        printAverages();
      } else if (input == "VERSION") {
        showVersion = !showVersion;
        Serial.print("Version display now ");
        Serial.println(showVersion ? "ON" : "OFF");
      } else if (input == "AUTO") {
        autoInfo = !autoInfo;
        Serial.print("Auto info now ");
        Serial.println(autoInfo ? "ON" : "OFF");
      } else if (input == "DESI") {
        rudeDesiMode = !rudeDesiMode;
        Serial.println(rudeDesiMode ? "Desi mode ON: Ab samjha karo bina kaam ke kuch mat dabao." : "Desi mode OFF: Back to normal, you boring human.");
      } else if (input == "DB") {
        runVanityDB();
      } else if (input == "EXIT") {
        Serial.println(rudeDesiMode ? "Chal nikal admin mode se!" : "Exiting Admin Mode...");
        adminMode = false;
        awaitingAdminLogin = true;
        systemReady = true;
      } else {
        Serial.println(rudeDesiMode ? "Kya bakwaas command hai be?" : "Unknown Command");
      }

      while (Serial.available()) Serial.read();
    }

    if (autoInfo && millis() - lastInfoTime > autoInfoInterval) {
      printInfo(true);
      lastInfoTime = millis();
    }
  }
}

void checkAdminLogin() {
  if (awaitingAdminLogin && Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input == ADMIN_PASSWORD) {
      Serial.println("Admin login successful.");
      adminMode = true;
      awaitingAdminLogin = false;
      adminConsole();
    } else {
      Serial.println("Incorrect password.");
    }
    while (Serial.available()) Serial.read();
  }
}

void setup() {
  Serial.begin(9600);
  dryServo.attach(dryServoPin);
  wetServo.attach(wetServoPin);
  dryServo.write(0);
  wetServo.write(0);
  pinMode(pirSensorPin, INPUT);

  adminConsole();
  if (!systemReady) {
    calibrateMoistureSensor();
    systemReady = true;
  }
  Serial.println("System Running.");
}

void loop() {
  if (!systemReady) return;

  checkAdminLogin();

  unsigned long currentTime = millis();
  int pirValue = digitalRead(pirSensorPin);
  isMotionDetected = (pirValue == HIGH);

  int rawMoisture = analogRead(moistureSensorPin);
  int percentMoisture = getMoisturePercent(rawMoisture);

  if (isMotionDetected) {
    if ((currentTime - lastSensorReadTime) > debounceTime) {
      if (percentMoisture < 50 && !isDryServoMoving) {
        dryServo.write(60);
        lastDryActionTime = currentTime;
        isDryServoMoving = true;
      } else if (isDryServoMoving && (currentTime - lastDryActionTime >= moveDuration)) {
        dryServo.write(0);
        isDryServoMoving = false;
      }

      if (percentMoisture >= 50 && !isWetServoMoving) {
        wetServo.write(60);
        lastWetActionTime = currentTime;
        isWetServoMoving = true;
      } else if (isWetServoMoving && (currentTime - lastWetActionTime) >= moveDuration) {
        wetServo.write(0);
        isWetServoMoving = false;
      }
      lastSensorReadTime = currentTime;
    }
  } else {
    if (isDryServoMoving) {
      dryServo.write(0);
      isDryServoMoving = false;
    }
    if (isWetServoMoving) {
      wetServo.write(0);
      isWetServoMoving = false;
    }
  }

  if (autoInfo && (millis() - lastInfoTime > autoInfoInterval)) {
    printInfo(true);
    lastInfoTime = millis();
  }

  delay(100);
}
 
// HI

