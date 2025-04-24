# ESP32-S3 Voice Assistant with RGB LED & Buzzer Control

![Project Banner](https://placehold.co/600x200/EEE/31343C?text=ESP32-S3+Voice+Assistant) <!-- Add your banner here -->

Advanced voice control project for ESP32-S3-DevKit-C with INMP441 MEMS microphone, RGB LED control, and buzzer integration. Built on ESP-Skainet with IDF 5.x.

## 🛠 Hardware Setup

### Components
- ESP32-S3-DevKit-C
- INMP441 MEMS Microphone
- RGB LED (Common Cathode)
- Passive Buzzer
- Jumper Wires

### Connections
| Component       | ESP32-S3 Pin |
|-----------------|--------------|
| INMP441 LRCK    | GPIO 11      |
| INMP441 SCLK    | GPIO 12      |
| INMP441 SDIN    | GPIO 10      |
| Strip LED       | GPIO 48      |
| Buzzer          | GPIO 9       |

![Wiring Diagram](https://placehold.co/600x300/EEE/31343C?text=Wiring+Diagram+Here) <!-- Add your diagram here -->

## ⚙️ Software Configuration

### 1. Clone Repository
```bash
git clone --branch ESP32-S3-Devkit-C --recursive https://github.com/0015/esp-skainet.git
cd esp-skainet/examples/en_speech_commands_recognition

Edit components/hardware_driver/boards/include/esp32_s3_devkit_c.h
// I2S Configuration
#define FUNC_I2S_EN         (1)
#define GPIO_I2S_LRCK       (GPIO_NUM_11)
#define GPIO_I2S_SCLK       (GPIO_NUM_12)
#define GPIO_I2S_SDIN       (GPIO_NUM_10)

#define RGB_STRIP_GPIO        GPIO_NUM_48


// Buzzer Configuration
#define GPIO_BUZZER         (GPIO_NUM_9)

idf.py set-target esp32s3
idf.py menuconfig

🎙 Supported Voice Commands
Command	Action
"Turn on the light"	Enable built-in LED
"Turn off the light"	Disable built-in LED
"Turn on red light"	Activate red LED
"Turn off red light"	Deactivate red LED
"Turn on green light"	Activate green LED
"Turn off green light"	Deactivate green LED
"Turn on blue light"	Activate blue LED
"Turn off blue light"	Deactivate blue LED
🚀 Build & Flash
idf.py build
idf.py -p PORT flash monitor  # Replace PORT with your COM port
🔧 Customization
idf.py menuconfig
📌 Important Notes
Ensure proper power supply for multiple LEDs

Buzzer requires PWM configuration

MEMS microphone needs stable mounting

Check GPIO assignments match physical connections

📚 Resources
ESP-Skainet Documentation

INMP441 Datasheet

ESP32-S3 Pinout
