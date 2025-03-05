#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
float x_acc = 0, y_acc = 0, z_acc = 0;
float meanx = 0, meany = 0, meanz = 0;
float prevvarx, prevvary, prevvarz;
double varx = 0, vary = 0, varz = 0;
int max_frequencyx, max_frequencyy, max_frequencyz;
String prevy_acc = "MID";
String prevx_acc = "MID";
String prevz_acc = "MID";
float caliaccx = 0, caliaccy = 0, caliaccz = 0;
float frequencyx[20], frequencyy[20], frequencyz[20];
float decision_arrx[20], decision_arry[20], decision_arrz[20];
void setup(void) {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  } else {
    Serial.println("MPU6050 Found!");
  }
  for (int i = 0; i < 20; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    decision_arrx[i] = g.acceleration.x;
    decision_arry[i] = g.acceleration.y;
    decision_arrz[i] = g.acceleration.z;
  }
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 20; j++) {
      if (decision_arrx[i] == decision_arrx[j]) {
        frequencyx[i]++;
      }
      if (decision_arry[i] == decision_arry[j]) {
        frequencyy[i]++;
      }
      if (decision_arrz[i] == decision_arrz[j]) {
        frequencyz[i]++;
      }
    }
  }
  for (int i = 0; i < 20; i++) {
    if (frequencyx[i] > max_frequencyx) {
      max_frequencyx = frequencyx[i];
      caliaccx = decision_arrx[i];
    }
    if (frequencyy[i] > max_frequencyy) {
      max_frequencyy = frequencyy[i];
      caliaccy = decision_arry[i];
    }
    if (frequencyz[i] > max_frequencyz) {
      max_frequencyz = frequencyz[i];
      caliaccz = decision_arrz[i];
    }
  }
}
void loop() {
  for (int i = 0; i < 20; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    x_acc = g.acceleration.x - caliaccx;
    y_acc = g.acceleration.y - caliaccy;
    z_acc = g.acceleration.z - caliaccz;
    if (abs(x_acc) > abs(caliaccx)) {
      decision_arrx[i] = x_acc;
      meanx += x_acc;
    }
    if (abs(y_acc) > abs(caliaccy)) {
      decision_arry[i] = y_acc;
      meany += y_acc;
    }
    if (abs(z_acc) > abs(caliaccz)) {
      decision_arrz[i] = z_acc;
      meanz += z_acc;
    }
  }
    meanx /= 20;
    meany /= 20;
    meanz /= 20;
    for (int i = 0; i < 20; i++) {
      varx += pow((decision_arrx[i] - meanx), 2);
      vary += pow((decision_arry[i] - meany), 2);
      varz += pow((decision_arrz[i] - meanz), 2);
    }
    varx /= 19;
    vary /= 19;
    varz /= 19;
    if (varx > 0.05 && varx != prevvarx && abs(varx - prevvarx) > 0.02) {
      Serial.print("X = shake \t");
    } else {
      Serial.print("X = STILL \t");
    }
    if (vary > 0.05 && vary != prevvary && abs(vary - prevvary) > 0.02) {
      Serial.print("Y = shake \t");
    } else {
      Serial.print("Y = STILL \t");
    }
    if (varz > 0.05 && varz != prevvarz && abs(varz - prevvarz) > 0.02) {
      Serial.println("Z = shake");
    } else {
      Serial.println("Z STILL");
    }
    prevvarx = varx;
    prevvary = vary;
    prevvarz = varz;
  delay(10);
}