#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

// ========== WIFI CONFIGURATION ==========
const char* ssid = "YOUR_SSID";           // Change to your WiFi name
const char* password = "YOUR_PASSWORD";   // Change to your WiFi password

// ========== LCD SETUP ==========
// LCD address (0x27 or 0x3F depending on your module)
// 16x2 display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ========== BUTTON PINS ==========
#define SPIN_BUTTON 0        // BOOT button (GPIO 0)
#define BET_INCREASE 14
#define BET_DECREASE 13

// ========== I2C PINS ==========
#define SDA_PIN 8
#define SCL_PIN 9

// ========== GAME VARIABLES ==========
int balance = 100;           // Starting balance
int bet = 10;                // Current bet
int winnings = 0;
bool spinning = false;
unsigned long lastDisplayUpdate = 0;

// ========== SLOT SYMBOLS ==========
const int NUM_SYMBOLS = 10;
const char symbols[NUM_SYMBOLS] = {'7', '$', '*', 'O', 'X', 'C', 'P', 'D', 'B', 'M'};

// ========== DEBOUNCE VARIABLES ==========
unsigned long lastSpinTime = 0;
unsigned long lastBetTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// ========== WEB SERVER ==========
WebServer server(80);

// ========== CUSTOM CHARACTERS ==========
byte seven[8] = {0x00, 0x1F, 0x10, 0x08, 0x04, 0x04, 0x02, 0x00};
byte cherry[8] = {0x00, 0x03, 0x07, 0x1E, 0x1C, 0x08, 0x00, 0x00};

// ========== SETUP ==========
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
  pinMode(SPIN_BUTTON, INPUT);
  pinMode(BET_INCREASE, INPUT_PULLUP);
  pinMode(BET_DECREASE, INPUT_PULLUP);

  // Display welcome screen
  displayWelcome();
  delay(2000);

  // Initialize WiFi
  initializeWiFi();

  // Setup web server routes
  setupWebServer();

  displayMainScreen();
}

// ========== MAIN LOOP ==========
void loop() {
  // Handle web server requests
  server.handleClient();

  // Check button inputs (physical buttons)
  handleButtons();

  // Update display periodically
  delay(50);
}

// ========== WIFI INITIALIZATION ==========
void initializeWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());

    delay(2000);
  } else {
    Serial.println("\nFailed to connect WiFi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Check credentials");
    delay(2000);
  }
}

// ========== WEB SERVER SETUP ==========
void setupWebServer() {
  // Root page - main HTML interface
  server.on("/", handleRoot);

  // API endpoints
  server.on("/api/spin", handleSpin);
  server.on("/api/bet-increase", handleBetIncrease);
  server.on("/api/bet-decrease", handleBetDecrease);
  server.on("/api/status", handleStatus);
  server.on("/api/reset", handleReset);

  server.begin();
  Serial.println("Web server started");
}

