subl //encoders

///////|  Rotary_encoders.h |///////////////////
// Contains functions for setting up          //
// and reading the rotary encoders on         //
// the DigiPot V.2, using interrupt on change //
////////////////////////////////////////////////

////| Hardware IO defines: |////
#define LA PORTBbits.RB4
#define LB PORTBbits.RB5
#define RA PORTBbits.RB6
#define RB PORTBbits.RB7

////| function prototyping: |////
void Encoder(void);
void Encoderinit(void);

////| Global variables: |////
char LastLA, LastRA, Left_rotary_flag=0, Right_rotary_flag=0;
signed char Lcount=0, Rcount=0;

////| Interrupt declaration |////
// sets the interrupt vector at 0x8 to
// point at the encoder interrupt routine

#pragma code HIGH_INTERRUPT_VECTOR = 0x8
void high_ISR (void){

    _asm
    goto Encoder
    _endasm

}
#pragma code

////| Encoder (interrupt function) |////
// Reads the encoders upon an interrupt,
// and determines the rotational vector:
// + is clockwise (CW)
// - is counterclockwise (CCW)

#pragma interrupt Encoder
void Encoder(void)
{
    if(INTCONbits.RBIF == 1)
        {
        //this code requires the input that triggered the interrupt
        //to be the data that is read out in the below code. (no failsafe)
        //(state must not have changed after interrupt is called):

    //  Lcount=0; //to be set immediately after a send
    //  Rcount=0; //to be set immediately after a send

        //Left rotary encoder:
        if(LastLA != LA)
            {
            if     (LA == 1 & LB == 0) Lcount--;  //CW rotation
            else if(LA == 0 & LB == 1) Lcount--;  //CW rotation  = negative increment
            else if(LA == 1 & LB == 1) Lcount++;  //CCW rotation
            else if(LA == 0 & LB == 1) Lcount++;  //CCW rotation = positive increment
            LastLA = LA;
            Left_rotary_flag=1;
            }

        //right rotary encoder:
        if(LastRA != RA)
            {
            if     (RA == 1 & RB == 0) Rcount--; //CW rotation
            else if(RA == 0 & RB == 1) Rcount--; //CW rotation
            else if(RA == 1 & RB == 1) Rcount++; //CCW rotation
            else if(RA == 0 & RB == 1) Rcount++; //CCW rotation
            LastRA = RA;
            Right_rotary_flag=1;
            }

        INTCONbits.RBIF = 0;
        }

}

///////| Encoderinit |///////
// This function sets up the neccesary interrupts
// and IO for the encoders to work.

void Encoderinit(void)
    {
    //Tristate registers to input:
    TRISBbits.TRISB4=1;  //
    TRISBbits.TRISB5=1;  //
    TRISBbits.TRISB6=1;  //
    TRISBbits.TRISB7=1;  //

    //initialize the last-state indicators:
    LastLA = LA;
    LastRA = RA;

    //Hight priority Interrupt on change:
    INTCONbits.RBIE = 1;    //PORTB interrupt enabled
    INTCONbits.PEIE = 1;    //enable peripheral interrupt
    INTCON2bits.RBIP = 1;   //IOC on PORTB set to high priority
    //INTCON2bits.RBPU = 0;   //enable weak pull-up on all PORTB pins ---for debugging

    //Interrupt - check this again:!!!
    RCONbits.IPEN=1; //interrupt prioritizing disabled - all interrupts are same priority

    INTCONbits.RBIF = 0;    //reset port B interrupt flag

    INTCONbits.GIE = 1;     //Global interrupt enabled!
    }
