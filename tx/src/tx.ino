#include <VirtualWire.h>
#define DEBOUNCE_VAL 15
#define NUMBER_OF_GROUPS 8

//#define DEBUG

//TODO: Use decimal point in seven segment to indicate activation?


//                              A,B,C,D,E,F,G
const int seven_segment_pins[]={10,14,4,5,6,7,8};
//                             DP
const int seven_segment_DP_pin=9;


const int ENC_A=2;
const int ENC_B=15; //A1
const int ENC_BTN=3;

bool ENC_HAS_MOVED = false;
bool cw_divider=false;
bool ccw_divider=false;


unsigned long ENC_BTN_MILLIS = 0;
bool ENC_BTN_TRIGGERED = false;
bool ENC_BTN_MILLIS_STORED = false;
bool ENC_BTN_DEBOUNCED = false;

bool countdown_btn_released=true;

const int blink_led = 13;
const int TX_pin = 11;

const int blink_switch=A4;
const int btn_pin=A2;
const int countdown_led=A3;

typedef struct //group struct
{
  int group_number;
  bool active;
  int countdown_secs;
  unsigned long trigger_time;
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

char *msg_on =      "ON!";
char *msg_off =     "OFF";
char *msg_blink=    "BLK";

char *tx_string = "GRxxxx";


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


int selected_group=0;


void setup() {

    #ifdef DEBUG
    Serial.begin(9600);
    #endif
    /*-------- Memory ---------*/

    for(int i = 0;i < NUMBER_OF_GROUPS;i++)
        {
            group_map[i]->group_number=i+1;
            group_map[i]->active=LOW;
            group_map[i]->trigger_time=0;
            group_map[i]->countdown_secs=0;
        }

    /*-------- IO ----------*/

    //7seg:
    for(int i = 0; i<7; i++) pinMode(seven_segment_pins[i],OUTPUT);
    pinMode(seven_segment_DP_pin, OUTPUT);

    //encoder
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(ENC_BTN, INPUT_PULLUP);

    //other I/O
    pinMode(blink_switch,INPUT_PULLUP);
    pinMode(btn_pin,INPUT_PULLUP);
    pinMode(countdown_led,OUTPUT);


    // Initialise the IO and ISR
    vw_set_tx_pin(TX_pin);
    vw_setup(2000);  // Bits per sec

    //ATTACH INTERRUPT 0 (pin 2) ON CHANGE FOR ENCODER PIN A, to call the read_encoder() function
    attachInterrupt(0, isr_read_encoder, CHANGE);

    //ATTACH INTERRUPT 1 (pin 3) ON CHANGE FOR ENCODER BUTTON, to call the read_button() function
    attachInterrupt(1, isr_read_button, LOW);

    seven_segment_write_number(group_map[selected_group]->group_number);
}

void loop()
{

    if(ENC_HAS_MOVED)
        {
        seven_segment_write_number(group_map[selected_group]->group_number);
        digitalWrite(seven_segment_DP_pin,group_map[selected_group]->active); //indicate whether selected group is active with 7seg decimal point.
        ENC_HAS_MOVED=false; //disarm the encoder move indicator

        #ifdef DEBUG
        Serial.print("Selected group: ");
        Serial.println( selected_group+1 );
        #endif

        }


    if(ENC_BTN_TRIGGERED) store_button_bounce_time();
    if(ENC_BTN_MILLIS_STORED) check_button_bounce();

    if (ENC_BTN_DEBOUNCED) //BUTTON PRESSED!
        {

        ENC_BTN_DEBOUNCED=false; //disarm the button debounce indicator

        //3rd char = Group ID:
        tx_string[2]=(char)group_map[selected_group]->group_number+48; //48 = ascii (char)offset

        if(group_map[selected_group]->active==true) for (int i=0;i<3;i++) tx_string[i+3]=msg_off[i];

        else if (!digitalRead(blink_switch)) for (int i=0;i<3;i++) tx_string[i+3]=msg_blink[i];
        else for (int i=0;i<3;i++) tx_string[i+3]=msg_on[i];

        //send newly constructed string:
        handle_tx(tx_string);
        handle_tx(tx_string);
        handle_tx(tx_string);

        if(group_map[selected_group]->countdown_secs) group_map[selected_group]->trigger_time=millis();

        #ifdef DEBUG
        Serial.print("TX: ");
        Serial.println(tx_string);

        Serial.print("Trigger time: ");
        Serial.println(group_map[selected_group]->trigger_time);
        Serial.print("Timeout: ");
        Serial.println(group_map[selected_group]->countdown_secs);
        #endif

         //toogle current state.
        group_map[selected_group]->active = !group_map[selected_group]->active;
        }

        unsigned long millis_for_this_loop=millis();
        for(int i=0;i<NUMBER_OF_GROUPS;i++)
            {
            if(group_map[i]->active==false) continue; //skip this group
            if(group_map[i]->countdown_secs==0) continue; // -||-
            if(group_map[i]->trigger_time==0) continue; // -||-

            if(millis_for_this_loop-(group_map[i]->countdown_secs*1000.0)>group_map[i]->trigger_time)
                    {
                        //construct tx_string:
                    tx_string[2]=(char)group_map[i]->group_number+48; //48 = ascii (char)offset
                    for (int i=0;i<3;i++) tx_string[i+3]=msg_off[i];
                        //tx:
                    handle_tx(tx_string);
                    handle_tx(tx_string);
                    handle_tx(tx_string);

                    #ifdef DEBUG
                    Serial.print("TX: ");
                    Serial.println(tx_string);
                    Serial.print("Trigger time: ");
                    Serial.println(group_map[i]->trigger_time);
                    Serial.print("Timeout: ");
                    Serial.println(group_map[i]->countdown_secs);
                    Serial.print("Timeout time: ");
                    Serial.println(millis_for_this_loop);
                    #endif

                    group_map[i]->active=false;
                    group_map[i]->trigger_time = 0;
                    }
            }


        while(!digitalRead(btn_pin))
            {
                if(countdown_btn_released)
                    {
                    group_map[selected_group]->countdown_secs=0;
                    countdown_btn_released=false;
                    }
                digitalWrite(countdown_led,HIGH);
                seven_segment_write_number(group_map[selected_group]->countdown_secs);

                if(ENC_BTN_TRIGGERED) //both buttons are pushed.
                    {
                        handle_tx("ALLOFF");
                        handle_tx("ALLOFF");
                        handle_tx("ALLOFF");
                        ENC_BTN_TRIGGERED=false;
                        for(int i=0;i<NUMBER_OF_GROUPS;i++) group_map[i]->active=false;
                    }

            }
        countdown_btn_released=true;

        //        redraw...
        digitalWrite(seven_segment_DP_pin,group_map[selected_group]->active); //indicate whether selected group is active with 7seg decimal point.
        seven_segment_write_number(group_map[selected_group]->group_number); //write proper number

        //set countdown led if a countdown is set:
        if(group_map[selected_group]->countdown_secs) digitalWrite(countdown_led,HIGH);
        else digitalWrite(countdown_led,LOW);
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

void isr_read_encoder() //triggers on change of ENC_A , but divides by 2 to make it match the hardware click of the encoder
{
    bool A = digitalRead(ENC_A);
    bool B = digitalRead(ENC_B);

    if(A!=B)
    {
        ccw_divider=false;

        if(cw_divider)
        {
            if (digitalRead(btn_pin)) selected_group--; //CW
            else group_map[selected_group]->countdown_secs--;
            cw_divider=false;
        }
        else cw_divider=true;
    }
    else
    {
        cw_divider=false;
        if(ccw_divider)
            {
                if (digitalRead(btn_pin)) selected_group++; //CCW
                else group_map[selected_group]->countdown_secs++;
                ccw_divider=false;
            }
        else ccw_divider=true;
    }

    //0-7
    if(selected_group>NUMBER_OF_GROUPS-1) selected_group=0;
    else if(selected_group<0) selected_group=NUMBER_OF_GROUPS-1;

    //0-9
    if(group_map[selected_group]->countdown_secs>9) group_map[selected_group]->countdown_secs=0;
    else if (group_map[selected_group]->countdown_secs<0) group_map[selected_group]->countdown_secs=9;


    ENC_HAS_MOVED=true;
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
    if ((millis()>ENC_BTN_MILLIS+DEBOUNCE_VAL) /*&& (digitalRead(ENC_BTN)==LOW)*/) //triggered on falling edge, so no need to check state
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
}
