
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
Adafruit_MPU6050 mpu;
float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int RateCalibrationNumber;
float AccX, AccY, AccZ;
float AngleRoll, AnglePitch, AngleYaw;
uint32_t LoopTimer;
float KalmanAngleRoll = 0, KalmanUncertaintyAngleRoll = 2 * 2;
float KalmanAnglePitch = 0, KalmanUncertaintyAnglePitch = 2 * 2;
float KalmanAngleYaw = 0, KalmanUncertaintyAngleYaw = 2 * 2;
float Kalman1DOutput[] = { 0, 0 };
void kalman_1d(float &KalmanState, float &KalmanUncertainty, float KalmanInput, float KalmanMeasurement) {
  KalmanState = KalmanState + 0.004 * KalmanInput;
  KalmanUncertainty = KalmanUncertainty + 0.004 * 0.004 * 4 * 4;
  float KalmanGain = KalmanUncertainty * 1 / (1 * KalmanUncertainty + 3 * 3);
  KalmanState = KalmanState + KalmanGain * (KalmanMeasurement - KalmanState);
  KalmanUncertainty = (1 - KalmanGain) * KalmanUncertainty;
  Kalman1DOutput[0] = KalmanState;
  Kalman1DOutput[1] = KalmanUncertainty;
}
void gyro_signals(void) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  int16_t AccXLSB = a.acceleration.x;
  int16_t AccYLSB = a.acceleration.y;
  int16_t AccZLSB = a.acceleration.z;

  int16_t GyroX = g.gyro.x;
  int16_t GyroY = g.gyro.y;
  int16_t GyroZ = g.gyro.z;
  RateRoll = (float)GyroX / 65.5;
  RatePitch = (float)GyroY / 65.5;
  RateYaw = (float)GyroZ / 65.5;
  AccX = (float)AccXLSB / 4096;
  AccY = (float)AccYLSB / 4096;
  AccZ = (float)AccZLSB / 4096;
  // Calculate AngleRoll with safety checks
  float denominatorRoll = sqrt(AccX * AccX + AccZ * AccZ);
  if (denominatorRoll != 0) {
    AngleRoll = atan(AccY / denominatorRoll) * (180 / 3.142);  // Convert to degrees
  } else {
    AngleRoll = 90;
    Serial.println("Warning: Denominator for AngleRoll calculation is zero.");
  }
  float denominatorPitch = sqrt(AccY * AccY + AccZ * AccZ);
  if (denominatorPitch != 0) {
    AnglePitch = -atan(AccX / denominatorPitch) * (180 / 3.142);  // Convert to degrees
  } else {
    AnglePitch = 90;  // Default value if denominator is zero
    Serial.println("Warning: Denominator for AnglePitch calculation is zero.");
  }
  AngleYaw += RateYaw * 0.004;
}
void setup() {
  Serial.begin(57600);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  } else {
    Serial.println("MPU6050 Found!");
  }
  for (RateCalibrationNumber = 0; RateCalibrationNumber < 2000; RateCalibrationNumber++) {
    gyro_signals();
    RateCalibrationRoll += RateRoll;
    RateCalibrationPitch += RatePitch;
    RateCalibrationYaw += RateYaw;
    delay(1);
  }
  RateCalibrationRoll /= 2000;
  RateCalibrationPitch /= 2000;
  RateCalibrationYaw /= 2000;
  LoopTimer = micros();
}
void loop() {
  gyro_signals();
  RateRoll -= RateCalibrationRoll;
  RatePitch -= RateCalibrationPitch;
  RateYaw -= RateCalibrationYaw;
  kalman_1d(KalmanAngleRoll, KalmanUncertaintyAngleRoll, RateRoll, AngleRoll);
  KalmanAngleRoll = Kalman1DOutput[0];
  KalmanUncertaintyAngleRoll = Kalman1DOutput[1];
  kalman_1d(KalmanAnglePitch, KalmanUncertaintyAnglePitch, RatePitch, AnglePitch);
  KalmanAnglePitch = Kalman1DOutput[0];
  KalmanUncertaintyAnglePitch = Kalman1DOutput[1];
  // Apply Kalman filter for Yaw
  kalman_1d(KalmanAngleYaw, KalmanUncertaintyAngleYaw, RateYaw, AngleYaw);
  KalmanAngleYaw = Kalman1DOutput[0];
  KalmanUncertaintyAngleYaw = Kalman1DOutput[1];
  Serial.println("");
  Serial.print("Roll Angle [°] ");
  Serial.print(KalmanAngleRoll);
  Serial.print(" Pitch Angle [°] ");
  Serial.print(KalmanAnglePitch);
  Serial.print(" Yaw Angle [°] ");  // Print Yaw angle
  Serial.println(KalmanAngleYaw);

  while (micros() - LoopTimer < 4000)
    ;
  LoopTimer = micros();
}