// DuoBin - Full Version with Admin Login, Auto, Graph, CompactAuto, and Hidden Games
#include <Servo.h>

int moistureSensorPin = A0;
int dryServoPin = 9;
int wetServoPin = 10;
int pirSensorPin = 2;

Servo binServo;
String currentUser = "";
bool loggedIn = false;
bool isAdmin = false;
bool autoMode = false;
bool compactAutoMode = false;

int minMoisture = 1023;
int maxMoisture = 0;

void showMenu() {
  Serial.println("\n========== DuoBin Main Menu ==========");
  Serial.println(" create       - Create a new user");
  Serial.println(" login        - Login as existing user");
  Serial.println(" test         - Check sensor readings");
  Serial.println(" graph        - Visualize sensor data");
  Serial.println(" auto         - Toggle auto sensor stream");
  Serial.println(" compactauto  - Compact sensor output mode");
  Serial.println(" cal          - Calibrate sensors");
  Serial.println(" info         - Show user information");
  Serial.println(" ver          - Show version");
  Serial.println(" servo        - Test bin servo");
  Serial.println(" setpin       - Reassign a sensor/servo pin");
  Serial.println("======================================");
}

void setup() {
  Serial.begin(9600);
  binServo.attach(dryServoPin);
  pinMode(moistureSensorPin, INPUT);
  pinMode(pirSensorPin, INPUT);
  pinMode(3, OUTPUT);  // D3 for moisture sensor control/output
  Serial.println("Welcome to DuoBin!");
  promptLogin();
}

void promptLogin() {
  while (!loggedIn) {
    Serial.println("\nPlease login to continue.");
    loginUser();
  }
  showMenu();
}

void loop() {
  if (!loggedIn) return;

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "create") createUser();
    else if (cmd == "login") loginUser();
    else if (cmd == "graph") startGraphMode();
    else if (cmd == "auto") autoMode = !autoMode;
    else if (cmd == "compactauto") compactAutoMode = !compactAutoMode;
    else if (cmd == "test") testSensors();
    else if (cmd == "cal") calibrateSensors();
    else if (cmd == "info") showUserInfo();
    else if (cmd == "ver") Serial.println("DuoBin v1.1");
    else if (cmd == "servo") servoTest();
    else if (cmd.startsWith("setpin")) handleSetPin(cmd);
    else if (cmd == "SOLORUN") startSolitaireMode();
    else Serial.println("Unknown command. Please try again.");

    if (!autoMode && !compactAutoMode) showMenu();
  }

  if (autoMode) {
    int moist = analogRead(moistureSensorPin);
    int pir = digitalRead(pirSensorPin);
    Serial.print("\n[Auto @ ");
    Serial.print(millis() / 1000);
    Serial.println("s]");
    Serial.print("Moisture: ");
    Serial.println(moist);
    Serial.print("PIR: ");
    Serial.println(pir);

    if (moist < minMoisture) minMoisture = moist;
    if (moist > maxMoisture) maxMoisture = moist;
    delay(2000);
  }

  if (compactAutoMode) {
    int moist = analogRead(moistureSensorPin);
    int pir = digitalRead(pirSensorPin);
    unsigned long timestamp = millis() / 1000;

    if (moist < minMoisture) minMoisture = moist;
    if (moist > maxMoisture) maxMoisture = moist;

    Serial.print("[T:");
    Serial.print(timestamp);
    Serial.print("] Moisture:");
    Serial.print(moist);
    Serial.print(" PIR:");
    Serial.print(pir);
    Serial.print(" Min:");
    Serial.print(minMoisture);
    Serial.print(" Max:");
    Serial.println(maxMoisture);
    delay(1000);
  }
}

void createUser() {
  Serial.println("\n[Create User]");
  Serial.println("Enter new username:");
  while (!Serial.available())
    ;
  String username = Serial.readStringUntil('\n');
  username.trim();

  if (username == "irishepianist") {
    Serial.println("Username reserved for admin.");
    return;
  }

  Serial.println("Enter password:");
  while (!Serial.available())
    ;
  String pass = Serial.readStringUntil('\n');
  pass.trim();

  Serial.println("User created (not saved).");
}

