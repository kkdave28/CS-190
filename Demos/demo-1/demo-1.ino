#include <Servo.h>
unsigned long count;

Servo myservo;
void setup() {
  // put your setup code here, to run once:
  count = 0;
  myservo.attach(3);
  Serial.begin(9600);
}

void loop() {
  //myservo.write(50);
  unsigned long init = millis();
  unsigned int light_max = 0;
  unsigned int light_min = 1024;
  unsigned int temp = 0;
  Serial.print("Starting Measurement in 2 seconds\n");
  unsigned int servo_angle = 0;
  while(millis() - init< 5000)
  {
    digitalWrite(LED_BUILTIN, HIGH); 
    temp = analogRead(A0);
    if(temp> light_max)
    {
      light_max = temp;
    }
    digitalWrite(LED_BUILTIN, LOW);
   
  }
  init = millis();
  while(millis() -init < 5000)
  {
    
    digitalWrite(LED_BUILTIN, HIGH); 
    temp = analogRead(A0);
    if(temp< light_min)
    {
      light_min = temp;
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
//  Serial.print(count);
 // myservo.write(30);
  // put your main code here, to run repeatedly:
//  delay(1000);
//  count+= 1;
char buf [32];
sprintf(buf, "max = %i, min = %i\n", light_max, light_min);
Serial.print(buf);
delay(2000);
servo_angle = light_max;
map(servo_angle, 0, 760, 0, 179);
myservo.write(servo_angle);


servo_angle = light_min;
map(servo_angle, 0, 760, 0, 179);
myservo.write(servo_angle);
//  if(count %3 ==0 && count/ 3 !=0)
//    Serial.print("Message that prints every 3 seconds\n");  
//  if(count %7 == 0 && count /7 != 0)
//    Serial.print("Message that print every 7 seconds\n");
//  if(count %15 ==0 && count /15 != 0)
//    Serial.print("Message that prints every 15 seconds\n");  
//  //delay(3000);
}
