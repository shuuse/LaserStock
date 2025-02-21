#ifndef CONFIG_H
#define CONFIG_H
#include "secrets.h"  

// Stock Configuration
const char* STOCK_TICKER = "AAPL";
const float PRICE_MIN = 220.0;   // Lowest stock price mapped to 0° on servo
const float PRICE_MAX = 260.0;   // Highest stock price mapped to 180° on servo

// Servo Configuration
const int SERVO_PIN = 25;         // Servo control on IO5
const int PULSE_WIDTH_MIN = 500;  // Adjusted minimum
const int PULSE_WIDTH_MAX = 2400; // Adjusted maximum

const int MOVE_DELAY = 30;       // Delay between servo movements
const int HOLD_TIME = 1500;      // Time to hold position before releasing

// Network Configuration
const char* WIFI_SSID = "Kloaknettet";
const char* WIFI_PASSWORD = "34D934D934";
const char* API_ENDPOINT = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=%s&apikey=%s";
const char* API_KEY = "B33ZYL2SWE2Y7XAD";
const int UPDATE_INTERVAL = 14400000;  // 4 hours in milliseconds

// Motion Sensor Configuration
const int MOTION_SENSOR_PIN = 18;  // GPIO for motion sensor
const int LASER_PIN = 16;          // GPIO for laser
const int MOTION_DURATION = 5000;  // Laser stays on for 5 seconds after motion detected


// Error Handling
const int MAX_RETRY_ATTEMPTS = 2;   // Reduce retries to conserve API calls
const int RETRY_DELAY = 10000;      // Delay between retries (10 seconds)

// Trading Hours (NASDAQ)
const int TRADING_START_HOUR = 9;   // 9 AM EST
const int TRADING_END_HOUR = 16;    // 4 PM EST

#endif // CONFIG_H
