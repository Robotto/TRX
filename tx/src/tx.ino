#include <VirtualWire.h>

//TODO: Use decimal point in seven segment to indicate acitvation?


//                              A,B,C,D,E,F,G
const int seven_segment_pins[]={1,2,3,4,5,6,7};

int pwm_led = 3;
int blink_led =13;
int TX_pin = 2;

const char *msg = "hello";

const char *msg_gr_1 = "GR1";
const char *msg_gr_2 = "GR2";
const char *msg_gr_3 = "GR3";
const char *msg_gr_4 = "GR4";
const char *msg_gr_5 = "GR5";
const char *msg_gr_6 = "GR6";
const char *msg_gr_7 = "GR7";
const char *msg_gr_8 = "GR8";

const char *msg_off = "ALL_OFF";


void setup() {

  for(int i = 0; i<8; i++) pinMode(seven_segment_pins[i],OUTPUT);

  pinMode(pwm_led, OUTPUT);

   // Initialise the IO and ISR
   vw_set_tx_pin(TX_pin);
   //vw_set_ptt_inverted(true); // Required for DR3100
   vw_setup(2000);  // Bits per sec



}

void loop() {


    digitalWrite(blink_led, true); // Flash a light to show transmitting

    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx(); // Wait until the whole message is gone
    digitalWrite(blink_led, false);

    pulse(1,20);


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