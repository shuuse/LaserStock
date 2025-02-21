#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"

// Global Variables
Servo stockServo;
Preferences preferences;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000, 60000); // EST Timezone
float lastStockPrice = -1;
unsigned long lastFetchTime = 0;
String lastFetchTimestamp = "Unknown";
volatile bool motionDetected = false;  // Flag to track motion detection
unsigned long laserOffTime = 0;        // Timestamp to turn off laser


// Function Declarations
void connectToWiFi();
float fetchStockPrice();
bool isTradingHours();
void moveServo(float price);
void saveStockPrice(float price, const String& timestamp);
void loadStockPrice();
void syncTime();

void connectToWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) {
        delay(500);
        Serial.print(".");
        attempt++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi.");
    }
}

void IRAM_ATTR motionDetectedISR() {
  motionDetected = true;  // Set the flag
  laserOffTime = millis() + MOTION_DURATION;  // Set time to turn off the laser
}

void syncTime() {
    Serial.println("Syncing time...");
    timeClient.begin();
    if (!timeClient.update()) {
        Serial.println("Failed to sync time, using last stored data.");
    } else {
        Serial.print("Time Synced: ");
        Serial.println(timeClient.getFormattedTime());
    }
}

float fetchStockPrice() {
  if (WiFi.status() != WL_CONNECTED) {
      Serial.println("No WiFi connection, using last stored price.");
      return lastStockPrice;
  }

  if (!isTradingHours()) {
      Serial.println("Outside trading hours, using last stored price.");
      return lastStockPrice;
  }

  char url[256];
  snprintf(url, sizeof(url), API_ENDPOINT, STOCK_TICKER, API_KEY);

  Serial.print("Fetching stock price from: ");
  Serial.println(url);

  HTTPClient http;
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode != 200) {
      Serial.print("HTTP Request Failed: ");
      Serial.println(httpResponseCode);
      http.end();
      return lastStockPrice;
  }

  String payload = http.getString();
  http.end();

  Serial.print("Received JSON: ");
  Serial.println(payload);  // Debug: Print full JSON response

  StaticJsonDocument<1024> doc;  // Use a larger buffer
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
      Serial.print("JSON Parse Error: ");
      Serial.println(error.f_str());
      return lastStockPrice;
  }

  // Extract the stock price from the JSON
  JsonObject quote = doc["Global Quote"];
  if (!quote.containsKey("05. price")) {
      Serial.println("Error: '05. price' key not found in JSON.");
      return lastStockPrice;
  }

  float stockPrice = quote["05. price"].as<float>();

  if (stockPrice > 0) {
      Serial.print("Stock Price Retrieved: ");
      Serial.println(stockPrice);
      lastFetchTimestamp = timeClient.getFormattedTime();
      saveStockPrice(stockPrice, lastFetchTimestamp);
      return stockPrice;
  } else {
      Serial.println("Invalid stock price received.");
      return lastStockPrice;
  }
}


bool isTradingHours() {
    timeClient.update();
    int currentHour = timeClient.getHours();
    return (currentHour >= TRADING_START_HOUR && currentHour < TRADING_END_HOUR);
}

void moveServo(float price) {
  float ratio = (price - PRICE_MIN) / (PRICE_MAX - PRICE_MIN);
  int pulseWidth = PULSE_WIDTH_MAX - ratio * (PULSE_WIDTH_MAX - PULSE_WIDTH_MIN);  // Reverse mapping
  pulseWidth = constrain(pulseWidth, PULSE_WIDTH_MIN, PULSE_WIDTH_MAX);

  Serial.print("Price: "); Serial.print(price);
  Serial.print(" | Ratio: "); Serial.print(ratio);
  Serial.print(" | Corrected Pulse Width: "); Serial.println(pulseWidth);

  stockServo.attach(SERVO_PIN, PULSE_WIDTH_MIN, PULSE_WIDTH_MAX);
  stockServo.writeMicroseconds(pulseWidth);
  delay(HOLD_TIME);
  stockServo.detach();
}



