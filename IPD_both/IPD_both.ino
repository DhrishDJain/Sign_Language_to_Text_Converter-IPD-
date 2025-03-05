

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "LittleFS.h"

#include <TFT_eSPI.h>  // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Arduino_JSON.h>
// Replace with your network credentials
const char *ssid = "MATAJI";
const char *password = "anshibai@1";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;
float dgXAxis = 0;
float dgYAxis = 0;
float dgZAxis = 0;

// Reset threshold values
float thXAxis = 0;
float thYAxis = 0;
float thZAxis = 0;
float sumX = 0;
float sumY = 0;
float sumZ = 0;
float sigmaX = 0;
float sigmaY = 0;
float sigmaZ = 0;
// Timer variables
unsigned long lastTime = 0;
unsigned long lastTimeTemperature = 0;
unsigned long lastTimeAcc = 0;
unsigned long lastTimeflex = 0;
unsigned long gyroDelay = 10;
unsigned long flexDelay = 500;
unsigned long temperatureDelay = 1000;
unsigned long accelerometerDelay = 200;

// Create a sensor object
Adafruit_MPU6050 mpu;

sensors_event_t a, g, temp;

float gyroX, gyroY, gyroZ;
int f1_temp, f2_temp, f3_temp;
float accX, accY, accZ;
float temperature;
String f1, f2, f3;
String temprature, accelometre, gyro, palmx, palmy, palmz;
bool getread = false;
//Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;
#define prev 25
#define ok 33
#define next 32
#define back 34
#define TFT_CS 13
#define TFT_RST 12
#define TFT_DC 14
#define TFT_MOSI 27
#define TFT_SCLK 26

char *selectedmode = "letter";
uint16_t okpressedcount = 0;
TFT_eSPI tft = TFT_eSPI();
void printtext(String text, uint16_t color, int x, int y, float s) {
  tft.setCursor(x, y);
  tft.setTextSize(s);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void fillrect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color1) {
  tft.fillRect(x0, y0, w, h, color1);
}

void select() {
  Serial.println("select run");
  printtext("Select   Mode", TFT_YELLOW, 5, 5, 3);
}

void letterselected() {
  fillrect(0, 62, tft.width(), 20, TFT_WHITE);
  printtext("Letter", TFT_BLACK, 8, 65, 2);
  fillrect(0, 87, tft.width(), 20, TFT_BLACK);
  printtext("Word", TFT_WHITE, 8, 90, 2);
  selectedmode = "letter";
}
void wordselected() {
  fillrect(0, 62, tft.width(), 20, TFT_BLACK);
  printtext("Letter", TFT_WHITE, 8, 65, 2);
  fillrect(0, 87, tft.width(), 20, TFT_WHITE);
  printtext("Word", TFT_BLACK, 8, 90, 2);
  selectedmode = "word";
}

void lettermode() {
  printtext("Letter", TFT_BLUE, 5, 6, 2);
  printtext("Mode", TFT_BLUE, 78, 6, 2);
  tft.drawLine(0, 30, tft.width(), 30, TFT_BLUE);
  printtext("Press ok to go back", TFT_BLUE, 3, tft.height() - 15, 1);
}

void wordmode() {
  printtext("Word Mode", TFT_BLUE, 5, 5, 2);
  tft.drawLine(0, 30, tft.width(), 30, TFT_BLUE);
  printtext("Press ok to go back", TFT_BLUE, 3, tft.height() - 15, 1);
  getread = true;
}
void initMPU() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    // while (1) {
    //   delay(10);
    //   Serial.println("finding mpu6050");
    // }
  } else {
    Serial.println("MPU6050 Found!");
    Serial.println("Calibrating __________");
    mpu.getEvent(&a, &g, &temp);


    // Read n-100
    for (uint8_t i = 0; i < 100; ++i) {
      sumX += g.gyro.x;
      sumY += g.gyro.y;
      sumZ += g.gyro.z;

      sigmaX += g.gyro.x * g.gyro.x;
      sigmaY += g.gyro.y * g.gyro.y;
      sigmaZ += g.gyro.z * g.gyro.z;

      delay(5);
    }

    // Calculate delta vectors
    dgXAxis = sumX / 100;
    dgYAxis = sumY / 100;
    dgZAxis = sumZ / 100;

    // Calculate threshold vectors
    thXAxis = sqrt((sigmaX / 100) - (dgXAxis * dgXAxis));
    thYAxis = sqrt((sigmaY / 100) - (dgYAxis * dgYAxis));
    thZAxis = sqrt((sigmaZ / 100) - (dgZAxis * dgZAxis));
  }
}

void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
}

