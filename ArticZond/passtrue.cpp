
#include <Arduino.h>
#include <Wire.h>
void setup()
{
  // Инициализация стандартного Serial для связи с компьютером
  Serial.begin(9600);

  // Инициализация Serial1 для связи с модулем LoRa E32 Ebyte
  Serial1.begin(9600);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, HIGH); // Логическая единица
  digitalWrite(9, HIGH); // Логическая единица

  delay(2000);
  // Сообщение об успешном запуске
  Serial.println("System initialized. Type commands to communicate with LoRa module.");
}

void loop()
{
  // Если есть данные в стандартном Serial (от компьютера)
  if (Serial.available() > 0)
  {
    // Читаем байт из Serial
    char incomingByte = Serial.read();

    // Отправляем прочитанный байт на Serial1 (модуль LoRa)
    Serial1.write(incomingByte);
  }

  // Если есть данные в Serial1 (от модуля LoRa)
  if (Serial1.available() > 0)
  {
    // Читаем байт из Serial1
    char incomingByte = Serial1.read();

    // Отправляем прочитанный байт на стандартный Serial (компьютер)
    Serial.write(incomingByte);
  }
}