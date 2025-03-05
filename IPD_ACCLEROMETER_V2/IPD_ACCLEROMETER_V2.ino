#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

Adafruit_MPU6050 mpu;

const int numReadings = 20;
float axReadings[numReadings];
float ayReadings[numReadings];
float azReadings[numReadings];

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
}

void loop() {
  // Collect readings
  for (int i = 0; i < numReadings; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    axReadings[i] = a.acceleration.x;
    ayReadings[i] = a.acceleration.y;
    azReadings[i] = a.acceleration.z;
    delay(10);
  }

  // Calculate and print variance
  float varX = calculateVariance(axReadings);
  float varY = calculateVariance(ayReadings);
  float varZ = calculateVariance(azReadings);

  Serial.print("Variance X: ");
  Serial.println(varX);
  Serial.print("Variance Y: ");
  Serial.println(varY);
  Serial.print("Variance Z: ");
  Serial.println(varZ);

  
  if (varX > 15) {
    Serial.println("X Hand is waving!");
  } else {
    Serial.println("No waving detected.");
  }
  if (varY > 15) {
    Serial.println("y Hand is waving!");
  } else {
    Serial.println("No waving detected.");
  }
  if (varZ > 15) {
    Serial.println("Z Hand is waving!");
  } else {
    Serial.println("No waving detected.");
  }
}

float calculateVariance(float readings[]) {
  float mean = 0.0;
  float M2 = 0.0;
  for (int i = 0; i < numReadings; i++) {
    float delta = readings[i] - mean;
    mean += delta / (i + 1);
    float delta2 = readings[i] - mean;
    M2 += delta * delta2;
  }
  return M2 / numReadings;
}

