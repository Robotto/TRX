#include <Arduino.h>
#include <VirtualWire.h>
void setup();
void loop();
void pulse(unsigned int pulses, unsigned int delay_time);
#line 1 "src/rx.ino"
//#include <VirtualWire.h>

int pwm_led = 3;
int blink_led =13;
int RX_pin = 6;

String rx_string="";

void setup() {

    //Serial.begin(9600);

    pinMode(pwm_led, OUTPUT);
    pinMode(RX_pin, INPUT);

    // Initialise the IO and ISR
    vw_set_rx_pin(RX_pin);
    vw_setup(2000);  // Bits per sec
    vw_rx_start();       // Start the receiver PLL running

    pulse(1,1); //hello world

}

void loop() {

    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
        int i;

        rx_string="";

        for (i = 0; i < buflen; i++) rx_string+=String(char(buf[i]));

        if(rx_string=="hello") pulse(1,1);

/*
        Serial.print("RX: ");
        Serial.print(rx_string);
        Serial.println("<--- EOL ");
*/
    }
}

void pulse(unsigned int pulses, unsigned int delay_time)
{
    int val;
    while(pulses)
    {
        for (val=0; val<255; val++) {analogWrite(pwm_led, val); delay(delay_time);}
        for (val=255; val>=0; val--) {analogWrite(pwm_led, val); delay(delay_time);}
        pulses--;
    }
}