// ========== WEB HANDLERS ==========
void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Slots Machine</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Arial', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }

        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            padding: 40px;
            max-width: 500px;
            width: 100%;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
        }

        .header h1 {
            color: #333;
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.1);
        }

        .header p {
            color: #666;
            font-size: 1.1em;
        }

        .display-box {
            background: linear-gradient(135deg, #1a1a1a 0%, #333 100%);
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 30px;
            color: #00ff00;
            font-family: 'Courier New', monospace;
            text-align: center;
        }

        .stats {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-bottom: 20px;
        }

        .stat {
            background: rgba(0, 255, 0, 0.1);
            padding: 15px;
            border-radius: 10px;
            border: 2px solid #00ff00;
        }

        .stat-label {
            font-size: 0.9em;
            opacity: 0.7;
            margin-bottom: 5px;
        }

        .stat-value {
            font-size: 1.8em;
            font-weight: bold;
        }

        .reel-display {
            background: #000;
            border: 3px solid #00ff00;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            font-size: 3em;
            letter-spacing: 20px;
            line-height: 1.2;
        }

        .controls {
            display: grid;
            gap: 15px;
        }

        .bet-control {
            display: flex;
            gap: 10px;
            margin-bottom: 10px;
        }

        button {
            flex: 1;
            padding: 15px;
            font-size: 1.1em;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            font-weight: bold;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .btn-decrease, .btn-increase {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            flex: 0.5;
        }

        .btn-decrease:hover, .btn-increase:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }

        .btn-decrease:active, .btn-increase:active {
            transform: translateY(0);
        }

        .btn-spin {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            color: white;
            padding: 25px 15px;
            font-size: 1.3em;
            min-height: 60px;
        }

        .btn-spin:hover:not(:disabled) {
            transform: translateY(-3px);
            box-shadow: 0 10px 25px rgba(245, 87, 108, 0.4);
        }

        .btn-spin:active:not(:disabled) {
            transform: translateY(-1px);
        }

        .btn-spin:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }

        .btn-reset {
            background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);
            color: #333;
            padding: 12px;
            font-size: 0.9em;
        }

        .btn-reset:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(250, 112, 154, 0.4);
        }

        .status {
            text-align: center;
            margin-top: 20px;
            color: #666;
            font-size: 0.95em;
        }

        .spinning {
            animation: pulse 0.5s infinite;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }

        .info-box {
            background: #f0f0f0;
            border-left: 4px solid #667eea;
            padding: 15px;
            margin-top: 20px;
            border-radius: 5px;
            font-size: 0.9em;
            color: #333;
        }

        .win-message {
            color: #00cc00;
            font-weight: bold;
            animation: glow 1s infinite;
        }

        @keyframes glow {
            0%, 100% { color: #00cc00; }
            50% { color: #00ff00; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎰 SLOTS</h1>
            <p>ESP32-S3 Web Interface</p>
        </div>

        <div class="display-box">
            <div class="stats">
                <div class="stat">
                    <div class="stat-label">BALANCE</div>
                    <div class="stat-value" id="balance">100</div>
                </div>
                <div class="stat">
                    <div class="stat-label">BET</div>
                    <div class="stat-value" id="bet">10</div>
                </div>
            </div>

            <div class="reel-display" id="reels">
                | 7 | 7 | 7 |
            </div>
        </div>

        <div class="controls">
            <div class="bet-control">
                <button class="btn-decrease" onclick="decreaseBet()">- BET</button>
                <button class="btn-increase" onclick="increaseBet()">+ BET</button>
            </div>

            <button class="btn-spin" id="spinBtn" onclick="spin()">🎰 SPIN</button>

            <button class="btn-reset" onclick="resetGame()">Reset Game</button>
        </div>

        <div class="status">
            <p id="statusMsg">Ready to spin!</p>
        </div>

        <div class="info-box">
            <strong>How to Play:</strong><br>
              Adjust your bet with +- BET buttons<br>
              Click SPIN to spin the reels<br>
              Match all 3 symbols to win big!<br>
              3 matching: x50 multiplier<br>
              2 matching: x5 multiplier
        </div>
    </div>

    <script>
        const API_URL = window.location.origin;
        let isSpinning = false;

        async function fetchStatus() {
            try {
                const response = await fetch(API_URL + '/api/status');
                const data = await response.json();
                
                document.getElementById('balance').textContent = data.balance;
                document.getElementById('bet').textContent = data.bet;
                
                if (data.result) {
                    displayReels(data.result);
                }
                
                updateStatus(data.status);
            } catch (error) {
                console.error('Error:', error);
            }
        }

        async function spin() {
            if (isSpinning) return;
            
            isSpinning = true;
            document.getElementById('spinBtn').disabled = true;
            document.getElementById('spinBtn').classList.add('spinning');
            document.getElementById('statusMsg').textContent = 'SPINNING...';

            try {
                const response = await fetch(API_URL + '/api/spin');
                const data = await response.json();
                
                // Animate spinning
                for (let i = 0; i < 15; i++) {
                    document.getElementById('reels').textContent = 
                        '| ' + String.fromCharCode(Math.random() > 0.5 ? 36 : 55) + ' | ' + 
                        String.fromCharCode(Math.random() > 0.5 ? 36 : 55) + ' | ' + 
                        String.fromCharCode(Math.random() > 0.5 ? 36 : 55) + ' |';
                    await sleep(100);
                }

                // Display result
                displayReels(data.result);
                updateStatus(data.message);
                
                await sleep(2000);
                
            } catch (error) {
                console.error('Error:', error);
                document.getElementById('statusMsg').textContent = 'Connection error!';
            }

            fetchStatus();
            
            isSpinning = false;
            document.getElementById('spinBtn').disabled = false;
            document.getElementById('spinBtn').classList.remove('spinning');
        }

        function displayReels(result) {
            document.getElementById('reels').innerHTML = 
                '| ' + result.reel1 + ' | ' + result.reel2 + ' | ' + result.reel3 + ' |';
        }

        function updateStatus(message) {
            const statusElement = document.getElementById('statusMsg');
            statusElement.textContent = message;
            
            if (message.includes('WIN') || message.includes('JACKPOT')) {
                statusElement.classList.add('win-message');
                setTimeout(() => statusElement.classList.remove('win-message'), 3000);
            }
        }

        async function increaseBet() {
            await fetch(API_URL + '/api/bet-increase');
            await sleep(100);
            fetchStatus();
        }

        async function decreaseBet() {
            await fetch(API_URL + '/api/bet-decrease');
            await sleep(100);
            fetchStatus();
        }

        async function resetGame() {
            if (confirm('Reset game to initial balance?')) {
                await fetch(API_URL + '/api/reset');
                await sleep(200);
                fetchStatus();
                document.getElementById('statusMsg').textContent = 'Game reset!';
            }
        }

        function sleep(ms) {
            return new Promise(resolve => setTimeout(resolve, ms));
        }

        // Update status periodically
        setInterval(fetchStatus, 2000);

        // Initial load
        fetchStatus();
    </script>
</body>
</html>
  )";

  server.sendHeader("Content-Type", "text/html; charset=utf-8");
  server.send(200, "text/html", html);
}

void handleSpin() {
  if (spinning) {
    server.send(400, "application/json", "{\"error\":\"Already spinning\"}");
    return;
  }

  if (balance < bet) {
    server.send(400, "application/json", "{\"error\":\"Insufficient balance\"}");
    return;
  }

  // Perform spin
  spinning = true;
  balance -= bet;

  // Simulate spinning animation
  for (int i = 0; i < 15; i++) {
    delay(100);
  }

  // Generate results
  int reel1 = random(NUM_SYMBOLS);
  int reel2 = random(NUM_SYMBOLS);
  int reel3 = random(NUM_SYMBOLS);

  // Check for wins
  String message = "No match... Try again!";
  int winAmount = 0;

  if (reel1 == reel2 && reel2 == reel3) {
    // Jackpot!
    winAmount = bet * 50;
    balance += winAmount;
    message = "JACKPOT! Won $" + String(winAmount) + "!";
  } else if (reel1 == reel2 || reel2 == reel3 || reel1 == reel3) {
    // Two match
    winAmount = bet * 5;
    balance += winAmount;
    message = "You won $" + String(winAmount) + "!";
  }

  // Update LCD display
  updateLCDAfterSpin(reel1, reel2, reel3, winAmount);

  // Prepare JSON response
  String response = "{\"result\":{\"reel1\":\"" + String(symbols[reel1]) + 
                    "\",\"reel2\":\"" + String(symbols[reel2]) + 
                    "\",\"reel3\":\"" + String(symbols[reel3]) + 
                    "\"},\"message\":\"" + message + 
                    "\",\"balance\":" + String(balance) + ",\"bet\":" + String(bet) + "}";

  spinning = false;
  server.send(200, "application/json", response);
}

void handleBetIncrease() {
  if (!spinning && bet < balance) {
    bet += 5;
    if (bet > balance) bet = balance;
  }
  sendStatusJSON();
}

void handleBetDecrease() {
  if (!spinning && bet > 5) {
    bet -= 5;
  }
  sendStatusJSON();
}

void handleStatus() {
  sendStatusJSON();
}

void handleReset() {
  balance = 100;
  bet = 10;
  spinning = false;
  displayMainScreen();
  sendStatusJSON();
}

void sendStatusJSON() {
  String response = "{\"balance\":" + String(balance) + 
                    ",\"bet\":" + String(bet) + 
                    ",\"spinning\":" + String(spinning) + 
                    ",\"status\":\"Ready\",\"result\":null}";
  server.send(200, "application/json", response);
}

// ========== BUTTON HANDLING ==========
void handleButtons() {
  unsigned long currentTime = millis();

  // Spin button (BOOT button - GPIO 0, active LOW)
  if (digitalRead(SPIN_BUTTON) == LOW && !spinning &&
      (currentTime - lastSpinTime) > DEBOUNCE_DELAY) {
    lastSpinTime = currentTime;
    if (balance >= bet) {
      performSpin();
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

// ========== SPIN LOGIC ==========
void performSpin() {
  spinning = true;
  balance -= bet;

  // Spinning animation
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SPINNING...");

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

  // Generate results
  int reel1 = random(NUM_SYMBOLS);
  int reel2 = random(NUM_SYMBOLS);
  int reel3 = random(NUM_SYMBOLS);

  // Display result
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("| ");
  lcd.print(symbols[reel1]);
  lcd.print(" | ");
  lcd.print(symbols[reel2]);
  lcd.print(" | ");
  lcd.print(symbols[reel3]);
  lcd.print(" |");

  delay(3000);

  // Check for wins
  if (reel1 == reel2 && reel2 == reel3) {
    winnings = bet * 50;
    balance += winnings;
    displayWin(50);
  } else if (reel1 == reel2 || reel2 == reel3 || reel1 == reel3) {
    winnings = bet * 5;
    balance += winnings;
    displayWin(5);
  } else {
    displayLoss();
  }

  spinning = false;
  delay(2000);
  displayMainScreen();
}

void updateLCDAfterSpin(int reel1, int reel2, int reel3, int winAmount) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("| ");
  lcd.print(symbols[reel1]);
  lcd.print(" | ");
  lcd.print(symbols[reel2]);
  lcd.print(" | ");
  lcd.print(symbols[reel3]);
  lcd.print(" |");

  lcd.setCursor(0, 1);
  if (winAmount > 0) {
    lcd.print("WIN $");
    lcd.print(winAmount);
  } else {
    lcd.print("Try again...");
  }
}

// ========== LCD DISPLAY FUNCTIONS ==========
void displayWelcome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SLOT MACHINE");
  lcd.setCursor(0, 1);
  lcd.print(" ESP32-S3");
}

void displayMainScreen() {
  unsigned long currentTime = millis();
  if (currentTime - lastDisplayUpdate > 500) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BAL:");
    lcd.print(balance);
    lcd.print(" BET:");
    lcd.print(bet);
    lcd.setCursor(0, 1);
    lcd.print("BOOT=SPIN");
    lastDisplayUpdate = currentTime;
  }
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
