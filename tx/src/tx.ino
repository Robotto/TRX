#include <VirtualWire.h>
#define DEBOUNCE_VAL 15
#define NUMBER_OF_GROUPS 8

//TODO: Use decimal point in seven segment to indicate activation?


//                              A,B,C,D,E,F,G <- A moved from 2, B moved from 3
const int seven_segment_pins[]={10,14,4,5,6,7,8};
//                             DP
const int seven_segment_DP_pin=9;


const int ENC_A=2; //A0 <-MOVED FROM 14
const int ENC_B=15; //A1
const int ENC_BTN=3; //A2 <-MOVED FROM 10

bool ENC_HAS_MOVED = false;


unsigned long ENC_BTN_MILLIS = 0;
bool ENC_BTN_TRIGGERED = false;
bool ENC_BTN_MILLIS_STORED = false;
bool ENC_BTN_DEBOUNCED = false;

const int blink_led = 13;
const int TX_pin = 11;

typedef struct //group struct
{
  int group_number;
  char *group_message;
  bool active;
  //bool last_state;
} GROUP;

//instatitation of the group structs:
GROUP GR1;
GROUP GR2;
GROUP GR3;
GROUP GR4;
GROUP GR5;
GROUP GR6;
GROUP GR7;
GROUP GR8;

GROUP *group_map[NUMBER_OF_GROUPS]={&GR1,&GR2,&GR3,&GR4,&GR5,&GR6,&GR7,&GR8}; //array of pointers to the structs ... abstract? .. indeed!

//char *msg = "hello";

char *msg_on = "ON";
char *msg_off = "OFF";
char *msg_kill = "ALL_OFF";



/*
const char *msg_gr_1 = "GR1";
const char *msg_gr_2 = "GR2";
const char *msg_gr_3 = "GR3";
const char *msg_gr_4 = "GR4";
const char *msg_gr_5 = "GR5";
const char *msg_gr_6 = "GR6";
const char *msg_gr_7 = "GR7";
const char *msg_gr_8 = "GR8";
*/


int selected_group=1;


void setup() {

    /*-------- Memory ---------*/

    for(int i = 0;i < NUMBER_OF_GROUPS;i++)
        {
            group_map[i]->group_number=i+1;
            group_map[i]->active=LOW;
            group_map[i]->group_message="GR"; //makes group messages be "GR1" to "GR8"
            group_map[i]->group_message+=i+1;
        }

    /*-------- IO ----------*/

    //7seg:
    for(int i = 0; i<8; i++) pinMode(seven_segment_pins[i],OUTPUT);
    pinMode(seven_segment_DP_pin, OUTPUT);

    //encoder
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(ENC_BTN, INPUT_PULLUP);



    // Initialise the IO and ISR
    vw_set_tx_pin(TX_pin);
    vw_setup(2000);  // Bits per sec

    //ATTACH INTERRUPT 0 (pin 2) ON CHANGE FOR ENCODER PIN A, to call the read_encoder() function
    attachInterrupt(0, isr_read_encoder, CHANGE);

    //ATTACH INTERRUPT 1 (pin 3) ON CHANGE FOR ENCODER BUTTON, to call the read_button() function
    attachInterrupt(1, isr_read_button, LOW);

}

void loop()
{

    if(ENC_HAS_MOVED)
        {
        seven_segment_write_number(group_map[selected_group]->group_number);
        digitalWrite(seven_segment_DP_pin,group_map[selected_group]->active); //indicate whether selected group is active with 7seg decimal point.
        ENC_HAS_MOVED=false;
        }

    if(ENC_BTN_TRIGGERED) store_button_bounce_time();
    if(ENC_BTN_MILLIS_STORED) check_button_bounce();

    if (ENC_BTN_DEBOUNCED) //BUTTON PRESSED!
        {

        ENC_BTN_DEBOUNCED=false; //disarm the button debounce indicator

        handle_tx(group_map[selected_group]->group_message);

        if(group_map[selected_group]->active==true) handle_tx(msg_off);
        else handle_tx(msg_on);

        group_map[selected_group]->active = !group_map[selected_group]->active; //toogle current state.

        }

    /*if(active_group%2) digitalWrite(seven_segment_DP_pin,HIGH);
    else digitalWrite(seven_segment_DP_pin,LOW);*/

    //active_group++;
    //if(active_group>0xf) active_group=0;

}





void handle_tx(char *msg)
{
    digitalWrite(blink_led, true); // Flash a light to show transmitting
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx(); // Wait until the whole message is gone
    digitalWrite(blink_led, false);
}

void isr_read_button() //triggers on a falling edge of ENC_BTN pin
{
    ENC_BTN_TRIGGERED = true; //set to false when reacting to flag.
}

