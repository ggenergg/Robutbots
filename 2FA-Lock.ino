#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// ========== PIN ASSIGNMENTS ==========
// LCD I2C -   A4(SDA) and A5(SCL) + The usual VCC-5V GND
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LEDs
const int GREEN_LED = 11;
const int RED_LED = 12;

// Buzzer
const int BUZZER = 10;

// Reset Button
const int RESET_BTN = 13;

// Keypad Configuration
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Keypad pins: Row pins muna, tapos Column pins
// Basta nasa baba terminals at Magkakasunod
byte rowPins[ROWS] = {9, 8, 7, 6};    // Rows 1-4
byte colPins[COLS] = {5, 4, 3, 2};    // Columns 1-4


// Instance ni Keypad... i-mapa ang row pin 9 to 6 , col 5 to 2 , 4 na row, 4 na col. 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


// ========== 2FA VARIABLES ==========
String password = "1234";        // Master password
String oneTimeCode = "";         // Generated OTP
String userInput = "";           // User's input
int step = 0;                    // 0=password, 1=2FA code
unsigned long codeExpiry = 0;    // When OTP expires
int failedAttempts = 0;          // Track failures
bool systemLocked = false;       // Lockout status
unsigned long lockTime = 0;      // When locked

void setup() {
  Serial.begin(9600);
  
  // Initialize pins
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RESET_BTN, INPUT_PULLUP);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Random seed (or Hashing sa term ni CompTIA Security+ for OTP generation
  randomSeed(analogRead(A0));
  
  // Welcome message
  lcd.print("2FA System Ready");
  delay(1500);
  lcd.clear();
  lcd.print("Enter Password:");
  lcd.setCursor(0, 1);
  lcd.print(">");
  
  Serial.println("2FA System Active");
  Serial.println("Enter password using keypad");
}

void loop() {
  // Check for system lockout for 3 wrong entry, kontra BruteForce
  if(systemLocked) {
    if(millis() - lockTime > 30000) {  // 30 second lockout
      systemLocked = false;
      failedAttempts = 0;
      step = 0;
      userInput = "";
      lcd.clear();
      lcd.print("System Unlocked");
      delay(1500);
      lcd.clear();
      lcd.print("Enter Password:");
      lcd.setCursor(0, 1);
      lcd.print(">");
    }
    return;
  }
  
  // Check reset button kung pinindot
  if(digitalRead(RESET_BTN) == LOW) {
    resetSystem();
    delay(500);  // Debounce
  }
  
  char key = keypad.getKey();
  
  if(key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    
    if(step == 0) {
      // PASSWORD STEP
      if(key == '#') {  // Submit password
        checkPassword();
      }
      else if(key == '*') {  // Clear input
        userInput = "";
        updatePasswordDisplay();
      }
      else if(key >= '0' && key <= '9') {  // Only digits
        if(userInput.length() < 6) {  // Limit to 6 digits
          userInput += key;
          updatePasswordDisplay();
        }
      }
    }
    else if(step == 1) {
      // 2FA CODE STEP when Goods ang Password
      if(key == '#') {  // Submit 2FA code
        check2FACode();
      }
      else if(key == '*') {  // Clear input
        userInput = "";
        update2FADisplay();
      }
      else if(key >= '0' && key <= '9') {  // Only digits
        if(userInput.length() < 6) {
          userInput += key;
          update2FADisplay();
        }
      }
    }
  }
}