void saveStockPrice(float price, const String& timestamp) {
    preferences.begin("stock_data", false);
    preferences.putFloat("lastPrice", price);
    preferences.putString("lastTimestamp", timestamp);
    preferences.end();
}

void loadStockPrice() {
    preferences.begin("stock_data", true);
    lastStockPrice = preferences.getFloat("lastPrice", -1);
    lastFetchTimestamp = preferences.getString("lastTimestamp", "Unknown");
    preferences.end();

    Serial.print("Last Stored Price: ");
    Serial.println(lastStockPrice);
    Serial.print("Last Fetched At: ");
    Serial.println(lastFetchTimestamp);
}

void setup() {
  Serial.begin(115200);

  delay(1000);  
  connectToWiFi();
  syncTime();
  
  // Ensure Laser Pin is configured as OUTPUT first
  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN, HIGH);  // Turn on laser before servo test sequence
  delay(1000);  
  // Attach servo only when needed
  stockServo.attach(SERVO_PIN);

  // Servo Test Sequence: Moves from min to max in 5 steps
  float testPrices[] = {
      PRICE_MIN, 
      PRICE_MIN + (float)0.25 * (PRICE_MAX - PRICE_MIN),
      PRICE_MIN + (float)0.50 * (PRICE_MAX - PRICE_MIN),
      PRICE_MIN + (float)0.75 * (PRICE_MAX - PRICE_MIN),
      PRICE_MAX  
  };

  for (int i = 0; i < 5; i++) {
      Serial.print("Testing servo at simulated price: ");
      Serial.println(testPrices[i]);
      moveServo(testPrices[i]);  
      delay(1000);  
  }

  // Detach servo after test sequence to prevent excess power draw
  stockServo.detach();

  digitalWrite(LASER_PIN, LOW);  // Turn off laser after startup sequence


  // Load last stored stock price
  loadStockPrice();
  lastStockPrice = fetchStockPrice();
  lastFetchTime = millis();

  Serial.print("Current Stock Price: ");
  Serial.println(lastStockPrice);
  Serial.print("Last Fetch Time: ");
  Serial.println(lastFetchTimestamp);

  // Move servo to actual stock price after test sequence
  if (lastStockPrice > 0) {
      stockServo.attach(SERVO_PIN);  // Reattach servo before moving
      moveServo(lastStockPrice);
      stockServo.detach();  // Detach again after movement
  }

  // Initialize Motion Sensor
  pinMode(MOTION_SENSOR_PIN, INPUT);

  // Attach interrupt to detect motion
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_PIN), motionDetectedISR, RISING);
}



void handleSerialInput() {
  if (Serial.available()) {
      char input = Serial.read();
      if (input == 'u') {
          lastStockPrice += 5;  // Increase price by 5
          Serial.print("Simulated increase: New stock price = ");
          Serial.println(lastStockPrice);
          moveServo(lastStockPrice);
      } 
      else if (input == 'd') {
          lastStockPrice -= 5;  // Decrease price by 5
          Serial.print("Simulated decrease: New stock price = ");
          Serial.println(lastStockPrice);
          moveServo(lastStockPrice);
      }
  }
}

void loop() {
  // Check for manual stock price simulation
  handleSerialInput();

  // Check if it's time to update stock price
  if (millis() - lastFetchTime >= UPDATE_INTERVAL) {
      lastStockPrice = fetchStockPrice();
      lastFetchTime = millis();
      moveServo(lastStockPrice);
  }

  // Handle motion detection
  if (motionDetected) {
      Serial.println("Motion detected! Turning on laser.");
      digitalWrite(LASER_PIN, HIGH);
      motionDetected = false;
  }

  // Turn off laser after 5 seconds
  if (millis() >= laserOffTime && digitalRead(LASER_PIN) == HIGH) {
      Serial.println("Turning off laser.");
      digitalWrite(LASER_PIN, LOW);
  }

  delay(100);
}