void isr_read_encoder() //triggers on change of ENC_A
{
    bool A = digitalRead(ENC_A);
    bool B = digitalRead(ENC_B);

    if(A!=B) selected_group++; //CW
    else selected_group--; //CCW

    //0-7
    if(selected_group>NUMBER_OF_GROUPS-1) selected_group=0;
    else if(selected_group<0) selected_group=NUMBER_OF_GROUPS-1;

    ENC_HAS_MOVED=true;


    //if(ENC_LastA != A) //<-Pretty much always true, since the interrupt only fires on change.. so... um.. delete?
    //        {
                /*
            if     (A == FALSE & B == TRUE) active_group++;  //CW rotation
            else if(A == TRUE & B == FALSE) active_group++;  //CW rotation
            else if(A == FALSE & B == FALSE) active_group--;  //CCW rotation
            else if(A == TRUE & B == TRUE) active_group--;  //CCW rotation
                */

                /*
            if (A==TRUE)
                {
                    if (B==FALSE) active_group++; //A=1,B=0
                    else active_group--; // A=1,B=1
                }
            else
                {
                    if (B==TRUE) active_group++; //A=0,B=1
                    else active_group--; //A=0,B=0
                }
            ENC_LastA = A;
                */
    //        }
}

//Called when the main loop reads the ENC_BTN_TRIGGERED bool as true
//ENC_BTN_TRIGGERED is set by the isr_read_button() function when a fallig edge is detected.
void store_button_bounce_time()
    {
    ENC_BTN_MILLIS=millis(); //store time of change.
    ENC_BTN_TRIGGERED=false; //reset the ISR triggered bool
    ENC_BTN_MILLIS_STORED=true; //arm the check routine
    }

//Called when the main loop reads the ENC_BTN_MILLIS_STORED bool as true,
//this causes the main loop to store current millis in ENC_BTN_MILLIS and set ENC_BTN_MILLIS_STORED to true
void check_button_bounce()
{
    if ((millis()>ENC_BTN_MILLIS+DEBOUNCE_VAL) && (digitalRead(ENC_BTN)==LOW))
        {
            ENC_BTN_DEBOUNCED=true;
            ENC_BTN_MILLIS_STORED=false; //disarm this routine
        }
    else ENC_BTN_DEBOUNCED=false;
}


void seven_segment_write_number(int decimal)
{
    for(int i = 0; i<8; i++) digitalWrite(seven_segment_pins[i],0b00000001&(decimal_to_seven_seg(decimal)>>i)); //for each pin on the 7-seg.. shift out the corresponding bit of the "decimal" byte.

}

unsigned char decimal_to_seven_seg(int decimal)
{   //                      CC          CA
    switch(decimal)
    {                   //ABCDEFG
        case 0:  return 0b0111111;  //0b1000000;
        case 1:  return 0b0000110;  //0b1111001;
        case 2:  return 0b1011011;  //0b0100100;
        case 3:  return 0b1001111;  //0b0110000;
        case 4:  return 0b1100110;  //0b0011001;
        case 5:  return 0b1101101;  //0b0010010;
        case 6:  return 0b1111101;  //0b0000010;
        case 7:  return 0b0000111;  //0b1111000;
        case 8:  return 0b1111111;  //0b0000000;
        case 9:  return 0b1100111;  //0b0011000;
        case 10: return 0b1110111;  //0b0001000;
        case 11: return 0b1111100;  //0b0000011;
        case 12: return 0b0111001;  //0b1000110;
        case 13: return 0b1011110;  //0b0100001;
        case 14: return 0b1111001;  //0b0000110;
        case 15: return 0b1110001;  //0b0001110;
        default: return 0b0111110;  //0b1000001;    //'U' for undefined
    }
    /*
    if (decimal==0)  return 0b0111111;  //0b1000000;
    if (decimal==1)  return 0b0000110;  //0b1111001;
    if (decimal==2)  return 0b1011011;  //0b0100100;
    if (decimal==3)  return 0b1001111;  //0b0110000;
    if (decimal==4)  return 0b1100110;  //0b0011001;
    if (decimal==5)  return 0b1101101;  //0b0010010;
    if (decimal==6)  return 0b1111101;  //0b0000010;
    if (decimal==7)  return 0b0000111;  //0b1111000;
    if (decimal==8)  return 0b1111111;  //0b0000000;
    if (decimal==9)  return 0b1100111;  //0b0011000;
    if (decimal==10) return 0b1110111;  //0b0001000;
    if (decimal==11) return 0b1111100;  //0b0000011;
    if (decimal==12) return 0b0111001;  //0b1000110;
    if (decimal==13) return 0b1011110;  //0b0100001;
    if (decimal==14) return 0b1111001;  //0b0000110;
    if (decimal==15) return 0b1110001;  //0b0001110;
    else return 0b0111110;      //0b1000001;    //'U' for undefined
    */
}
