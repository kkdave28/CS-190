#include <Arduino.h>
#include <BTLE.h>
#include <RF24.h>
#include <SparkFunLSM6DS3.h>
#include <Wire.h>
#include <SPI.h>

LSM6DS3 SensorTwo( I2C_MODE, 0x6B );
RF24 radio(9,10);
BTLE btle(&radio);
#define DEBUG 0
//LSM6DS3Core myIMU( SPI_MODE, 10 );
double z_high = 0.0;
unsigned long calibrate_timer = 0;
float steps_taken = 0.0;
void calibrate()
{
	double temp_z = 0.0;
	if((temp_z =SensorTwo.readFloatAccelZ()) > z_high)
	{
		z_high = temp_z;
	}
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  btle.begin("Jesus");
  delay(1000); //relax...
  Serial.println("Processor came out of reset.\n");
  
  //Call .begin() to configure the IMUs

  if( SensorTwo.begin() != 0 )
  {
	  Serial.println("Problem starting the sensor at 0x6B.");
  }
  else
  {
	  Serial.println("Sensor at 0x6B started.");
  }
  Serial.println("Calibrating in 5 seconds.");
  delay(5000);
  calibrate_timer = millis()+ 10000;
  while(calibrate_timer > millis())
  {
	  calibrate();
  }
  
}


void loop()
{
  //Get all parameters

  //Serial.println(SensorTwo.readFloatAccelZ(), 4);
  double temp_z = SensorTwo.readFloatAccelZ();
  
  if(temp_z > (z_high*0.8))
  {
	  steps_taken+=1;
	  Serial.print("Steps taken = ");
	  Serial.println(steps_taken);
  }
  nrf_service_data buf;
  buf.service_uuid = NRF_TEMPERATURE_SERVICE_UUID;
  buf.value = BTLE::to_nRF_Float(steps_taken);
  if(!btle.advertise(0x16, &buf, sizeof(buf))) 
  {
  	Serial.println("BTLE advertisement failure");

  }
  btle.hopChannel();
  
  #if DEBUG
  Serial.print("Current z = ");
  Serial.println(temp_z);
  Serial.print("Z high = ");
  Serial.println(z_high);
  Serial.print("SensorTwo Bus Errors Reported:\n");
  Serial.print(" All '1's = ");
  Serial.println(SensorTwo.allOnesCounter);
  Serial.print(" Non-success = ");
  Serial.println(SensorTwo.nonSuccessCounter);
  #endif
  delay(1000);
}
