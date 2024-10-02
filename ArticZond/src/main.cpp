#include <Arduino.h>

#include <TinyGPSPlus.h>
#include <Wire.h>
#include "MPU6050.h"
#include <MS5611.h>
#include <ArduinoJson.h>
#include <QMC5883LCompass.h>
#include <microDS18B20.h>

#define DS_SENSOR_AMOUNT 2

uint8_t addr[][8] = {
    {0x28, 0x61, 0x64, 0x8, 0xeb, 0xe, 0x38, 0x5e},
    {0x28, 0x23, 0xc7, 0x48, 0xa1, 0x22, 0x9, 0xef},
};

MicroDS18B20<10, DS_ADDR_MODE> sensor[DS_SENSOR_AMOUNT];

#define PERIOD 2000                   // Период отправки данных в миллисекундах
uint32_t tmr1;                        // Переменная для отслеживания времени
uint32_t tmr2;                        // Переменная для отслеживания времени
static const uint32_t GPSBaud = 9600; // Скорость передачи данных GPS модуля
TinyGPSPlus gps;                      // Объект для работы с GPS модулем
MPU6050 mpu;                          // Объект для работы с гироскопом и акселерометром
MS5611 ms5611;                        // Объект для работы с барометром
double referencePressure;             // Ссылочное давление для вычисления высоты
JsonDocument masage;                  // JSON объект для хранения данных
JsonDocument doc;                     // JSON объект для хранения данных
uint32_t count1 = 0;                  // Счетчик отправленных сообщений
uint32_t count2 = 0;                  // Счетчик отправленных сообщений
QMC5883LCompass compass;

#define I2C_ADDRESS 0x04

void setup()
{
  Serial.begin(9600);     // Инициализация порта для связи с ПК
  Serial1.begin(115200);  // Инициализация порта для связи с транслятором
  Serial2.begin(9600);    // Инициализация порта для связи с логером
  Serial3.begin(GPSBaud); // Инициализация порта для связи с GPS модулем
  // Проверка доступности гироскопа
  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("гироскоп не");
    delay(500);
  }

  // Проверка доступности барометра
  while (!ms5611.begin(MS5611_HIGH_RES))
  {
    Serial.println("баро не");
    delay(500);
  }

  referencePressure = ms5611.readPressure();

  mpu.calibrateGyro(); // Калибровка гироскопа

  mpu.setThreshold(3); // Установка порога для гироскопа

  compass.init(); // инициализация компаса

  for (int i = 0; i < DS_SENSOR_AMOUNT; i++)
  {
    sensor[i].setAddress(addr[i]);
  }
}

void dozimetr()
{
  Wire.requestFrom(I2C_ADDRESS, 32);

  String jsonStr;
  while (Wire.available())
  {
    char c = Wire.read();
    jsonStr += c;
  }

  if (jsonStr.length() > 0)
  {
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (!error)
    {
      masage["s_s"] = doc["s"];
      masage["p_c"] = doc["p"];
      // masage["temperature"] = doc["t"];
      // masage["battery"] = doc["b"];
    }
  }
}

// Функция для считывания данных с датчика температуры аккумулятора
void bat_tmp()
{
  // читаем прошлое значение
  if (sensor[0].readTemp())
    masage["t_bt"] = sensor[0].getTemp();
  else
    Serial.println("error");
  if (sensor[1].readTemp())
    masage["t_i"] = sensor[1].getTemp();
  else
    Serial.println("error");
  // запрашиваем новое измерение
  for (int i = 0; i < DS_SENSOR_AMOUNT; i++)
  {
    sensor[i].requestTemp();
  }
}

// Функция для считывания данных с барометра
void baro_tmp()
{
  long realPressure = ms5611.readPressure();
  masage["t_b"] = ms5611.readTemperature();
  masage["p"] = realPressure;
  masage["alt"] = ms5611.getAltitude(realPressure, referencePressure);
}
// Функция для считывания направления с компаса
void compas()
{
  compass.read();

  masage["c_x"] = compass.getX();
  masage["c_y"] = compass.getY();
  masage["c_z"] = compass.getZ();
}

// Функция для кодирования данных гироскопа и акселерометра
void gyro_acel_encode()
{
  Vector normGyro = mpu.readNormalizeGyro();
  Vector normAccel = mpu.readNormalizeAccel();

  masage["g_x"] = normGyro.XAxis;
  masage["g_y"] = normGyro.YAxis;
  masage["g_z"] = normGyro.ZAxis;
  masage["a_x"] = normAccel.XAxis;
  masage["a_y"] = normAccel.YAxis;
  masage["a_z"] = normAccel.ZAxis;
  masage["t_g"] = mpu.readTemperature();
}

// Функция для кодирования данных GPS
void gps_encode()
{
  if (gps.location.isValid())
  {
    masage["lat"] = gps.location.lat();
    masage["lng"] = gps.location.lng();
  }
  if (gps.date.isValid())
  {
    masage["d"] = gps.date.day();
    masage["mh"] = gps.date.month();
    masage["y"] = gps.date.year();
  }
  if (gps.time.isValid())
  {
    masage["h"] = gps.time.hour();
    masage["m"] = gps.time.minute();
    masage["s"] = gps.time.second();
    // masage["centisecond"] = gps.time.centisecond();
  }
  if (gps.speed.isValid())
  {
    masage["mps"] = gps.speed.mps();
    // masage["kmph"] = gps.speed.kmph();
  }
  if (gps.course.isValid())
  {
    masage["deg"] = gps.course.deg();
  }
  if (gps.altitude.isValid())
  {
    masage["g_alt"] = gps.altitude.meters();
    // masage["gps_altitude_km"] = gps.altitude.kilometers();
  }
  if (gps.satellites.isValid())
  {
    masage["s_c"] = gps.satellites.value();
  }
  if (gps.hdop.isValid())
  {
    masage["hdop"] = gps.hdop.hdop();
  }
}


static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial3.available())
      gps.encode(Serial3.read());
  } while (millis() - start < ms);
}

void loop()
{
  // Чтение данных с порта, связанного с GPS модулем
  smartDelay(1000);
  // Если прошло достаточное время для отправки данных
  if (millis() - tmr1 >= 2000)
  {
    tmr1 = millis(); // Обновление времени последней отправки

    dozimetr();
    bat_tmp();
    gps_encode();
    gyro_acel_encode(); // Кодирование данных гироскопа и акселерометра
    baro_tmp();         // Считывание данных с барометра
    compas();

    masage["c"] = count1;          // Добавление номера сообщения
    serializeJson(masage, Serial); // Отправка JSON данных на порт для связи с ПК
    Serial.println();
    serializeJson(masage, Serial2); // Отправка JSON данных на порт для связи с логером
    Serial2.println();
    count1++; // Увеличение счетчика отправленных сообщений
  }
  // Если прошло достаточное время для отправки данных
  if (millis() - tmr2 >= 5000)
  {
    tmr2 = millis(); // Обновление времени последней отправки
    dozimetr();
    bat_tmp();
    gps_encode();
    gyro_acel_encode(); // Кодирование данных гироскопа и акселерометра
    baro_tmp();         // Считывание данных с барометра
    compas();

    masage["c"] = count2;           // Добавление номера сообщения
    serializeJson(masage, Serial1); // Отправка JSON данных на порт для связи с транслятором
    Serial1.println();
    count2++; // Увеличение счетчика отправленных сообщений
  }
}
