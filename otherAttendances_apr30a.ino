#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define pins
#define TRIG_PIN 7
#define ECHO_PIN 6
#define SERVO_PIN 9
#define LED_PIN 8
#define BUZZER_PIN 10

Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD I2C address

String command = "";
bool isUnlocked = false;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(0); // Initial position

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ProxiGuard Ready");
}

void loop() {
  float distance = getDistanceCM();

  // Display distance
  lcd.setCursor(0, 1);
  lcd.print("Distance: ");
  lcd.print(distance, 1);
  lcd.print("cm ");

  // Receive Bluetooth command
  while (Serial.available()) {
    char c = Serial.read();
    command += c;
    if (c == '\n') {
      command.trim();

      if (command.equalsIgnoreCase("Unlock") && distance <= 70) {
        isUnlocked = true;
        myServo.write(80);  // Open gate
        lcd.setCursor(0, 0);
        lcd.print("Gate Unlocked   ");
      }

      command = ""; // Reset
    }
  }

  // Alert when car is near
  if (distance <= 70) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }

  // Auto-close when car drives away
  if (isUnlocked && distance > 80) {
    myServo.write(0);  // Close gate
    lcd.setCursor(0, 0);
    lcd.print("Gate Locked     ");
    isUnlocked = false;
  }

  delay(300);
}

float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2;
}
