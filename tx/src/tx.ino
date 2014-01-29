#include <VirtualWire.h>

//TODO: Use decimal point in seven segment to indicate acitvation?


//                              A,B,C,D,E,F,G
const int seven_segment_pins[]={2,3,4,5,6,7,8};
//                             DP
const int seven_segment_DP_pin=9;
const int btn_left = A0;
const int btn_middle  = A1;
const int btn_right = A2;


//const int pwm_led = 3;
const int blink_led =13;
const int TX_pin = 10;

const char *msg = "hello";

const char *msg_gr_1 = "GR1";
const char *msg_gr_2 = "GR2";
const char *msg_gr_3 = "GR3";
const char *msg_gr_4 = "GR4";
const char *msg_gr_5 = "GR5";
const char *msg_gr_6 = "GR6";
const char *msg_gr_7 = "GR7";
const char *msg_gr_8 = "GR8";

const char *msg_off = "ON";
const char *msg_kill = "ALL_OFF";

int active_group=1;

void setup() {

  for(int i = 0; i<8; i++) pinMode(seven_segment_pins[i],OUTPUT);

  pinMode(seven_segment_DP_pin, OUTPUT);

  pinMode(12, OUTPUT); //DEBUG - GROUND THE COMMON BTN WIRE
  digitalWrite(12, LOW);

  pinMode(btn_right, INPUT_PULLUP);
  pinMode(btn_middle, INPUT_PULLUP);
  pinMode(btn_left, INPUT_PULLUP);

   // Initialise the IO and ISR
   vw_set_tx_pin(TX_pin);
   //vw_set_ptt_inverted(true); // Required for DR3100
   vw_setup(2000);  // Bits per sec



}

void loop() {


    read_encoder(); //changes active group

    read_buttons(); //fires at group

    //handle_tx(); //sends relevant string
    //pulse(1,2);

    //if (active_group<1) active_group=1;
    //if (active_group>8) active_group=8;
    seven_segment_write_number(active_group);
    if(active_group%2) digitalWrite(seven_segment_DP_pin,HIGH);
    else digitalWrite(seven_segment_DP_pin,LOW);

    //delay(1000);
   // active_group++;


}

void read_encoder()
{

}

void read_buttons()
{

}

void handle_tx()
{
    digitalWrite(blink_led, true); // Flash a light to show transmitting
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx(); // Wait until the whole message is gone
    digitalWrite(blink_led, false);
}


/*
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
*/

void seven_segment_write_number(int decimal)
{
    for(int i = 0; i<8; i++) digitalWrite(seven_segment_pins[i],0b00000001&(decimal_to_seven_seg(decimal)>>i));
}

unsigned char decimal_to_seven_seg(int decimal)
{   //                        CC          CA
    if (decimal==0) return 0b0111111;//0b1000000;
    if (decimal==1) return 0b0000110;//0b1111001;
    if (decimal==2) return 0b1011011;//0b0100100;
    if (decimal==3) return 0b1001111;//0b0110000;
    if (decimal==4) return 0b1100110;//0b0011001;
    if (decimal==5) return 0b1101101;//0b0010010;
    if (decimal==6) return 0b1111101;//0b0000010;
    if (decimal==7) return 0b0000111;//0b1111000;
    if (decimal==8) return 0b1111111;//0b0000000;
    if (decimal==9) return 0b1100111;//0b0011000;
    if (decimal==10) return 0b1110111;//0b0001000;
    if (decimal==11) return 0b1111100;//0b0000011;
    if (decimal==12) return 0b0111001;//0b1000110;
    if (decimal==13) return 0b1011110;//0b0100001;
    if (decimal==14) return 0b1111001;//0b0000110;
    if (decimal==15) return 0b1110001;//0b0001110;
    else return 0b0111110;//0b1000001; //'U' for undefined
}