#include <Arduino.h>
#include "BLEDevice.h"
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <Wire.h>

static BLEUUID serviceUUID("63462a4a-c28c-4ffd-87a4-2d23a1c72581");
static BLEUUID charUUID("70bc767e-7a1a-4304-81ed-14b9af54f7bd");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic = nullptr;
static BLEAddress deviceAddress("a8:10:87:22:2e:cb");

uint8_t statusFlags;
float accumulatedDose;
float doseRate;
uint16_t pulseCount;
int8_t batteryCharge;
int8_t temperature;

JsonDocument masage;
String json;

#define I2C_ADDRESS 0x04


// Callback для обработки уведомлений
static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.write(pData, length);
    Serial.println();

    // Проверяем, что данные имеют достаточную длину для чтения
    if (length >= 13)
    {
        statusFlags = pData[0];
        accumulatedDose = *((float *)(pData + 1));
        doseRate = *((float *)(pData + 5));
        pulseCount = *((uint16_t *)(pData + 9));
        batteryCharge = (int8_t)pData[11];
        temperature = (int8_t)pData[12];

        // Выводим интерпретированные значения в Serial
        Serial.print("Status Flags: ");
        Serial.println(statusFlags);
        Serial.print("Accumulated Dose (mSv): ");
        Serial.println(accumulatedDose, 4); // Увеличиваем точность до 4 знаков после запятой
        Serial.print("Dose Rate (μSv/h): ");
        Serial.println(doseRate, 4); // Увеличиваем точность до 4 знаков после запятой
        Serial.print("Pulse Count: ");
        Serial.println(pulseCount);
        Serial.print("Battery Charge (%): ");
        Serial.println(batteryCharge);
        Serial.print("Temperature (°C): ");
        Serial.println(temperature);

        // Создаем JSON объект
        // masage["accum_dose"] = accumulatedDose;
        // masage["dose_rate"] = doseRate;
        masage["s"] = millis() / 1000ul;
        masage["p"] = pulseCount;
        // masage["t"] = temperature;
        // masage["b"] = batteryCharge;

        // Конвертируем JSON объект в строку
    }
}

// Класс для клиентских обратных вызовов
class MyClientCallback : public BLEClientCallbacks
{
public:
    void onConnect(BLEClient *pclient)
    {
        Serial.println("Connected to server.");
    }

    void onDisconnect(BLEClient *pclient)
    {
        connected = false;
        Serial.println("Disconnected from server.");
    }
};

void onRequest()
{
    char charBuf[32];
    serializeJson(masage, json);
    json.toCharArray(charBuf, 32);
    Wire.write(charBuf);
    Serial.println(json);
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("");

    doConnect = true;

    Wire.begin(I2C_ADDRESS);
    Wire.onRequest(onRequest);
    delay(1000);
}

bool connectToServer()
{
    Serial.print("Forming a connection to ");
    Serial.println(deviceAddress.toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    if (!pClient->connect(deviceAddress))
    {
        Serial.println("Failed to connect to server.");
        return false;
    }
    Serial.println(" - Connected to server");
    pClient->setMTU(517);

    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println(" - Found our service");

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println(" - Found our characteristic");

    if (pRemoteCharacteristic->canRead())
    {
        std::string value = pRemoteCharacteristic->readValue();
        Serial.print("The characteristic value was: ");
        Serial.println(value.c_str());
    }

    if (pRemoteCharacteristic->canNotify())
    {
        pRemoteCharacteristic->registerForNotify(notifyCallback);
    }

    connected = true;
    return true;
}

void loop()
{
    if (doConnect)
    {
        if (connectToServer())
        {
            Serial.println("We are now connected to the BLE Server.");
        }
        else
        {
            Serial.println("Failed to connect to the server.");
        }
        doConnect = false;
    }

    if (connected)
    {
        String newValue = "Time since boot: " + String(millis() / 1000);
        Serial.println("Setting new characteristic value to \"" + newValue + "\"");

        pRemoteCharacteristic->writeValue((uint8_t *)newValue.c_str(), newValue.length());
    }

    delay(1000);
}
