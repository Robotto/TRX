#include <VirtualWire.h>
#include <Enerlib.h>

//#define DEBUG
//#define BEEP

#define DEBOUNCE_MS 200
//#define SLEEPMODE() energy.Idle();
//#define SLEEPMODE() energy.PowerDown(); //the lowest power setting.


//Energy energy; //instantiate the low power library

//IO defines:
int btn_pin = 3;
int pwm_led = 5;
int blink_led = 13;
int RX_pin = 2;

#ifdef BEEP
int beep_pin = 10;
#endif

//Global vars:

//RX stuff:
//String rx_string="";
uint8_t buf[VW_MAX_MESSAGE_LEN];
uint8_t buflen = VW_MAX_MESSAGE_LEN;

//other stuff:
bool reset = true;
bool state_handled=false;
unsigned int chosen_channel=0;
unsigned int state=0;


String msg_gr_1 =   "GR1";
String msg_gr_2 =   "GR2";
String msg_gr_3 =   "GR3";
String msg_gr_4 =   "GR4";
String msg_gr_5 =   "GR5";
String msg_gr_6 =   "GR6";
String msg_gr_7 =   "GR7";
String msg_gr_8 =   "GR8";
String msg_on =     "ON!";
String msg_blink =  "BLK";
String msg_off =    "OFF";
String msg_all =    "ALL";

void INT0_ISR(void)
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > DEBOUNCE_MS)
  {
    #ifdef BEEP
    beep();
    #endif
    pulse(1,1);
    state=0;
    state_handled=false;
    reset=true;
  }
  last_interrupt_time = interrupt_time;
}



void setup() {


    #ifdef DEBUG
    Serial.begin(9600);
    #endif

    #ifdef BEEP
    pinMode(beep_pin,OUTPUT);
    #endif

    pinMode(pwm_led, OUTPUT);

   // Initialise the IO and ISR
    vw_set_rx_pin(RX_pin);
    vw_setup(2000);  // Bits per sec
    vw_rx_start();       // Start the receiver PLL running

    pulse(1,1); //hello world

    pinMode(btn_pin,INPUT_PULLUP);

    attachInterrupt(0, INT0_ISR, FALLING);

//    SLEEPMODE();

}

void loop()
    {

            //RX:
    if (vw_get_message(buf, &buflen)) //check for message
        {

            #ifdef DEBUG
            String rx_string;
            for (int i = 0; i < buflen; i++) rx_string+=String(char(buf[i]));
            Serial.print("RX raw: ");
            Serial.println(rx_string);
            Serial.print("RX string group: ");
            Serial.println(group_string_to_int());
            Serial.print("RX string state: ");
            Serial.println(rx_to_state());
            #endif

            if (reset) /*if newly awaken, we're armed and ready for a channel designation:*/
                {
                reset=false; //disarm
                chosen_channel=group_string_to_int(); //designate channel
                if (chosen_channel>8) reset=true; //error? REARM!
                }


            else if(group_string_to_int()==chosen_channel || group_string_to_int()==10) //10 = all
                {
                    unsigned int newstate=rx_to_state();
                    if(newstate!=state)
                        {
                            state=newstate;
                            state_handled=false;
                            #ifdef BEEP
                            if(state) beep();
                            #endif
                        }
                }
        }
            //STATEMACHINE:

    if (!state_handled) switch (state)
        {
            case 0:
                digitalWrite(pwm_led,0);
                state_handled=true;
                break;
            case 1:
                for (int val=0; val<255; val++) {analogWrite(pwm_led, val); delay(1);}
                state_handled=true;
                break;
            case 2:
                pulse(1,1);
                //delay?
                break;
        }

    }


int group_string_to_int()
{
    String group_string="";
    for (int i = 0; i < 3; i++) group_string+=String(char(buf[i]));

    if     (group_string==msg_gr_1) return 1;
    else if(group_string==msg_gr_2) return 2;
    else if(group_string==msg_gr_3) return 3;
    else if(group_string==msg_gr_4) return 4;
    else if(group_string==msg_gr_5) return 5;
    else if(group_string==msg_gr_6) return 6;
    else if(group_string==msg_gr_7) return 7;
    else if(group_string==msg_gr_8) return 8;
    else if(group_string==msg_all)  return 10;
    else return 11; //in case of error
}

unsigned int rx_to_state()
{
    String rx="";
    for(int i=3; i<6;i++) rx+=String(char(buf[i]));

    if(rx==msg_on) return 1;
    if(rx==msg_blink) return 2; //an else would be redundant
    return 0;


}

#ifdef BEEP
void beep()
{
    static unsigned long tone1_reps=300;
    static unsigned long tone1_delay=125;

    static unsigned long tone2_reps=300;
    static unsigned long tone2_delay=105;

    static unsigned long tone3_reps=600;
    static unsigned long tone3_delay=99;


    for(unsigned long i=0;i<tone1_reps;i++)
    {
        digitalWrite(beep_pin,HIGH);
        delayMicroseconds(tone1_delay);
        digitalWrite(beep_pin,LOW);
        delayMicroseconds(tone1_delay);
    }

    for(unsigned long i=0;i<tone2_reps;i++)
    {
        digitalWrite(beep_pin,HIGH);
        delayMicroseconds(tone2_delay);
        digitalWrite(beep_pin,LOW);
        delayMicroseconds(tone2_delay);
    }


    for(unsigned long i=0;i<tone3_reps;i++)
    {
        digitalWrite(beep_pin,HIGH);
        delayMicroseconds(tone3_delay);
        digitalWrite(beep_pin,LOW);
        delayMicroseconds(tone3_delay);
    }


}

#endif

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


