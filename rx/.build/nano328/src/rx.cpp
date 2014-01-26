#include <Arduino.h>
#include <VirtualWire.h>
void setup();
void loop();
void pulse(unsigned int pulses, unsigned int delay_time);
void seven_segment_write(int decimal);
unsigned char decimal_to_seven_seg(int decimal);
#line 1 "src/rx.ino"
//#include <VirtualWire.h>

//                              A,B,C,D,E,F,G
const int seven_segment_pins[]={1,2,3,4,5,6,7};

int pwm_led = 3;
int blink_led =13;
int TX_pin = 2;
String rx_string="";

boolean dir=true;


void setup() {

    //Serial.begin(9600);

    pinMode(pwm_led, OUTPUT);

   // Initialise the IO and ISR
    vw_set_rx_pin(TX_pin);
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

        //digitalWrite(13, true); // Flash a light to show received good message
        // Message with a good checksum received, dump it.
        //Serial.print("Got: ");

        for (i = 0; i < buflen; i++) rx_string+=String(char(buf[i]));


        if(rx_string=="hello") pulse(1,1);

        //if(rx_string!="") pulse(1,1);

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

void seven_segment_write(int decimal)
{
    for(int i = 0; i<8; i++) digitalWrite(seven_segment_pins[i],0b00000001&(decimal_to_seven_seg(decimal)>>i));
}

unsigned char decimal_to_seven_seg(int decimal) //common anode
{
    if (decimal==0) return 0b1000000;
    if (decimal==1) return 0b1111001;
    if (decimal==2) return 0b0100100;
    if (decimal==3) return 0b0110000;
    if (decimal==4) return 0b0011001;
    if (decimal==5) return 0b0010010;
    if (decimal==6) return 0b0000010;
    if (decimal==7) return 0b1111000;
    if (decimal==8) return 0b0000000;
    if (decimal==9) return 0b0011000;
    if (decimal==10) return 0b0001000;
    if (decimal==11) return 0b0000011;
    if (decimal==12) return 0b1000110;
    if (decimal==13) return 0b0100001;
    if (decimal==14) return 0b0000110;
    if (decimal==15) return 0b0001110;
    else return 0b1000001; //'U' for undefined
}