#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
float rawxaxis = 0;
float rawyaxis = 0;
float rawzaxis = 0;
float pitch = 0;
float roll = 0;
float yaw = 0;
String prevpitch = "MID";
String prevroll = "MID";
String prevyaw = "MID";
float caligyrox = 0, caligyroy = 0, caligyroz = 0;
int max_frequencyx, max_frequencyy, max_frequencyz;
float prevvarx, prevvary, prevvarz;
double varx = 0, vary = 0, varz = 0;
float frequencyx[20], frequencyy[20], frequencyz[20];
float datax[20], datay[20], dataz[20];
float meanx = 0, meany = 0, meanz = 0;

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
  for (int i = 0; i < 20; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    rawxaxis = g.gyro.x - caligyrox;
    rawyaxis = g.gyro.y - caligyroy;
    rawzaxis = g.gyro.z - caligyroz;
    for (int i = 0; i < 10; i++) {
      if (abs(rawxaxis) > caligyrox && abs(roll + rawxaxis) <= 1000) {
        roll = roll + rawxaxis;
      }
      if (abs(rawyaxis) > caligyroy && abs(pitch + rawyaxis) <= 1000) {
        pitch = pitch + rawyaxis;
      }
      if (abs(rawzaxis) > caligyroz && abs(yaw + rawzaxis) <= 1000) {
        yaw = yaw + rawzaxis;
      }
    }
    if (abs(rawzaxis) > abs(caligyroz)) {
      dataz[i] = rawzaxis;
      meanz += rawzaxis;
    }
  }
  meanz /= 20;
  for (int i = 0; i < 20; i++) {
    varz += pow((dataz[i] - meanz), 2);
  }
  varz /= 19;

  if (yaw <= -950) {
    prevyaw = "finger pointing right";
  } else if (yaw > 950) {
    prevyaw = "finger pointing left";
  } else if (abs(yaw) > 0 && abs(yaw) < 900) {
    prevyaw = "finger pointing MID";
  }
  if (pitch <= -950) {
    prevpitch = "down1";
  } else if (pitch > 950) {
    prevpitch = "UP";
  } else if (abs(pitch) > 0 && abs(pitch) < 900) {
    prevpitch = "MID";
  }
  if (roll <= -940) {
    prevroll = "rigth";
  } else if (roll > 945) {
    prevroll = "left";
  } else if (abs(roll) > 0 && abs(roll) < 900) {
    prevroll = "MID";
  }
  Serial.print("pitch = ");
  Serial.print(prevpitch);
  Serial.print("\t");
  Serial.print("roll = ");
  Serial.print(prevroll);
  Serial.print("\t");
  Serial.print("yaw = ");
  Serial.print(prevyaw);
  Serial.print("\t");
  Serial.print("\t");
  if (varz > 0.07 && varz != prevvarz && abs(varz - prevvarz) > 0.05) {
    Serial.println("Z = shake");
  } else {
    Serial.println("Z = STILL");
  }
  prevvarz = varz;

  delay(10);
}