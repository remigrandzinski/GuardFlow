#include <Arduino.h>
#include <IRremote.h>

const unsigned int MOTION_SENSOR_PIN = 2;
const unsigned int LED_RESPONSE_PIN = 13;
const unsigned int SOLENOID_VALVE_PIN = 4;
const unsigned int IR_RECIEVER_PIN = 3;

IRrecv irSensor(IR_RECIEVER_PIN);
decode_results *irSignal;

unsigned int lastCommandTicks = 0;

unsigned int flowOnTicks = 0;
unsigned int flowDelayTicks = 0;

unsigned int delayTime = 10;

unsigned int blinkCycles = 0;
unsigned int ledTicks = 0;

boolean ledState = LOW;

boolean isPaused = false;

void turnOnFlow(){
    digitalWrite(SOLENOID_VALVE_PIN, HIGH);
}

void turnOffFlow(){
    digitalWrite(SOLENOID_VALVE_PIN, LOW);
}

void updateFlow(){
    if(flowOnTicks > 0){
        flowOnTicks--;
        turnOnFlow();
    }else{
        turnOffFlow();
    }
}

void updateLED(){

    if(ledTicks > 0){

        if(ledTicks % (250 / delayTime) == 0){
            ledState = !ledState;
        }

        ledTicks--;
    }else{

        if(blinkCycles > 0){
            blinkCycles--;
            ledTicks = 500 / delayTime;
        }

        ledState = LOW;
    }

    digitalWrite(LED_RESPONSE_PIN, ledState);
}

void blinkLED(int i){
    blinkCycles = i;
}

void receiveIR(){

    lastCommandTicks++;

    if(irSensor.decode(irSignal)){

        unsigned int val = irSignal->value;
        Serial.println(val);

        if(lastCommandTicks > (3000 / delayTime)){
            unsigned int val = irSignal->value;

            if(val == 16575){

                lastCommandTicks = 0;

                isPaused = !isPaused;

                if(isPaused){
                    blinkLED(3);
                }else{
                    blinkLED(2);
                }

                Serial.println("On/off");

            }else if (val == 5865)
            {
                lastCommandTicks = 0;

                flowOnTicks = 5000 / delayTime;
                blinkLED(1);
                Serial.println("Enter");
            }
            
        }

        irSensor.resume();
    }
}

boolean detectPresence(){
    return !digitalRead(MOTION_SENSOR_PIN);
}

void setup(){
    Serial.begin(9600);
    irSensor.enableIRIn();
    pinMode(MOTION_SENSOR_PIN, INPUT);
    pinMode(SOLENOID_VALVE_PIN, OUTPUT);
    pinMode(LED_RESPONSE_PIN, OUTPUT);
    digitalWrite(SOLENOID_VALVE_PIN, LOW);

    for (int i = 0; i < (60000 / 200); i++)
    {
        digitalWrite(LED_RESPONSE_PIN, HIGH);
        delay(100);
        digitalWrite(LED_RESPONSE_PIN, LOW);
        delay(100);
    }
    
    digitalWrite(LED_RESPONSE_PIN, LOW);

}

void loop(){
    receiveIR();
    updateLED();
    updateFlow();

    if(flowDelayTicks > 0){
        flowDelayTicks--;
    }

    if(!isPaused && detectPresence() && flowDelayTicks == 0){
        flowOnTicks = 3000 / delayTime;
        flowDelayTicks = 5000 / delayTime;
    }

    Serial.println(detectPresence());

    delay(delayTime);
}