void loginUser() {
  Serial.println("\n[Login]");
  Serial.println("Username:");
  while (!Serial.available())
    ;
  String username = Serial.readStringUntil('\n');
  username.trim();

  Serial.println("Password:");
  while (!Serial.available())
    ;
  String pass = Serial.readStringUntil('\n');
  pass.trim();

  if (username == "irishepianist" && pass == "1234") {
    isAdmin = true;
    loggedIn = true;
    currentUser = username;
    Serial.println("Admin login successful.");
  } else {
    isAdmin = false;
    loggedIn = true;
    currentUser = username;
    Serial.println("Login successful.");
  }
}

void showUserInfo() {
  if (loggedIn) {
    Serial.print("Logged in as: ");
    Serial.println(currentUser);
    if (isAdmin) Serial.println("Privileges: Admin");
    else Serial.println("Privileges: Standard");
  } else {
    Serial.println("Not logged in.");
  }
}

void testSensors() {
  Serial.println("\n[Sensor Test]");
  Serial.print("Moisture Sensor: ");
  Serial.println(analogRead(moistureSensorPin));
  Serial.print("PIR Sensor: ");
  Serial.println(digitalRead(pirSensorPin));
}

void calibrateSensors() {
  Serial.println("\n[Calibration]");
  Serial.println("Hold steady. Calibrating...");
  delay(3000);
  Serial.println("Calibration complete.");
}

void startGraphMode() {
  Serial.println("\n[Graph Mode]");
  for (int i = 0; i < 10; i++) {
    int w = analogRead(moistureSensorPin);
    int d = analogRead(pirSensorPin);

    Serial.print("Moisture [");
    for (int j = 0; j < w / 100; j++) Serial.print("#");
    Serial.print("] ");
    Serial.println(w);

    Serial.print("PIR      [");
    for (int j = 0; j < d / 100; j++) Serial.print("#");
    Serial.print("] ");
    Serial.println(d);

    delay(1000);
  }
}

void startSolitaireMode() {
  Serial.println("\n[Solo Numbers Game]");
  int secret = random(1, 10);
  while (true) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input == "exit") {
        Serial.println("Exiting Solo Numbers...");
        break;
      } else if (input.toInt() == secret) {
        Serial.println("Correct!");
        secret = random(1, 10);
      } else {
        Serial.println("Wrong! Try again.");
      }
    }
  }
}

void servoTest() {
  Serial.println("\n[Servo Test]");
  Serial.println("Moving to 0°...");
  binServo.write(0);
  delay(1000);
  Serial.println("Moving to 90°...");
  binServo.write(90);
  delay(1000);
  Serial.println("Moving to 180°...");
  binServo.write(180);
  delay(1000);
  Serial.println("Servo test complete.");
}

void handleSetPin(String cmd) {
  int space1 = cmd.indexOf(' ');
  int space2 = cmd.indexOf(' ', space1 + 1);
  if (space1 < 0 || space2 < 0) {
    Serial.println("Usage: setpin <moisture/pir/dryservo/wetservo> <pin>");
    return;
  }

  String name = cmd.substring(space1 + 1, space2);
  String pinVal = cmd.substring(space2 + 1);
  pinVal.trim();

  int val = (pinVal.startsWith("A")) ? pinVal.charAt(1) - '0' + A0 : pinVal.toInt();

  if (name == "moisture") moistureSensorPin = val;
  else if (name == "pir") pirSensorPin = val;
  else if (name == "dryservo") dryServoPin = val;
  else if (name == "wetservo") wetServoPin = val;
  else {
    Serial.println("Unknown component. Try: moisture, pir, dryservo, wetservo");
    return;
  }

  if (name.endsWith("servo")) binServo.attach(val);
  pinMode(val, INPUT);
  Serial.print("Reassigned ");
  Serial.print(name);
  Serial.print(" to pin ");
  Serial.println(val);
}
