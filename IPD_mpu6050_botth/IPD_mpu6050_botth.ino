#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
float rawxaxis = 0, rawyaxis = 0, rawzaxis = 0;
float pitch = 0;
float roll = 0;
float yaw = 0;
float meanx = 0, meany = 0, meanz = 0;
float prevvarx, prevvary, prevvarz;
double varx = 0, vary = 0, varz = 0;
int max_frequencyx, max_frequencyy, max_frequencyz;
String prevpitch = "MID";
String prevroll = "MID";
String prevyaw = "MID";
float caligyrox = 0, caligyroy = 0, caligyroz = 0;
float frequencyx[20], frequencyy[20], frequencyz[20];
float datax[20], datay[20], dataz[20];

void setup() {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  } else {
    Serial.println("MPU6050 Found!");
  }
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
}

void loop() {

  for (int i = 0; i < 10; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    rawxaxis = g.gyro.x - caligyrox;
    rawyaxis = g.gyro.y - caligyroy;
    rawzaxis = g.gyro.z - caligyroz;
    if (abs(rawxaxis) > abs(caligyrox)) {
      datax[i] = rawxaxis;
      meanx += rawxaxis;
      if (abs(roll + rawxaxis) <= 700) {
        roll = roll + rawxaxis;
      }
    }
    if (abs(rawyaxis) > abs(caligyroy)) {
      datay[i] = rawyaxis;
      meany += rawyaxis;
      if (abs(pitch + rawyaxis) <= 700) {
        pitch = pitch + rawyaxis;
      }
    }
    if (abs(rawzaxis) > abs(caligyroz)) {
      dataz[i] = rawzaxis;
      meanz += rawzaxis;
      if (abs(yaw + rawzaxis) <= 700) {
        yaw = yaw + rawzaxis;
      }
    }
  }
  meanx /= 20;
  meany /= 20;
  meanz /= 20;
  for (int i = 0; i < 20; i++) {
    varx += pow((datax[i] - meanx), 2);
    vary += pow((datay[i] - meany), 2);
    varz += pow((dataz[i] - meanz), 2);
  }
  varx /= 19;
  vary /= 19;
  varz /= 19;
  // if (varx > 0.05 && varx != prevvarx && abs(varx - prevvarx) > 0.02) {
  //   Serial.print("X = shake \t");
  // } else {
  //   Serial.print("X = STILL \t");
  // }
  // if (vary > 0.05 && vary != prevvary && abs(vary - prevvary) > 0.02) {
  //   Serial.print("Y = shake \t");
  // } else {
  //   Serial.print("Y = STILL \t");
  // }
  // if (varz > 0.05 && varz != prevvarz && abs(varz - prevvarz) > 0.02) {
  //   Serial.println("Z = shake");
  // } else {
  //   Serial.println("Z = STILL");
  // }
  Serial.println();
  if (yaw <= -650) {
    prevyaw = "finger pointing right";
  } else if (yaw > 650) {
    prevyaw = "finger pointing left";
  } else if (abs(yaw) > 0 && abs(yaw) < 600) {
    prevyaw = "MID";
  }
  if (pitch <= -650) {
    prevpitch = "down1";
  } else if (pitch > 650) {
    prevpitch = "UP";
  } else if (abs(pitch) > 0 && abs(pitch) < 600) {
    prevpitch = "MID";
  }
  if (roll <= -650) {
    prevroll = "rigth";
  } else if (roll > 650) {
    prevroll = "left";
  } else if (abs(roll) > 0 && abs(roll) < 600) {
    prevroll = "MID";
  }

  Serial.print("roll = ");
  Serial.print(roll);
  Serial.print("\t");
  Serial.print("pitch = ");
  Serial.print(pitch);
  Serial.print("\t");
  Serial.print("yaw = ");
  Serial.println(yaw);
  Serial.println("=========================================================================");

  prevvarx = varx;
  prevvary = vary;
  prevvarz = varz;
  delay(10);
}
