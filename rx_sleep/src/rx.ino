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
bool btn_state = false;
unsigned long btn_timer=0;

String msg_gr_1 = "GR1";
String msg_gr_2 = "GR2";
String msg_gr_3 = "GR3";
String msg_gr_4 = "GR4";
String msg_gr_5 = "GR5";
String msg_gr_6 = "GR6";
String msg_gr_7 = "GR7";
String msg_gr_8 = "GR8";
String msg_kill = "ALL_OFF";

void INT0_ISR(void)
{
  /*
  The WasSleeping function will return true if Arduino
  was sleeping before the IRQ. Subsequent calls to
  WasSleeping will return false until Arduino reenters
  in a low power state. The WasSleeping function should
  only be called in the ISR.
  */
  if (energy.WasSleeping())
  {
    /*
    Arduino was waked up by IRQ.
    If you shut down external peripherals before sleeping, you
    can reinitialize them here. Look on ATMega's datasheet for
    hardware limitations in the ISR when microcontroller just
    leave any low power state.
    */

    vw_rx_start();
    reset=true;

  }
  else
  {

    /*
    The IRQ happened in awake state.

    This code is for the "normal" ISR.
    */
    /*ONLY WORKS IF THIS IS A RISING EDGE TRIGGERED ISR.*/

    if(btn_state==false)
        {
            btn_state = true;
            btn_timer=millis();
        }

    //else reset=true;
  }
}



void setup() {



    //Serial.begin(9600);

    pinMode(pwm_led, OUTPUT);

   // Initialise the IO and ISR
    vw_set_rx_pin(RX_pin);
    vw_setup(2000);  // Bits per sec
//    vw_rx_start();       // Start the receiver PLL running

    pulse(1,1); //hello world

   attachInterrupt(0, INT0_ISR, RISING);

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

void loop() {
    //for RX'ing:

    /* check if the button is being continually pressed:*/
    if (digitalRead(btn_pin)==LOW) btn_state=false;
    else if (millis()>btn_timer+BTN_TIMEOUT_MS) go_to_sleep();

    /*if newly awaken, we're armed and ready for a channel designation:*/
    if(reset=true)
        {
            if (vw_get_message(buf, &buflen)) //check for message
            {
                reset=false; //disarm
                chosen_channel=rx_string_to_command(); //designate channel
                if (chosen_channel<8) reset=true; //error? REARM!
            }
        }

    else
        {

        }


    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
        //int i;

        //rx_string="";

        //digitalWrite(13, true); // Flash a light to show received good message
        // Message with a good checksum received, dump it.
        //Serial.print("Got: ");

        //for (i = 0; i < buflen; i++) rx_string+=String(char(buf[i]));


        //if(rx_string=="hello") pulse(1,1);

        //if(rx_string!="") pulse(1,1);

        /*
        Serial.print("RX: ");
        Serial.print(rx_string);
        Serial.println("<--- EOL ");
        */
    }



}

int rx_string_to_command()
{
    String rx_string="";
    int command=11; //undefined state.
    for (int i = 0; i < buflen; i++) rx_string+=String(char(buf[i]));
    if(rx_string==msg_gr_1) command=1;
    else if(rx_string==msg_gr_2) command=2;
    else if(rx_string==msg_gr_3) command=3;
    else if(rx_string==msg_gr_4) command=4;
    else if(rx_string==msg_gr_5) command=5;
    else if(rx_string==msg_gr_6) command=6;
    else if(rx_string==msg_gr_7) command=7;
    else if(rx_string==msg_gr_8) command=8;
    else if(rx_string==msg_kill) command=10;

    return command;
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

