// Reference: https://youtu.be/Hid_jB_Dy-A?list=PLbFgDf51ZkCGCGxMXQiIQWJUbOoYj9Y5X
// Reference: https://github.com/selimg76/microcontroller/blob/master/dht11_MSP430G2553

//Connect DHT11 Data pin to P2.4 pin of MSP430G2553
// Preview final output in Expressions by clicking on Debug -> Resume -> Suspend
// temperature = TemperatureHigh.TemperatureLow celcius
// Humidity = humidityHigh.humidityLow


#include<msp430.h>

volatile int temp[50];                            // storing temporary data
volatile int diff[50];                            // for storing time differences
volatile unsigned int i = 0;
volatile unsigned int j = 0;
volatile unsigned int humidityHigh = 0;                    // For humidity high byte
volatile unsigned int humidityLow = 0;                     // For humidity low byte
volatile unsigned int TemperatureHigh = 0;                 // For temperature high byte
volatile unsigned int TemperatureLow = 0;                  // For temperature low byte
volatile unsigned char checksum = 0;              // For DHT11 checksum byte
volatile unsigned char check = 0;                 // For DHT11 40 bytes temp and humidity byte
volatile unsigned char dataok = 0;                // Assign 1 if checksum == check

void main(void){
    WDTCTL = WDTPW | WDTHOLD;                     // Stop watchdog timer
    BCSCTL1 = CALBC1_1MHZ;                        // set Clock control
    DCOCTL = CALDCO_1MHZ;

    __delay_cycles(2000000);                       // 1MHZ -> 1microsec -> 1 second = 2000000

    P2DIR |= BIT4;                                // Set P2.4 as GPIO - Low(0)
    P2OUT &= ~BIT4;                               // Set P2.4 as Input intially - Low(0)

    __delay_cycles(20000);                         // Delay for 20 milli second  = 20000

    P2OUT |= BIT4;                               // Set P2.4 as Output - HIGH(1)

    __delay_cycles(20);                           // Delay for n micro second  = 20
    P2DIR &= ~BIT4;                              // Set P2.4 as Low

    P2SEL |= BIT4;                               // Set P2.4 as  specialized function

    TA1CTL = TASSEL_2|MC_2;                         // Select SMCLK as clock source and selects 'Continuous' mode for timer
    TA1CCTL2 = CAP | CCIE | CCIS_0 | CM_2 | SCS ;   // Command sets the timer to
                                                    // CAP - capture mode,
                                                    // CCIE - enables capture compare interrupt
                                                    // CCIS_0 - Capture input select to P2.4
                                                    // CM_2 - Capture mode as falling edge
                                                    // SCS - Synchronizes capture signal with clock

    _enable_interrupts();                           // Enable general interrupt

    while(1){
        // Check if we have got 40 captures are not
        if (i >= 40){
            // First 8 Bytes Humidty High
            for (j = 1; j <= 8; j++){
                // 110 is threshold  if > 110 -> 1 else 0
                if (diff[j] >= 110)
                    // Shift 1 bit to left 7 times
                    humidityHigh |= (0x01 << (8-j));
            }
            // Next 8 Bytes Humidty Low
            for (j = 9; j <= 16; j++){
                // Shift 1 bit to left 7 times
                if (diff[j] >= 110)
                    humidityLow |= (0x01 << (16-j));
            }
            // Next 8 Bytes Temperature High
            for (j = 17; j <= 24; j++){
                // Shift 1 bit to left 7 times
                if (diff[j] >= 110)
                    TemperatureHigh |= (0x01 << (24-j));
            }
            // Next 8 Bytes Temperature Low
            for (j = 25; j <= 32; j++){
                // Shift 1 bit to left 7 times
                if (diff[j] >= 110)
                    TemperatureLow |= (0x01 << (32-j));
            }
            // Next 8 Bytes Checksum value
            for (j = 33; j <= 40; j++){
                // Shift 1 bit to left 7 times
                if (diff[j] >= 110)
                    checksum |= (0x01 << (40-j));
            }
            // Sum of all values
            check = humidityHigh + humidityLow + TemperatureHigh + TemperatureLow;
            if (check == checksum)
                dataok = 1;
            else
                dataok = 0;
        }

    }

}

// Interrupt vector(ISR)
#pragma vector = TIMER1_A1_VECTOR    // Memory address of ISR
__interrupt void Timer_A1(void){     // ISR
    temp[i] = TA1CCR2;               // Each interrupt store value to temporary
    i += 1;                          //
    if (i>=2) diff[i-1]=temp[i-1]-temp[i-2]; // falling edge capture
    TA1CCTL2 &= ~CCIFG ;             // Interrupt Flag
}
