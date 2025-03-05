#include <TFT_eSPI.h>  // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// //Pins
// #define r1 18
// #define r2 19
// #define prev 32
// #define ok 33
// #define next 25
// #define back 35
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 9
#define TFT_MOSI 11
#define TFT_SCLK 13
// #define buttonPin 2

// Create a sensor object
Adafruit_MPU6050 mpu;

//Variables
sensors_event_t a, g, temp;
float rawxaxis = 0, rawyaxis = 0, rawzaxis = 0;
float pitch = 0, roll = 0, yaw = 0;
String prevpitch = "MID", prevroll = "MID", prevyaw = "MID";
String r1read = "OPEN", r2read = "MID", r3read = "MID";
float caligyrox = 0, caligyroy = 0, caligyroz = 0;
int max_frequencyx, max_frequencyy, max_frequencyz;
float prevvarx, prevvary, prevvarz;
double varx = 0, vary = 0, varz = 0;
float frequencyx[20], frequencyy[20], frequencyz[20];
float datax[20], datay[20], dataz[20];
float meanx = 0, meany = 0, meanz = 0;
char *selectedmode = "letter";
uint16_t okpressedcount = 0;
bool ReadingRqe = false;
String dataset_reading = "";
const int ledPin = 2;
bool recording = false;
bool lockgyro = false;
bool correction = false;
const int numReadings = 20;
float axReadings[numReadings];
float ayReadings[numReadings];
float azReadings[numReadings];
TFT_eSPI tft = TFT_eSPI();
// GUI function
void homepage() {
  ReadingRqe = false;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(0xBAA0);
  tft.drawString("Select", 5, 4, 4);
  tft.drawString("Mode", 50, 30, 4);
  tft.fillRect(0, 55, 128, 27, TFT_WHITE);
  // if (selectedmode == "letter") {
  //   prevbtn();
  // } else {
  //   nextbtn();
  // }
}

const int pitchThreshold = 450;
const int rollThreshold = 450;
const int yawThreshold = 375;  // Threshold for considering pitch, roll, and yaw as neutral
// Declare last values for roll, pitch, and yaw
float lastRoll = 0;
float lastPitch = 0;
float lastYaw = 0;

void getGyroReadings() {
  for (int i = 0; i < 20; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Calculate raw gyro values
    rawxaxis = g.gyro.x - caligyrox;
    rawyaxis = g.gyro.y - caligyroy;
    rawzaxis = g.gyro.z - caligyroz;
    axReadings[i] = a.acceleration.x;
    ayReadings[i] = a.acceleration.y;
    azReadings[i] = a.acceleration.z;

    if ((lastRoll <= 0 && rawxaxis > 0) || (lastRoll >= 0 && roll < 0)) {
      roll = 0;  // Reset roll to zero
    }
    if (rawxaxis > 0.5) {
      roll = roll + 10;
    } else if (rawxaxis < -0.5) {
      roll = roll - 10;
    }
    if ((lastPitch < 0 && pitch > 0) || (lastPitch > 0 && pitch < 0)) {
      Serial.println("triggered");
      pitch = 0;  // Reset pitch to zero
      break;
    }
    if (rawyaxis > 0.25) {
      pitch = pitch + 10;
    } else if (rawyaxis < -0.5) {
      pitch = pitch - 10;
    }
    if ((lastYaw <= 0 && rawzaxis > 0) || (lastYaw >= 0 && yaw < 0)) {
      yaw = 0;  // Reset yaw to zero
    }
    if (rawzaxis > 0.5) {
      yaw = yaw + 10;
    } else if (rawzaxis < -0.5) {
      yaw = yaw - 10;
    }
    lastRoll = roll;
    lastYaw = yaw;
    lastPitch = pitch;
    roll = constrain(roll, -500, 500);
    pitch = constrain(pitch, -500, 500);
    yaw = constrain(yaw, -500, 500);
    delay(10);
  }

  // Calculate and print variance
  varx = calculateVariance(axReadings);
  vary = calculateVariance(ayReadings);
  varz = calculateVariance(azReadings);



  if (pitch == -500) {
    prevpitch = "down";
  } else if (pitch == 500) {
    prevpitch = "up";
  } else {
    prevpitch = "mid";
  }

  if (roll <= -450) {
    prevroll = "right";
  } else if (roll >= 480) {
    prevroll = "left";
  } else {
    prevroll = "mid";
  }

  if (yaw <= -yawThreshold) {
    prevyaw = "finger pointing left";
  } else if (yaw >= yawThreshold) {
    prevyaw = "finger pointing right";
  } else {
    prevyaw = "finger pointing mid";
  }
}