void checkPassword() {
  if(userInput == password) {
    // Password correct - generate OTP
    digitalWrite(GREEN_LED, HIGH);
    tone(BUZZER, 1000, 200);
    
    // Generate 6-digit OTP (valid for 30 seconds)
    oneTimeCode = String(random(100000, 999999));
    codeExpiry = millis() + 30000;  // 30 seconds expiry
    
    lcd.clear();
    lcd.print("2FA Code Sent!");
    lcd.setCursor(0, 1);
    lcd.print("Check your phone");
    
    Serial.print("2FA Code (simulated): ");
    Serial.println(oneTimeCode);
    
    delay(2000);
    lcd.clear();
    lcd.print("Enter 2FA Code:");
    lcd.setCursor(0, 1);
    lcd.print(">");
    
    step = 1;
    userInput = "";
    digitalWrite(GREEN_LED, LOW);
  }
  else {
    // Password wrong
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 200, 500);
    failedAttempts++;
    
    lcd.clear();
    lcd.print("WRONG PASSWORD!");
    lcd.setCursor(0, 1);
    lcd.print("Attempts: ");
    lcd.print(failedAttempts);
    lcd.print("/3");
    
    Serial.println("Access Denied - Wrong Password");
    delay(1500);
    
    if(failedAttempts >= 3) {
      systemLocked = true;
      lockTime = millis();
      lcd.clear();
      lcd.print("SYSTEM LOCKED!");
      lcd.setCursor(0, 1);
      lcd.print("Wait 30 sec");
      tone(BUZZER, 150, 1000);
    }
    else {
      userInput = "";
      lcd.clear();
      lcd.print("Enter Password:");
      lcd.setCursor(0, 1);
      lcd.print(">");
    }
    
    digitalWrite(RED_LED, LOW);
  }
}

void check2FACode() {
  // Check if code expired
  if(millis() > codeExpiry) {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 400, 500);
    
    lcd.clear();
    lcd.print("CODE EXPIRED!");
    lcd.setCursor(0, 1);
    lcd.print("Try again");
    
    Serial.println("2FA Code Expired!");
    delay(2000);
    
    // Reset to password step
    step = 0;
    userInput = "";
    lcd.clear();
    lcd.print("Enter Password:");
    lcd.setCursor(0, 1);
    lcd.print(">");
    digitalWrite(RED_LED, LOW);
  }
  else if(userInput == oneTimeCode) {
    // Access granted!
    digitalWrite(GREEN_LED, HIGH);
    tone(BUZZER, 1500, 300);
    
    lcd.clear();
    lcd.print("ACCESS GRANTED!");
    lcd.setCursor(0, 1);
    lcd.print("Welcome!");
    
    Serial.println("✓ 2FA Successful - Access Granted");
    
    delay(3000);
    
    // Reset for next user
    resetSystem();
  }
  else {
    // Invalid code
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER, 200, 800);
    
    lcd.clear();
    lcd.print("INVALID CODE!");
    lcd.setCursor(0, 1);
    lcd.print("Access Denied");
    
    Serial.println("✗ Invalid 2FA Code");
    delay(2000);
    
    // Reset to start
    step = 0;
    userInput = "";
    lcd.clear();
    lcd.print("Enter Password:");
    lcd.setCursor(0, 1);
    lcd.print(">");
    digitalWrite(RED_LED, LOW);
  }
}

void updatePasswordDisplay() {
  lcd.setCursor(0, 1);
  lcd.print(">");
  for(int i = 0; i < userInput.length(); i++) {
    lcd.print("*");  // Masks actual password
  }
  // Clear remaining spaces
  for(int i = userInput.length(); i < 15; i++) {
    lcd.print(" ");
  }
}

void update2FADisplay() {
  lcd.setCursor(0, 1);
  lcd.print(">");
  lcd.print(userInput);
  // Clear remaining spaces
  for(int i = userInput.length(); i < 15; i++) {
    lcd.print(" ");
  }
}

void resetSystem() {
  step = 0;
  userInput = "";
  failedAttempts = 0;
  systemLocked = false;
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  lcd.clear();
  lcd.print("System Reset");
  delay(1000);
  lcd.clear();
  lcd.print("Enter Password:");
  lcd.setCursor(0, 1);
  lcd.print(">");
  tone(BUZZER, 800, 200);
  Serial.println("System Reset by User");
}








/*

The 2FA (Two-Factor Authentication) example provided is actually serial monitor based, 
so it doesn't use physical pins for input/output. However, here's a revised version 
with designated pins for physical components:
2FA System with Designated Pins:
Components & Pin Connections:
text

Component          → Arduino Pin
------------------------------------
4x4 Keypad         → Pins 9,8,7,6 (Rows), 5,4,3,2 (Cols)
I2C LCD            → SDA = A4, SCL = A5, VCC=5V, GND=GND
Green LED (Access) → Pin 11
Red LED (Denied)   → Pin 12
Buzzer             → Pin 10
Button (Reset)     → Pin 13


*/

