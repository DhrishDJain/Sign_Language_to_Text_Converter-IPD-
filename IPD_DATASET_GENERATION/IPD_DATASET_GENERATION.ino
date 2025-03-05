#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SD.h>

// Create an instance of the MPU6050
Adafruit_MPU6050 mpu;
#define SD_CS 27
#define SD_SCLK 14
#define SD_MOSI 12
#define SD_MISO 13
// Pin definitions
const int buttonPin = 34;                                // Change this to your main recording button pin
const int fingerButtonPins[5] = { 35, 32, 33, 25, 26 };  // Pins for the 5 finger buttons
const int ledPin = 2;                                    // Optional: LED pin to indicate recording status
const int chipSelect = 5;                                // Change this to your SD card CS pin

bool recording = false;
const int maxReadings = 1000;                // Maximum number of readings to store
int readingCount = 0;

// Arrays to store sensor data
float ax[maxReadings], ay[maxReadings], az[maxReadings];
float gx[maxReadings], gy[maxReadings], gz[maxReadings];
bool fingerStates[maxReadings][5];  // Array to store finger button states

void setup() {
  Serial.begin(115200);
  Wire.begin();
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  SPI.setFrequency(1000000);
  while (!SD.begin(SD_CS)) {
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    Serial.println("Searching for sd card..");
  }
  // Initialize the MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 found!");

  // Set the range of the accelerometer and gyroscope
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  pinMode(buttonPin, INPUT_PULLUP);  // Use internal pull-up resistor for main button
  for (int i = 0; i < 5; i++) {
    pinMode(fingerButtonPins[i], INPUT_PULLUP);  // Use internal pull-up resistor for finger buttons
  }
  pinMode(ledPin, OUTPUT);

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");
  Serial.println("Ax,Ay,Az,Gx,Gy,Gz,Finger1,Finger2,Finger3,Finger4,Finger5");
}

void loop() {
  // Check if the main button is pressed
  if (digitalRead(buttonPin) == LOW) {
    if (!recording) {
      // Start recording
      recording = true;
      readingCount = 0;            // Reset reading count
      digitalWrite(ledPin, HIGH);  // Turn on LED to indicate recording
      Serial.println("Recording started...");
    }
  } else {
    if (recording) {
      // Stop recording
      recording = false;
      digitalWrite(ledPin, LOW);  // Turn off LED
      Serial.println("Recording stopped.");
      saveToCSV();
    }
  }

  // If recording, read sensor data
  if (recording && readingCount < maxReadings) {
    //   sensors_event_t a, g, temp;
    //   mpu.getEvent(&a, &g, &temp);

    //   // Store accelerometer and gyroscope data
    //   ax[readingCount] = a.acceleration.x;
    //   ay[readingCount] = a.acceleration.y;
    //   az[readingCount] = a.acceleration.z;
    //   gx[readingCount] = g.gyro.x;
    //   gy[readingCount] = g.gyro.y;
    //   gz[readingCount] = g.gyro.z;

    //   // Read finger button states
    //   for (int i = 0; i < 5; i++) {
    //     fingerStates[readingCount][i] = digitalRead(fingerButtonPins[i]) == HIGH;  // HIGH means finger is open
    //   }

    //   // Print sensor data to Serial
    //   Serial.print("Ax: ");
    //   Serial.print(ax[readingCount]);
    //   Serial.print(", Ay: ");
    //   Serial.print(ay[readingCount]);
    //   Serial.print(", Az: ");
    //   Serial.print(az[readingCount]);
    //   Serial.print(", Gx: ");
    //   Serial.print(gx[readingCount]);
    //   Serial.print(", Gy: ");
    //   Serial.print(gy[readingCount]);
    //   Serial.print(", Gz: ");
    //   Serial.print(gz[readingCount]);
    //   Serial.print(", Finger States: ");
    //   for (int i = 0; i < 5; i++) {
    //     Serial.print(fingerStates[readingCount][i] ? "Open " : "Closed ");
    //   }
    //   Serial.println();

    //   readingCount++;
    // }

    // Check if the recording duration has elapsed
    if (recording) {
      recording = false;
      digitalWrite(ledPin, LOW);  // Turn off LED
      // Serial.println("Recording duration reached. Stopping...");
      // saveToCSV();
    }

    delay(100);  // Adjust delay as needed
  }

  void saveToCSV() {
    File file = SD.open("sensor_data.csv", FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }

    // Write CSV header
    file.println("Ax,Ay,Az,Gx,Gy,Gz,Finger1,Finger2,Finger3,Finger4,Finger5");

    // // Write sensor data
    // for (int i = 0; i < readingCount; i++) {
    //   file.print(ax[i]);
    //   file.print(",");
    //   file.print(ay[i]);
    //   file.print(",");
    //   file.print(az[i]);
    //   file.print(",");
    //   file.print(gx[i]);
    //   file.print(",");
    //   file.print(gy[i]);
    //   file.print(",");
    //   file.print(gz[i]);
    //   file.print(",");
    //   for (int j = 0; j < 5; j++) {
    //     file.print(fingerStates[i][j] ? "Open" : "Closed");
    //     if (j < 4) file.print(",");  // Add comma between finger states
    //   }
    //   file.println();
    // }

    file.close();
    Serial.println("Data saved to sensor_data.csv");
  }