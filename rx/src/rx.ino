#include <VirtualWire.h>
#include <Enerlib.h>

#define BTN_TIMEOUT_MS 2500
//#define SLEEPMODE() energy.Idle();
#define SLEEPMODE() energy.PowerDown(); //the lowest power setting.


Energy energy; //instantiate the low power library

//IO defines:
int btn_pin = 3;
int pwm_led = 4;
int blink_led = 13;
int RX_pin = 2;

//Global vars:

//RX stuff:
//String rx_string="";
uint8_t buf[VW_MAX_MESSAGE_LEN];
uint8_t buflen = VW_MAX_MESSAGE_LEN;

//other stuff:
bool reset = true;
int chosen_channel=0;

bool btn_triggered = false;
bool trigger_handled = false;
bool btn_debounced = true;
unsigned long btn_timer=0;

String msg_gr_1 =   "GR1";
String msg_gr_2 =   "GR2";
String msg_gr_3 =   "GR3";
String msg_gr_4 =   "GR4";
String msg_gr_5 =   "GR5";
String msg_gr_6 =   "GR6";
String msg_gr_7 =   "GR7";
String msg_gr_8 =   "GR8";
String msg_on =     "ON!";
String msg_off =    "OFF";
String msg_all =    "ALL";

void INT1_ISR(void)
{
  /*
  The WasSleeping function will return true if Arduino
  was sleeping before the IRQ. Subsequent calls to
  WasSleeping will return false until Arduino reenters
  in a low power state. The WasSleeping function should
  only be called in the ISR.
  */

  /*
    If Arduino was waked up by IRQ.
    If you shut down external peripherals before sleeping, you
    can reinitialize them here. Look on ATMega's datasheet for
    hardware limitations in the ISR when microcontroller just
    leave any low power state.
    */

  if (energy.WasSleeping()) reset=true;
  else btn_triggered=true; //The IRQ happened in awake state.
  }



void setup() {



    //Serial.begin(9600);

    pinMode(pwm_led, OUTPUT);

   // Initialise the IO and ISR
    vw_set_rx_pin(RX_pin);
    vw_setup(2000);  // Bits per sec
//    vw_rx_start();       // Start the receiver PLL running

    pulse(1,1); //hello world

    pinMode(btn_pin,INPUT_PULLUP);

    attachInterrupt(1, INT1_ISR, FALLING);

    /*
  Pin 3 will be the "wake button". Due to uC limitations,
  it needs to be a level interrupt.
  For experienced programmers:
    ATMega's datasheet contains information about the rest of
    wake up sources. The Extended Standby is not implemented.
    */

   //energy.Idle();
   //energy.PowerDown(); //the lowest power setting.
    SLEEPMODE();
    //go_to_sleep();

}

void loop()
    {
    //for RX'ing:
    if (reset) vw_rx_start();

    /*Button debouncing*/
    if (btn_triggered) //<-set by ISR1
    {
        btn_timer=millis();
        btn_triggered = false;
    }
    if(!digitalRead(btn_pin) && btn_timer+BTN_TIMEOUT_MS>millis()) btn_debounced=true;
    else btn_debounced=false;
    /*end of debounce*/

    if(btn_debounced) go_to_sleep();

    if (vw_get_message(buf, &buflen)) //check for message
        {
            if (reset) /*if newly awaken, we're armed and ready for a channel designation:*/
                {
                reset=false; //disarm
                /*if (rx_to_state())*/ chosen_channel=group_string_to_int(); //designate channel
                if (chosen_channel==11) reset=true; //error? REARM!
                }

            else if(chosen_channel==group_string_to_int()) digitalWrite(pwm_led,rx_to_state());
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

bool rx_to_state()
{
    String onoff_string="";
    for(int i=3; i<6;i++) onoff_string+=String(char(buf[i]));
    if(onoff_string=="ON!") return true;
    return false; //an else would be redundant
}


void go_to_sleep()
{
    chosen_channel=0; //release channel
    vw_rx_stop();
    SLEEPMODE();
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

