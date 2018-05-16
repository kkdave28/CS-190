#include <Arduino.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#define DEBUG 0
static bool messagePending = false;
static bool messageSending = true;

static char *connectionString;
static char *ssid;
static char *pass;
DHT SensorOne;
float temp,humidity;
const char * AccessPoint = "UCInet Mobile Access";
/*
IOTHOSTNAME: Demo-4.azure-devices.net
iothubowner_primary_key: HostName=Demo-4.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=BJcqThfkGPZymdErw600KfLerctL8d09cijUkSj9o4A=
device_primary_key: HostName=Demo-4.azure-devices.net;DeviceId=Sparkfun-Kush;SharedAccessKey=C0iyv6klHT6LwzXyWHjEK4I2WZ8OXWJly5dD0TSUmk8=
*/
void setup_sensor()
{
    SensorOne.setup(13);
    temp = 0.0; humidity = 0.0;
}
void setup_wifi()
{
    WiFi.begin(AccessPoint);
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Wifi connected, IP = ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC ADDR: ");
    Serial.println(WiFi.macAddress());
}
void setup() 
{
    
    Serial.begin(9600);
    setup_sensor();
    setup_wifi();
    //Serial.println("DHT Sensor Demo, hooked to pin-13");
     // put your setup code here, to run once:
}
void loop() {

    temp = SensorOne.getTemperature();
    humidity = SensorOne.getHumidity();

    #if DEBUG
    Serial.print(WiFi.macAddress());
    Serial.println("DHT Sensor:");
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity ");
    Serial.println(humidity);
    #endif
    delay(2000);
    // put your main code here, to run repeatedly:
}