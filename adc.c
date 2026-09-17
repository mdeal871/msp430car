// ------------------------------------------------------------------------------
//  File Name: adc.h
//
//  Description: This file contains all functions and interrupts related to the
//  Analog-Digital converter. This interacts with the thumb wheel and left/right
//  detectors
//
//
//  Mason Deal
//  Mar 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------
#include  "msp430.h"
#include  <string.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"
#include  "globals.h"

void Num_Update(unsigned int adc_val);

void Num_Update(unsigned int adc_val) {                             // Function to update the displayed value from the ADC channels
    unsigned int temp = adc_val >> 2;                               // Logical shift by 2 bits to convert to Carlson's preferred max values
        thousands = '0';                                            // Initialize thousands to 0
        while (temp >= 1000) {temp -= 1000; thousands++;}           // While the value is above 1000, subtract 1000 and increment thousands
        hundreds = '0';                                             // Initialize hundreds to 0
        while (temp >= 100) {temp -= 100; hundreds++;}              // While the value is above 100, subtract 100 and increment hundreds
        tens = '0';                                                 // Initialize tens to 0
        while (temp >= 10) {temp -= 10; tens++;}                    // While the value is above 10, subtract 10 and increment tens
        ones = '0' + temp;                                          // The remainder is the value of ones; add that value to the ones holder
}

void ADC_Process(void) {

    // Thumb Wheel (line 3) - raw display
//    Num_Update(thumb_adc_value);

//    // Runtime timer
//    if (Runtime_FLG) {
//    Num_Update(Runtime_Count / 20);
//    display_line[3][5] = thousands;
//    display_line[3][6] = hundreds;
//    display_line[3][7] = tens;
//    display_line[3][8] = ones;
//    display_line[3][9] = ' ';
//    }

    // V_DETECT_L - raw display + threshold flag
//    Num_Update(left_detect_adc_value);
//    display_line[1][6] = thousands;
//    display_line[1][7] = hundreds;
//    display_line[1][8] = tens;
//    display_line[1][9] = ones;
    if ((left_detect_adc_value >> 2 ) > DETECT_THRESHOLD) {
        left_line_detected = LINE_DETECTED;
    } else {
        left_line_detected = LINE_NOT_DETECTED;
    }

    // V_DETECT_R - raw display + threshold flag
//    Num_Update(right_detect_adc_value);
//    display_line[2][6] = thousands;
//    display_line[2][7] = hundreds;
//    display_line[2][8] = tens;
//    display_line[2][9] = ones;
    if ((right_detect_adc_value >> 2) > DETECT_THRESHOLD) {
        right_line_detected = LINE_DETECTED;
    } else {
        right_line_detected = LINE_NOT_DETECTED;
    }

    display_changed = TRUE;
}

void Init_ADC(void){
//------------------------------------------------------------------------------
// V_DETECT_L (0x04) // Pin 2 A2
// V_DETECT_R (0x08) // Pin 3 A3
// V_THUMB (0x20) // Pin 5 A5
//------------------------------------------------------------------------------
// ADCCTL0 Register
ADCCTL0 = 0; // Reset
ADCCTL0 |= ADCSHT_2; // 16 ADC clocks
ADCCTL0 |= ADCMSC; // MSC
ADCCTL0 |= ADCON; // ADC ON
// ADCCTL1 Register
ADCCTL1 = 0; // Reset
ADCCTL1 |= ADCSHS_0; // 00b = ADCSC bit
ADCCTL1 |= ADCSHP; // ADC sample-and-hold SAMPCON signal from sampling timer.
ADCCTL1 &= ~ADCISSH; // ADC invert signal sample-and-hold.
ADCCTL1 |= ADCDIV_0; // ADC clock divider - 000b = Divide by 1
ADCCTL1 |= ADCSSEL_0; // ADC clock MODCLK
ADCCTL1 |= ADCCONSEQ_0; // ADC conversion sequence 00b = Single-channel single-conversion
// ADCCTL1 & ADCBUSY identifies a conversion is in process
// ADCCTL2 Register
ADCCTL2 = 0; // Reset
ADCCTL2 |= ADCPDIV0; // ADC pre-divider 00b = Pre-divide by 1
ADCCTL2 |= ADCRES_2; // ADC resolution 10b = 12 bit (14 clock cycle conversion time)
ADCCTL2 &= ~ADCDF; // ADC data read-back format 0b = Binary unsigned.
ADCCTL2 &= ~ADCSR; // ADC sampling rate 0b = ADC buffer supports up to 200 ksps

ADCCTL1 |= ADCCONSEQ_0;   // Single-channel single-conversion
ADCMCTL0 = 0;
ADCMCTL0 |= ADCSREF_0;
ADCMCTL0 |= ADCINCH_5;    // Start with V_THUMB on A5
ADCIE |= ADCIE0;
ADCCTL0 |= ADCENC;
ADCCTL0 |= ADCSC;
}

//---------------------------------------------------------------------------------
//  Interrupts
//---------------------------------------------------------------------------------
// ADC Interrupt - fires when conversion is complete
#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void) {
    switch(__even_in_range(ADCIV, ADCIV_ADCIFG)) {
    case ADCIV_ADCIFG:
        switch(adc_channel) {
            case 0:
                thumb_adc_value = ADCMEM0;
                adc_channel = 1;
                ADCCTL0 &= ~ADCENC;                // Disable to change channel
                ADCMCTL0 = ADCSREF_0 | ADCINCH_2;  // Switch to A2 - V_DETECT_L
                break;
            case 1:
                left_detect_adc_value = ADCMEM0;
                adc_channel = 2;
                ADCCTL0 &= ~ADCENC;
                ADCMCTL0 = ADCSREF_0 | ADCINCH_3;  // Switch to A3 - V_DETECT_R
                break;
            case 2:
                right_detect_adc_value = ADCMEM0;
                adc_channel = 0;
                ADCCTL0 &= ~ADCENC;
                ADCMCTL0 = ADCSREF_0 | ADCINCH_5;  // Switch back to A5 - V_THUMB
                break;
        }
        // DISABLED because it fires interrupts every 2.8us. This is now controlled by Timer_B0
//        ADCCTL0 |= ADCENC;   // Re-enable interrupt and restart
//        ADCCTL0 |= ADCSC;
        break;
    default: break;
    }
}
