#include "nu32dip.h" // constants, functions for startup and UART
#include <stdio.h>
#include "ws2812b.h"
#include "uart2.h"

#define numLED 9
#define maxDuty 50//45
#define minDuty 0 //15

volatile int line_loc = 30;
//float kp_right = 2;
//float kp_left = 2;
volatile int pwm_left = maxDuty;
volatile int pwm_right = maxDuty;
float kp_right = 1;
float kp_left = 1;
volatile int line_loc_prev = 30;



void __ISR(_TIMER_5_VECTOR, IPL5SOFT) Control200Hz(void) {
    char message2[100];
//    char message[100];
    int com = 0;
        // uart2_flag() is 1 when uart2 has rx a message and sprintf'd it into a value
        if(get_uart2_flag()){
            set_uart2_flag(0); // set the flag to 0 to be ready for the next message
            com = get_uart2_value();
            line_loc = (int) (com);
            //sprintf(message2,"%d\r\n",line_loc);
            //NU32DIP_WriteUART1(message2);
            //sprintf(message2, "%d\t%d\r\n", pwm_left, pwm_right);
            //NU32DIP_WriteUART1(message2);
        }
    
    // OC2 is right motor A
    // OC3 is left motor B
    
    // duty cycle = OC#RS/(PR#+1)
    // pwm = [0, 100]%
    

    
    if (line_loc > 32){
        // turn right
        // left is faster than right
        
//        pwm_left = maxDuty;
//        pwm_right = maxDuty - kp_right*(line_loc-30);
//        pwm_left = 50;
//        pwm_right = 20 - kp_right*(line_loc-30);
        pwm_left = 45;
        pwm_right = 0;
//        pwm_right = 0;
        
    } else if(line_loc < 28){
        // turn left
        // right is faster than left
        
//        pwm_right = maxDuty;
//        pwm_left = maxDuty - kp_left*(30-line_loc);
//        pwm_right = 50;
//        pwm_left = 20 - kp_left*(30-line_loc);
        pwm_right = 45;
        pwm_left = 0;
//        pwm_left = 0;
        
    } else {
        pwm_right = 40;
        pwm_left = 40;
    }
    
//    pwm_right = pwm_right + kp_right*(30-line_loc);
//    pwm_left = pwm_left - kp_left*(30-line_loc);
    
//    if (line_loc >= 31){
////        pwm_right = minDuty;
////        pwm_left = maxDuty;
////        pwm_right = pwm_right + kp_right*(30-line_loc);
////        pwm_left = pwm_left - kp_left*(30-line_loc);
//        pwm_right = kp_right*(line_loc-30);
//        pwm_left = maxDuty;
//    }else if (line_loc <= 29){
////        pwm_right = maxDuty;
////        pwm_left = minDuty;
////        pwm_right = pwm_right + kp_right*(30-line_loc);
////        pwm_left = pwm_left - kp_left*(30-line_loc);
//        pwm_right = maxDuty;
//        pwm_left = kp_left*(30-line_loc);
//    } else {
//        pwm_right = maxDuty;
//        pwm_left = maxDuty;
//    }
    
    
    
////    if (pwm_right <= maxDuty*-1){
////        pwm_right = -1*maxDuty;
////    } else if (pwm_right < minDuty & pwm_right > -1*minDuty){
//    if (pwm_right < minDuty){
//
//        pwm_right = 1*minDuty;
//
//    } else if (pwm_right > maxDuty){
//        pwm_right = maxDuty;
//    }
//    
////    if (pwm_left <= maxDuty*-1){
////        pwm_left = -1*maxDuty;
////    } else if (pwm_left < minDuty & pwm_left > -1*minDuty){
//    if (pwm_left < minDuty){
//        pwm_left = 1*minDuty;
//       
//    } else if (pwm_left > maxDuty){
//        pwm_left = maxDuty;
//    }
    
    
    
//    sprintf(message2, "%d\t%d\r\n", pwm_left, pwm_right);
//    NU32DIP_WriteUART1(message2);
//    sprintf(message2, "Right: %d\r\n", pwm_right);
//    NU32DIP_WriteUART1(message2);
    
    
    LATBbits.LATB12 = 1; // set phase to low for right motor A
    LATBbits.LATB13 = 0; // set phase to high for left motor B
    
//    if (pwm_right < 0){
//        LATBbits.LATB12 = 0;
//    }
//    if (pwm_left < 0){
//        LATBbits.LATB13 = 1;
//    }
    OC2RS = (int) (abs(pwm_right)*1200/100);
    OC3RS = (int) (abs(pwm_left)*1200/100);
    
//    OC2RS = 0;
//    OC3RS = 0;
    
    // insert line to clear interrupt flag
    IFS0bits.T5IF = 0;  
}



