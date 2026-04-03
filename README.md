# ESP32-S3 Slots Machine with Web Interface

An enhanced version of the original ESP32-S3 Slots Machine that adds a beautiful local web interface for controlling the game from any browser on your network.

## Features

✨ **Original Features**
- LCD1602 display with I2C interface
- Physical button controls (BOOT, GPIO 14, GPIO 13)
- Three-reel slot machine with 10 symbols
- Win detection (3-match jackpot, 2-match small win)
- Balance and bet tracking

🎰 **New Web Interface Features**
- Beautiful responsive web dashboard
- Control slots from any device on the network (phone, tablet, computer)
- Visual reel display in the browser
- Real-time balance and bet display
- Gradient-themed modern UI with animations
- Mobile-friendly design
- Live status updates
- Game reset functionality

## Hardware Requirements

- ESP32-S3 DevKit
- LCD1602 with I2C interface
- 3 push buttons (optional, for physical control)
- USB cable for programming

## Wiring

| LCM1602 Pin | ESP32-S3 Pin |
|-------------|--------------|
| GND         | GND          |
| VCC         | 5V / VIN     |
| SDA         | GPIO 8       |
| SCL         | GPIO 9       |

### Button Wiring (Optional)

| Button      | ESP32-S3 Pin |
|-------------|--------------|
| SPIN        | GPIO 0 (BOOT)|
| BET+        | GPIO 14      |
| BET-        | GPIO 13      |

## Installation

### Step 1: Install Required Libraries

1. Open Arduino IDE
2. Go to **Sketch > Include Library > Manage Libraries**
3. Search for and install:
   - **LiquidCrystal_I2C** (by Frank de Brabander)
   - (ESP32 boards and WiFi support are typically pre-installed)

### Step 2: Configure WiFi Credentials

Edit these lines in the code:

```cpp
const char* ssid = "YOUR_SSID";           // Your WiFi network name
const char* password = "YOUR_PASSWORD";   // Your WiFi password
```

### Step 3: Select Board and Upload

1. Go to **Tools > Board > esp32 > ESP32S3 Dev Module**
2. Select the correct **Port**
3. Click **Upload** (hold BOOT button if upload fails)

## Usage

### Web Interface Access

1. Open the Serial Monitor to see the ESP32's IP address after boot
2. Enter the IP address in any browser on your network (e.g., `http://192.168.1.100`)
3. You should see the slots machine interface

### Web Controls

- **+ BET / - BET**: Increase or decrease your betting amount
- **🎰 SPIN**: Spin the reels
- **Reset Game**: Reset balance to 100 and bet to 10

### Physical Controls (Optional)

- **BOOT Button (GPIO 0)**: Spin the reels
- **GPIO 14 Button**: Increase bet by 5
- **GPIO 13 Button**: Decrease bet by 5

## Game Rules

- **Starting Balance**: 100 credits
- **Minimum Bet**: 5 credits
- **3 Matching Symbols**: Win 50× your bet (JACKPOT!)
- **2 Matching Symbols**: Win 5× your bet
- **No Match**: Lose your bet

### Symbols

The game uses 10 different symbols: `7 $ * O X C P D B M`

## Winning Examples

- `| 7 | 7 | 7 |` → **JACKPOT!** Win 50× bet
- `| $ | $ | * |` → **WIN!** Win 5× bet (2 matches)
- `| O | X | C |` → No match, lose bet

## API Endpoints

The web server provides these REST API endpoints:

```
GET /                          - Main web interface
GET /api/status                - Get current game status (JSON)
GET /api/spin                  - Perform a spin
GET /api/bet-increase          - Increase bet by 5
GET /api/bet-decrease          - Decrease bet by 5
GET /api/reset                 - Reset game to initial state
```

### Status Response Example

```json
{
  "balance": 120,
  "bet": 10,
  "spinning": false,
  "status": "Ready",
  "result": null
}
```

### Spin Response Example

```json
{
  "result": {
    "reel1": "7",
    "reel2": "7",
    "reel3": "7"
  },
  "message": "JACKPOT! Won $500!",
  "balance": 600,
  "bet": 10
}
```

## Troubleshooting

### WiFi Connection Issues

1. Check SSID and password are correct
2. Ensure your ESP32 is within WiFi range
3. Check Serial Monitor for IP address
4. Try power cycling the ESP32

### LCD Not Displaying

1. Verify I2C address (default 0x27, some modules use 0x3F)
2. Check wire connections (GND, VCC, SDA, SCL)
3. Try adjusting LCD contrast with potentiometer on module
4. Check I2C bus with I2C scanner sketch

### Can't Access Web Interface

1. Find the IP address from Serial Monitor
2. Ensure you're on the same WiFi network
3. Check firewall settings
4. Try restarting the ESP32

### Buttons Not Responding

1. Check GPIO pin connections
2. Verify buttons are wired to GND correctly
3. Test with a multimeter for continuity

## Customization

### Change WiFi Credentials

```cpp
const char* ssid = "Your_Network_Name";
const char* password = "Your_Password";
```

### Change LCD I2C Address

```cpp
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Use 0x3F if 0x27 doesn't work
```

### Modify GPIO Pins

```cpp
#define SPIN_BUTTON 0       // Change to different pin
#define BET_INCREASE 14     // Change to different pin
#define BET_DECREASE 13     // Change to different pin
#define SDA_PIN 8           // Change to different pin
#define SCL_PIN 9           // Change to different pin
```

### Adjust Game Parameters

```cpp
int balance = 100;           // Starting balance
int bet = 10;                // Default bet
const unsigned long DEBOUNCE_DELAY = 200;  // Button debounce time
```

## Performance Notes

- The LCD updates every 500ms to avoid flickering
- Web interface updates status every 2 seconds
- Button debounce delay is 200ms
- Spinning animation takes ~1.5 seconds

## Differences from Original

| Feature | Original | Web Version |
|---------|----------|------------|
| Physical Controls | ✓ | ✓ |
| LCD Display | ✓ | ✓ |
| Web Interface | ✗ | ✓ |
| Remote Access | ✗ | ✓ |
| Mobile Support | ✗ | ✓ |
| API Endpoints | ✗ | ✓ |
| Animated UI | ✗ | ✓ |

## License

This project extends the original ESP32-S3 Slots Machine. Feel free to modify and use as needed.

## Support

For issues or improvements, check:
- ESP32 Documentation: https://docs.espressif.com/projects/esp-idf/
- Arduino IDE Boards Manager for ESP32
- LiquidCrystal_I2C Library Documentation

---

**Enjoy your slots machine!** 🎰
