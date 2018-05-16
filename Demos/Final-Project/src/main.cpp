#include <Arduino.h>
#include <Adafruit_CCS811.h>
#define TRIG 13
#define ECHO 12
/*
    Implemented the HCSRO4 Sensor
    PIN CONFIGS:
        PIN D-13 - TRIGGER
        PIN D-12 - ECHO
    Functions Used:
       void setup_UltrasonicSensor() --> initiates the pins
       unsigned long get_distance() --> returns an unsigned long which when multiplied by 0.017, gives the distance.
       void print_USData() --> prints the distance in a human readable format to the terminal.
    
    Implementing the CCS811 Sensor
    PIN CONFIGS:
        SDA [PIN D2] --> SDA On the Sensor
        SCL/SCLK[PIN D14] --> SCL On the Sensor
        USE ONLY 3.3V to power the Sensor
        Ground to Ground
        Ground the WAKE PINOUT on the Sensor.
    Functions Used:
        void setup_airqsensor() --> Sets up the air quality sensor
        void print AQSData() --> Prints the CO2 content and VOC level in a human readable format to the terminal
*/
unsigned long duration;
int distance;
Adafruit_CCS811 AirQualitySensor;
void setup_airqsensor()
{
    if(!AirQualitySensor.begin())
    {
        Serial.println("Not Detected Air Quality Sensor");
        while(1);
    }
}

void setup_UltrasonicSensor()
{
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);    
    distance = 0;
    duration =0;
}
unsigned long get_distance()
{
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    duration = pulseIn(ECHO, HIGH);
    return duration;
}
void setup() {
    setup_UltrasonicSensor();
    Serial.begin(9600);
    setup_airqsensor();
    // put your setup code here, to run once:
}
void print_USData()
{
    distance = get_distance()*0.034/2;
    Serial.print("Distance: ");
    Serial.println(distance);
}
void print_AQSData()
{
    if(AirQualitySensor.available())
    {
        if(!AirQualitySensor.readData())
        {
            Serial.print("CO2 = ");
            Serial.println(AirQualitySensor.geteCO2());
            Serial.print("VOC = ");
            Serial.println(AirQualitySensor.getTVOC());
        }
        else
        {
            Serial.println("ERROR READING AIR QUALITY SENSOR!!!");
            while(1);
        }
    }
}
void loop() {
    
    print_USData();
    print_AQSData();
    delay(1500);


    // put your main code here, to run repeatedly:
}