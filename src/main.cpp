/*
By Dr.Kishor Kumar
See the readme.md file more information about this example.
*/
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>


// Uncomment the following line to enable NeoPixel functionality
#define ENABLE_NEOPIXEL

#ifdef ENABLE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
const int neoPixelPin = 38;
const int numPixels = 1;
Adafruit_NeoPixel pixels(numPixels, neoPixelPin, NEO_GRB + NEO_KHZ800);
#endif

// Define the GPIO pin for the relay
const int relayPin = 15;
const int statusLEDPin = 13;

// Create an instance of the AsyncWebServer
AsyncWebServer server(80);

// Variables to store timer values for the relay (on and off times in milliseconds)
unsigned long relayOnTimer = 0;
unsigned long relayOffTimer = 0;

// Variable to track the last time the relay was toggled
unsigned long lastRelayToggleTime = 0;

// Relay state (0 = off, 1 = on)
int relayState = 0;

// Timer running state
bool timerRunning = false;

void toggleRelay(int state) {
  digitalWrite(relayPin, state);
}

void statusLEDBlinkTaskFunction(void* parameter) {
  while (true) {
    if (timerRunning) {
      if (relayState) {
        digitalWrite(statusLEDPin, HIGH); // Turn on the LED when the relay is on
        #ifdef ENABLE_NEOPIXEL
        pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Set NeoPixel to green when relay is on
        pixels.show();
        #endif
      } else {
        digitalWrite(statusLEDPin, LOW); // Turn off the LED when the relay is off
        #ifdef ENABLE_NEOPIXEL
        pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Set NeoPixel to red when relay is off
        pixels.show();
        #endif
      }
    } else {
      digitalWrite(statusLEDPin, LOW); // Turn off the LED when the timer is not running
      #ifdef ENABLE_NEOPIXEL
      pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Turn off NeoPixel when timer is not running
      pixels.show();
      #endif
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to avoid excessive loop iterations
  }
}

void saveTimers() {
  File timersFile = LittleFS.open("/timers.txt", "w");
  if (timersFile) {
    timersFile.println(relayOnTimer);
    timersFile.println(relayOffTimer);
    timersFile.close();
    Serial.println("Timers saved to LittleFS");
  } else {
    Serial.println("Failed to save timers to LittleFS");
  }
}

void loadTimers() {
  File timersFile = LittleFS.open("/timers.txt", "r");
  if (timersFile) {
    relayOnTimer = timersFile.readStringUntil('\n').toInt(); // Convert to milliseconds
    relayOffTimer = timersFile.readStringUntil('\n').toInt(); // Convert to milliseconds
    timersFile.close();
    Serial.println("Timers loaded from LittleFS");
  } else {
    Serial.println("Failed to load timers from LittleFS, using default values");
    // If the file doesn't exist, use default values (timers set to 0)
  }
}

void setup() {
  // Serial communication for debugging
  Serial.begin(115200);

  // Check if PSRAM is available
  if (psramFound()) {
    Serial.println("PSRAM found and initialized");
  } else {
    Serial.println("PSRAM not found");
  }

  // Initialize NeoPixel
  #ifdef ENABLE_NEOPIXEL
  pixels.begin();
  #endif

  // Initialize the GPIO pins
  pinMode(relayPin, OUTPUT);
  pinMode(statusLEDPin, OUTPUT);

  // Turn off the relay and status LED initially
  digitalWrite(relayPin, LOW);
  digitalWrite(statusLEDPin, LOW);

  // Start ESP32 in Access Point (AP) mode
  WiFi.softAP("RELAY BOARD", "123456789");

  // Initialize OTA
  ArduinoOTA.begin();

  // Create and start the status LED blink task
  xTaskCreatePinnedToCore(
    statusLEDBlinkTaskFunction,
    "statusLEDBlinkTask",
    10000,          // Stack size
    NULL,
    1,              // Priority
    NULL,
    0               // Task Core (0 or 1)
  );

 
  // Mount the LittleFS file system
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return;
  }
 
  // Serve the HTML file from LittleFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  // Serve the style.css file from LittleFS
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Handle POST request to set relay timers
  server.on("/setTimers", HTTP_POST, [](AsyncWebServerRequest *request){
    relayOnTimer = request->arg("relay_on").toInt() * 1000; // Convert to milliseconds
    relayOffTimer = request->arg("relay_off").toInt() * 1000; // Convert to milliseconds
    saveTimers(); // Save timers to LittleFS
    request->redirect("/"); // Redirect back to the main page
  });
  // Handle GET request to get the current timer values
  server.on("/getTimerValues", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = "{\"relayOnTimer\":" + String(relayOnTimer / 1000) + ",\"relayOffTimer\":" + String(relayOffTimer / 1000) + "}";
    request->send(200, "application/json", response);
  });

    // Handle GET request to get the current timer state
  server.on("/getTimerState", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = "{\"timerRunning\":" + String(timerRunning ? "true" : "false") + "}";
    request->send(200, "application/json", response);
  });
  // Handle POST request to toggle the timer
  server.on("/toggleTimer", HTTP_POST, [](AsyncWebServerRequest *request){
    timerRunning = !timerRunning;
    if (timerRunning) {
      lastRelayToggleTime = millis(); // Reset the last toggle time when starting the timer
    }
    request->redirect("/"); // Redirect back to the main page
  });

  // Start the server
  server.begin();
  // Load saved timers from LittleFS
  loadTimers();
}

void loop() {
  ArduinoOTA.handle();

  // Get the current time in milliseconds
  unsigned long currentTime = millis();

  // Update the relay state based on the timers
  if (timerRunning) {
    if (relayOnTimer > 0 && currentTime - lastRelayToggleTime < relayOnTimer) {
      // Turn on the relay during the on time
      toggleRelay(HIGH);
      relayState = 1;
    } else if (relayOffTimer > 0 && currentTime - lastRelayToggleTime >= relayOnTimer && currentTime - lastRelayToggleTime < relayOnTimer + relayOffTimer) {
      // Turn off the relay during the off time
      toggleRelay(LOW);
      relayState = 0;
    } else {
      // Reset the last toggle time when the on+off cycle is complete
      lastRelayToggleTime = currentTime;
    }
  } else {
    // Turn off the relay when the timer is not running
    toggleRelay(LOW);
    relayState = 0;
  }

  // Yield to other tasks
  vTaskDelay(pdMS_TO_TICKS(10)); // Adjust the delay as needed
}