int main(void) {
    NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
    ws2812b_setup();
    UART2_Startup();
    
    RPB15R = 0b0101; // Configure RB15 to OC1
    RPB11R = 0b0101; // Configure RB15 to OC2 motor A
    RPB10R = 0b0101; // Configure RB15 to OC3 motor B
    
    TRISBbits.TRISB12 = 0; // set phase pin motor A to output
    TRISBbits.TRISB13 = 0; // set phase pin motor B to output
    
    LATBbits.LATB12 = 1; // set phase to low for motor A
    LATBbits.LATB13 = 0; // set phase to high for motor B 
    
    __builtin_disable_interrupts();
    
    // Servo
    T2CONbits.TCKPS = 4;     // Timer2 prescaler N=4 (1:4)
    PR2 = 59999;              // period = (PR2+1) * N * (1/48000000) = 50 Hz
    TMR2 = 0;                // initial TMR2 count is 0
    OC1CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC1CON bits are defaults
    OC1CONbits.OCTSEL = 0;   // Use timer2
    OC1RS = 6000;             // duty cycle = OC1RS/(PR2+1) = 25%
    OC1R = 6000;              // initialize before turning OC1 on; afterward it is read-only
    T2CONbits.ON = 1;        // turn on Timer2
    OC1CONbits.ON = 1;       // turn on OC1
    
    // Motors
    // RIGHT
    T3CONbits.TCKPS = 1;     // Timer2 prescaler N=4 (1:4)
    PR3 = 1199;              // period = (PR2+1) * N * (1/48000000) = 50 Hz
    TMR3 = 0;                // initial TMR2 count is 0
    OC2CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC1CON bits are defaults
    OC2CONbits.OCTSEL = 1;   // Use timer2
    OC2RS = 500;             // duty cycle = OC1RS/(PR2+1) = 25%
    OC2R = 500;              // initialize before turning OC1 on; afterward it is read-only
    T3CONbits.ON = 1;        // turn on Timer2
    OC2CONbits.ON = 1;       // turn on OC1
    
    // LEFT
    OC3CONbits.OCM = 0b110;  // PWM mode without fault pin; other OC1CON bits are defaults
    OC3CONbits.OCTSEL = 1;   // Use timer2
    OC3RS = 500;             // duty cycle = OC1RS/(PR2+1) = 25%
    OC3R = 500;              // initialize before turning OC1 on; afterward it is read-only
    OC3CONbits.ON = 1;       // turn on OC1
    
    // Control
    T5CONbits.TCKPS = 54;     // Timer2 prescaler N=4 (1:4)
    PR5 = 7499;              // period = (PR2+1) * N * (1/48000000) = 50 Hz
    TMR5 = 0;                // initial TMR2 count is 0
    T5CONbits.ON = 1; 
    IPC5bits.T5IP = 5;            // step 4: interrupt priority 5
    IPC5bits.T5IS = 0;            // step 4: interrupt priority 1
    IFS0bits.T5IF = 0;            // step 5: clear the int flag
    IEC0bits.T5IE = 1;            // step 6: enable T4 by setting IEC0
    

    
    __builtin_enable_interrupts();

    
    
    
    
    // This range is ~180 degrees
    float min_t = 0.6;
    float max_t = 2.6;
    int max_duty = (int) (60000*max_t/20);
    int min_duty = (int) (60000*min_t/20);
    
    int pwm_val = min_duty;
    int pwm_flag = -1;
    
    char message[100];
    char message2[100];
    
//    NU32DIP_WriteUART1("Starting");
    
    unsigned int neopixel_counter = 0;

    wsColor LEDstrip[numLED];
    float hue = 0;
    float sat = 1.0;
    float brightness = 0.05;
    
    
    
    while (1) {
//        char message2[100];
//        int com = 0;
//        // uart2_flag() is 1 when uart2 has rx a message and sprintf'd it into a value
//        if(get_uart2_flag()){
//            set_uart2_flag(0); // set the flag to 0 to be ready for the next message
//            com = get_uart2_value();
//            line_loc = (int) (com);
//            sprintf(message2,"%d\r\n",line_loc);
//            NU32DIP_WriteUART1(message2);
//        }
//        
        
        
        
        neopixel_counter = neopixel_counter + 1;
        
        OC1RS = pwm_val;
        pwm_val = pwm_val+pwm_flag;
        if (pwm_val > max_duty){
            pwm_flag = -1;
        } else if (pwm_val < min_duty){
            pwm_flag = 1;
        }
        
        if (neopixel_counter == 25){
            LEDstrip[0].r = 0;
            LEDstrip[0].g = 100;
            LEDstrip[0].b = 127;
        
            for (int ind=1; ind<numLED; ind=ind+1){
                LEDstrip[ind] = HSBtoRGB(hue, sat, brightness);
            
                hue = hue + 0.1;
                if (hue > 360){
                    hue = 0;
                }
            }
            __builtin_disable_interrupts();
            ws2812b_setColor(LEDstrip,numLED);
            __builtin_enable_interrupts();
            
            neopixel_counter = 0;
        }
//        
//        
        unsigned int t = _CP0_GET_COUNT(); // should really check for overflow here
        while (_CP0_GET_COUNT() < t + 24000 * 2) {
        }
		
    }
}

