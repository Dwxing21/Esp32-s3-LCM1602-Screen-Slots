#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD address (0x27 or 0x3F depending on your module)
// 16x2 display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Button pins
#define SPIN_BUTTON 0      // BOOT button (GPIO 0)
#define BET_INCREASE 14
#define BET_DECREASE 13

// I2C pins
#define SDA_PIN 8
#define SCL_PIN 9

// Game variables
int balance = 100;  // Starting balance
int bet = 10;       // Current bet
int winnings = 0;
bool spinning = false;

// Slot symbols (numbers 0-9 representing different icons)
const int NUM_SYMBOLS = 10;
const char symbols[NUM_SYMBOLS] = {'7', '$', '*', 'O', 'X', 'C', 'P', 'D', 'B', 'M'};

// Debounce variables
unsigned long lastSpinTime = 0;
unsigned long lastBetTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// Custom characters for better display
byte seven[8] = {0x00, 0x1F, 0x10, 0x08, 0x04, 0x04, 0x02, 0x00};
byte cherry[8] = {0x00, 0x03, 0x07, 0x1E, 0x1C, 0x08, 0x00, 0x00};

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize I2C on GPIO 8 (SDA) and GPIO 9 (SCL)
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Create custom characters
  lcd.createChar(0, seven);
  lcd.createChar(1, cherry);
  
  // Setup button pins
  pinMode(SPIN_BUTTON, INPUT);      // BOOT button (active LOW)
  pinMode(BET_INCREASE, INPUT_PULLUP);
  pinMode(BET_DECREASE, INPUT_PULLUP);
  
  // Display welcome screen
  displayWelcome();
  delay(2000);
  
  displayMainScreen();
}

void loop() {
  // Check button inputs
  handleButtons();
  
  // Update display periodically
  delay(50);
}

void handleButtons() {
  unsigned long currentTime = millis();
  
  // Spin button (BOOT button - GPIO 0, active LOW)
  if (digitalRead(SPIN_BUTTON) == LOW && !spinning && 
      (currentTime - lastSpinTime) > DEBOUNCE_DELAY) {
    lastSpinTime = currentTime;
    if (balance >= bet) {
      spinReels();
    } else {
      displayInsufficientFunds();
      delay(1500);
      displayMainScreen();
    }
  }
  
  // Bet increase button
  if (digitalRead(BET_INCREASE) == LOW && !spinning && 
      (currentTime - lastBetTime) > DEBOUNCE_DELAY) {
    lastBetTime = currentTime;
    if (bet < balance) {
      bet += 5;
      if (bet > balance) bet = balance;
      displayMainScreen();
    }
  }
  
  // Bet decrease button
  if (digitalRead(BET_DECREASE) == LOW && !spinning && 
      (currentTime - lastBetTime) > DEBOUNCE_DELAY) {
    lastBetTime = currentTime;
    if (bet > 5) {
      bet -= 5;
    } else {
      bet = 5;
    }
    displayMainScreen();
  }
}

void spinReels() {
  spinning = true;
  balance -= bet;
  
  // Spinning animation
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SPINNING...");
  
  // Simulate spinning
  for (int i = 0; i < 15; i++) {
    lcd.setCursor(0, 1);
    lcd.print("| ");
    lcd.print(symbols[random(NUM_SYMBOLS)]);
    lcd.print(" | ");
    lcd.print(symbols[random(NUM_SYMBOLS)]);
    lcd.print(" | ");
    lcd.print(symbols[random(NUM_SYMBOLS)]);
    lcd.print(" |");
    
    delay(100);
  }
  
  // Generate final results
  int reel1 = random(NUM_SYMBOLS);
  int reel2 = random(NUM_SYMBOLS);
  int reel3 = random(NUM_SYMBOLS);
  
  // Display final result
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("| ");
  lcd.print(symbols[reel1]);
  lcd.print(" | ");
  lcd.print(symbols[reel2]);
  lcd.print(" | ");
  lcd.print(symbols[reel3]);
  lcd.print(" |");
  
  delay(500);
  
  // Check for wins
  if (reel1 == reel2 && reel2 == reel3) {
    // All three match - jackpot!
    winnings = bet * 50;
    balance += winnings;
    displayWin(50);
  } else if (reel1 == reel2 || reel2 == reel3 || reel1 == reel3) {
    // Two match - small win
    winnings = bet * 5;
    balance += winnings;
    displayWin(5);
  } else {
    // No match - loss
    displayLoss();
  }
  
  spinning = false;
  delay(2000);
  displayMainScreen();
}

void displayWelcome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  SLOT MACHINE");
  lcd.setCursor(0, 1);
  lcd.print("   ESP32-S3");
}

void displayMainScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BAL:");
  lcd.print(balance);
  lcd.print(" BET:");
  lcd.print(bet);
  
  lcd.setCursor(0, 1);
  lcd.print("BOOT=SPIN");
}

void displayWin(int multiplier) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("YOU WIN!");
  lcd.setCursor(0, 1);
  lcd.print("X");
  lcd.print(multiplier);
  lcd.print(" = ");
  lcd.print(winnings);
  delay(1500);
}

void displayLoss() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NO MATCH!");
  lcd.setCursor(0, 1);
  lcd.print("Try again...");
  delay(1500);
}

void displayInsufficientFunds() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INSUFFICIENT");
  lcd.setCursor(0, 1);
  lcd.print("BALANCE!");
}
