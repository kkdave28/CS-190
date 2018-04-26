#include <Arduino.h>
#include <Wire.h>
#define BUTTON_PIN 3
#define BUZZER_PIN 4
#define RED_LIGHT 10
#define YELLOW_LIGHT 9
#define GREEN_LIGHT 8

#define RED_STATE 0
#define RED_YELLOW_STATE 1
#define YELLOW_STATE 2
#define GREEN_STATE 3

#define RED_TIMER 10000
#define YELLOW_RED_YELLOW_TIMER 2000
#define MIN_GREEN_TIMER 5000

#define GREEN_BUZZER_ON 500
#define GREEN_BUZZER_OFF 1500

#define RED_BUZZER_ON 250
#define RED_BUZZER_OFF 250

int tl_state;
unsigned char from_green;
unsigned char buzzer_state;
unsigned long tl_timer;
unsigned long buzzer_timer;
void setup() 
{
    pinMode(BUTTON_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RED_LIGHT, OUTPUT);
    pinMode(YELLOW_LIGHT, OUTPUT);
    pinMode(GREEN_LIGHT, OUTPUT);
    tl_state = RED_STATE;
    tl_timer = millis() + RED_TIMER;
    buzzer_timer = millis() + RED_BUZZER_ON;
    from_green = 0;
    buzzer_state = 0;
    // put your setup code here, to run once:
}
void turn_all_leds_on()
{
    digitalWrite(RED_LIGHT, HIGH);
    digitalWrite(YELLOW_LIGHT, HIGH);
    digitalWrite(GREEN_LIGHT, HIGH);
}
void turn_all_leds_off()
{
    digitalWrite(RED_LIGHT, LOW);
    digitalWrite(YELLOW_LIGHT, LOW);
    digitalWrite(GREEN_LIGHT, LOW);    
}
void loop() 
{
    switch(tl_state)
    {
        case RED_STATE:
        {
            digitalWrite(RED_LIGHT, HIGH);
            if(buzzer_timer > millis() && buzzer_state == 0)
            {
              digitalWrite(BUZZER_PIN, HIGH);  
            }
            else if(buzzer_timer > millis() && buzzer_state == 1)
            {
                digitalWrite(BUZZER_PIN, LOW);
            }
            //digitalWrite(BUZZER_PIN, HIGH);
            if(millis() > buzzer_timer)
            {
                if(buzzer_state == 0)
                {
                    buzzer_state = 1;
                }
                else
                {
                    buzzer_state = 0;
                }
                buzzer_timer = millis()+ RED_BUZZER_ON;
            }
            if(millis() > tl_timer)
            {
                digitalWrite(RED_LIGHT, LOW);
                tl_timer = millis() + YELLOW_RED_YELLOW_TIMER;
                tl_state = RED_YELLOW_STATE;
                digitalWrite(BUZZER_PIN, LOW);
                buzzer_state = 0;
            }
            break;
        }
        case RED_YELLOW_STATE:
        {
            digitalWrite(RED_LIGHT, HIGH);
            digitalWrite(YELLOW_LIGHT, HIGH);
            if(millis() > tl_timer)
            {
                digitalWrite(RED_LIGHT, LOW);
                digitalWrite(YELLOW_LIGHT, LOW);
                tl_timer = millis()+ YELLOW_RED_YELLOW_TIMER;
                tl_state = YELLOW_STATE;
            }
            break;
        }
        case YELLOW_STATE:
        {
            digitalWrite(YELLOW_LIGHT, HIGH);
            if(millis() > tl_timer)
            {
                digitalWrite(YELLOW_LIGHT, LOW);
                
                if(from_green == 0)
                {
                    tl_timer = millis()+ MIN_GREEN_TIMER;
                    tl_state = GREEN_STATE;
                }
                else
                {
                    tl_timer = millis() + RED_TIMER;
                    tl_state = RED_STATE;
                    buzzer_timer += millis()+ 250;
                    from_green = 0;
                }
            }
            break;
        }
        case GREEN_STATE:
        {
            digitalWrite(GREEN_LIGHT, HIGH);
            if(digitalRead(BUTTON_PIN))
            {
                tl_timer = millis() + MIN_GREEN_TIMER;
            }
            if(millis() > tl_timer)
            {
                digitalWrite(GREEN_LIGHT,LOW);
                tl_timer = millis()+ YELLOW_RED_YELLOW_TIMER;
                from_green = 1;
                tl_state = YELLOW_STATE;
            }
        }
    }
    // if(digitalRead(BUTTON_PIN))
    // {
    //     turn_all_leds_on();
    // }
    // else
    // {
    //     turn_all_leds_off();
    // }
    // delay(100);
    // put your main code here, to run repeatedly:
}