float calculateVariance(float readings[]) {
  float mean = 0.0;
  float M2 = 0.0;
  for (int i = 0; i < 20; i++) {
    float delta = readings[i] - mean;
    mean += delta / (i + 1);
    float delta2 = readings[i] - mean;
    M2 += delta * delta2;
  }
  return M2 / 20;
}

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  } else {
    Serial.println("MPU6050 Found!");
  }
  tft.init();
  //Flash screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(0xFFFFFFFF);
  tft.drawString("IPD", 44, 55, 4);
  tft.drawString("Project", 25, 80, 4);
  delay(1000);
  homepage();

  // Gyro calibration
  for (int i = 0; i < 20; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    datax[i] = g.gyro.x;
    datay[i] = g.gyro.y;
    dataz[i] = g.gyro.z;
  }
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 20; j++) {
      if (datax[i] == datax[j]) {
        frequencyx[i]++;
      }
      if (datay[i] == datay[j]) {
        frequencyy[i]++;
      }
      if (dataz[i] == dataz[j]) {
        frequencyz[i]++;
      }
    }
  }
  for (int i = 0; i < 20; i++) {
    if (frequencyx[i] > max_frequencyx) {
      max_frequencyx = frequencyx[i];
      caligyrox = datax[i];
    }
    if (frequencyy[i] > max_frequencyy) {
      max_frequencyy = frequencyy[i];
      caligyroy = datay[i];
    }
    if (frequencyz[i] > max_frequencyz) {
      max_frequencyz = frequencyz[i];
      caligyroz = dataz[i];
    }
  }
  Serial.println(caligyrox);
  Serial.println(caligyroy);
  Serial.println(caligyroz);
  Serial.println("Calibration Successful");
}

void loop() {
  getGyroReadings();
  Serial.print("Pitch: ");
  Serial.print(prevpitch);
  Serial.print(" (Value: ");
  Serial.print(pitch);
  Serial.println(")");
  Serial.println();
  // Serial.print(")  Roll: ");
  // Serial.print(prevroll);
  // Serial.print(" (Value: ");
  // Serial.print(rawxaxis);
  // Serial.print(")  Yaw: ");
  // Serial.print(prevyaw);
  // Serial.print(" (Value: ");
  // Serial.print(rawzaxis);
  // Serial.println(")  ");
  // Serial.print("Variance X: ");

  // if (varx > 5) {
  //   Serial.print("X Hand is waving!");
  //   lockgyro = true;
  // } else {
  //   Serial.print("No waving detected.");
  //   lockgyro = false;
  // }
  // Serial.print(" (Value: ");
  // Serial.print(varx);
  // Serial.print(")  Variance Y: ");
  // if (vary > 10) {
  //   Serial.print(prevroll);
  //   Serial.print("y Hand is waving!");
  //   lockgyro = true;
  // } else {
  //   Serial.print("No waving detected.");
  //   lockgyro = false;
  // }
  // Serial.print(" (Value: ");
  // Serial.print(vary);

  // Serial.print(")  Variance Z: ");
  // if (varz > 5) {
  //   Serial.print("Z Hand is waving!");
  //   lockgyro = true;
  // } else {
  //   Serial.print("No waving detected.");
  //   lockgyro = false;
  // }
  // Serial.print(" (Value: ");
  // Serial.print(varz);
  // Serial.println(")");
  // Serial.println(" ");
  // if (digitalRead(buttonPin)) {
  //   if (!recording) {
  //     // Start recording
  //     recording = true;  // Reset reading count
  //     getGyroReadings();

  //     digitalWrite(ledPin, HIGH);  // Turn on LED to indicate recording
  //     Serial.println();
  //     Serial.println("Recording started...");
  //   }
  // } else {
  //   if (recording) {
  //     // Stop recording
  //     recording = false;
  //     digitalWrite(ledPin, LOW);
  //     dataset_reading = prevpitch + "," + prevroll + "," + prevyaw;
  //     Serial.print("dataset_reading: ");
  //     Serial.println(dataset_reading);
  //     Serial.println("Recording stopped.");
  //     Serial.println();
  //     Serial.println();
  //     // saveToCSV();
  //   }
  // }
  // if (ReadingRqe) {
  // }
  delay(10);
}