String getGyroReadings() {
  mpu.getEvent(&a, &g, &temp);
  float gyroX_temp = g.gyro.x;
  if (abs(gyroX_temp) > gyroXerror) {
    gyroX += gyroX_temp / 50.00;
  }

  float gyroY_temp = g.gyro.y;
  if (abs(gyroY_temp) > gyroYerror) {
    gyroY += gyroY_temp / 70.00;
  }

  float gyroZ_temp = g.gyro.z;
  if (abs(gyroZ_temp) > gyroZerror) {
    gyroZ += gyroZ_temp / 90.00;
  }
  readings["gyroX"] = String(gyroX);
  readings["gyroY"] = String(gyroY);
  readings["gyroZ"] = String(gyroZ);

  if (gyroX < thXAxis) {
    palmx = "Palm Facing left";
  } else if (gyroX > thXAxis) {
    palmx = "Palm Facing Rigth";
  } else if (gyroX == thXAxis) {
    palmx = "Palm in middle";
  }


  Serial.print(palmx);
  Serial.print("\t");
  Serial.println(gyroX);

  if (gyroY < thYAxis) {
    palmy = "Tilted down";
  } else if (gyroY > thYAxis) {
    palmy = "Tilted up";
  } else if (gyroY == thYAxis) {
    palmy = "Tilted middle";
  }
  Serial.print(palmy);
  Serial.print("\t");
  Serial.println(gyroY);

  if (gyroZ < thZAxis) {
    palmz = "Pointing left";
  } else if (gyroZ > thZAxis) {
    palmz = "Pointing Rigth";
  } else if (gyroZ == thZAxis) {
    palmz = "Pointing straigth";
  }
  Serial.print(palmz);
  Serial.print("\t");
  Serial.println(gyroZ);

  delay(500);
  tft.setTextWrap(true);
  printtext("gyroX : ", TFT_BLUE, 2, 40, 1);
  tft.fillRect(0, 50, tft.width(), 10, TFT_BLACK);
  tft.drawString(palmx, 3, 50, 1);

  printtext("gyroY : ", TFT_BLUE, 2, 60, 1);
  tft.fillRect(0, 70, tft.width(), 10, TFT_BLACK);
  tft.drawString(palmy, 3, 70, 1);

  printtext("gyroZ : ", TFT_BLUE, 2, 80, 1);
  tft.fillRect(0, 90, tft.width(), 10, TFT_BLACK);
  tft.drawString(palmz, 3, 90, 1);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

String getAccReadings() {
  mpu.getEvent(&a, &g, &temp);
  // Get current acceleration values
  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;
  readings["accX"] = String(accX);
  readings["accY"] = String(accY);
  readings["accZ"] = String(accZ);
  String accString = JSON.stringify(readings);
  return accString;
}

String getTemperature() {
  mpu.getEvent(&a, &g, &temp);
  temperature = temp.temperature;
  return String(temperature);
}

String getflexReadings() {
  f1_temp = analogRead(27);
  f2_temp = analogRead(2);
  f3_temp = analogRead(4);
  // Serial.println(f1_temp);
  if (f1_temp > 3000) {
    f1 = "BENDED";
  } else {
    f1 = "OPEN";
  }
  if (f2_temp > 3000) {
    f2 = "BENDED";
  } else {
    f2 = "OPEN";
  }
  if (f3_temp > 3000) {
    f3 = "BENDED";
  } else {
    f3 = "OPEN";
  }
  readings["index"] = String(f1);
  readings["middle"] = String(f2);
  readings["ring"] = String(f3);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}



void setup(void) {
  Serial.begin(115200);

  pinMode(prev, INPUT);
  pinMode(ok, INPUT);
  pinMode(next, INPUT);
  pinMode(back, INPUT);
  initWiFi();
  initLittleFS();
  initMPU();

  tft.init();

  tft.fillScreen(TFT_BLACK);

  printtext("IPD", TFT_WHITE, 40, 62, 3);
  printtext("Project", TFT_WHITE, 25, 92, 2);
  // delay(1000);
  tft.fillScreen(TFT_BLACK);
  // select();
  // letterselected();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroX = 0;
    gyroY = 0;
    gyroZ = 0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroX = 0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroY = 0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request) {
    gyroZ = 0;
    request->send(200, "text/plain", "OK");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  server.begin();
}

void loop() {
  // if (okpressedcount == 0) {
  //   if (digitalRead(prev) == HIGH) {
  //     letterselected();
  //   }
  //   if (digitalRead(next) == HIGH) {
  //     wordselected();
  //   }
  // }
  // if (digitalRead(ok) == HIGH) {
  //   okpressedcount += 1;
  //   if (selectedmode == "word") {
  //     tft.fillScreen(TFT_BLACK);
  //     wordmode();

  //   }
  //   if (selectedmode == "letter") {
  //     tft.fillScreen(TFT_BLACK);
  //     lettermode();
  //   }
  // }
  // if (digitalRead(back) == HIGH) {
  //   tft.fillScreen(TFT_BLACK);
  //   select();
  //   Serial.println("back run");
  //   getread = false;
  //   if (selectedmode == "letter") {
  //     letterselected();
  //   }
  //   if (selectedmode == "word") {
  //     wordselected();
  //   }
  //   delay(2000);
  //   okpressedcount = 0;
  // }
  //  if (getread) {
  if ((millis() - lastTime) > gyroDelay) {
    // Send Events to the Web Server with the Sensor Readings
    getGyroReadings();
    lastTime = millis();
  }
  // if ((millis() - lastTimeAcc) > accelerometerDelay) {
  //   // Send Events to the Web Server with the Sensor Readings
  //   getAccReadings();
  //   lastTimeAcc = millis();
  // }
  // if ((millis() - lastTimeTemperature) > temperatureDelay) {
  //   // Send Events to the Web Server with the Sensor Readings
  //   getTemperature();
  //   lastTimeTemperature = millis();
  // }
  // if ((millis() - lastTimeflex) > flexDelay) {
  //   // Send Events to the Web Server with the Sensor Readings
  //   getflexReadings();
  //   lastTimeflex = millis();
  // }
  // }
}