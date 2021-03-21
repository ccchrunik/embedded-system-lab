/*
 * Copyright (c) 2014-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PinNames.h"
#include "ThisThread.h"
#include "mbed.h"
#include "mbed_wait_api.h"
#include <cstdio>

// Adjust pin name to your board specification.
// You can use LED1/LED2/LED3/LED4 if any is connected to PWM capable pin,
// or use any PWM capable pin, and see generated signal on logical analyzer.
PwmOut led(PWM_OUT);
// PwmOut led(LED1);

int main()
{
    // // specify period first
    // led.period(0.05f);      // 0.05 second period
    // led.write(0.80f);      // 50% duty cycle, relative to period
    // //led = 0.5f;          // shorthand for led.write()
    // //led.pulsewidth(2);   // alternative to led.write, set duty cycle time in seconds
    
    // ThisThread::sleep_for(4);
    
    // led.period(1.0f);
    // led.write(0.2f);

    // ThisThread::sleep_for(4);

    // led.period(0.05f);      // 0.05 second period
    // led.write(0.80f); 
    float period = 0.0001f;
    // float intensity = 0.5f;
    
    led.period(period);
    // led.write(intensity);
    // int count = 0;
    // float diff = 0.05;
    
    // while (1) {
    //     if(intensity >= 0.9) {
    //         diff = -0.05;
    //     } else if(intensity <= 0.1) {
    //         diff = 0.05;
    //     }

    //     intensity += diff;
    //     count++;
    //     printf("intensity = %f\n", intensity);
    //     wait_us(50);
    // }
    float i;
    while(1)
    {
        for(i = 0; i <= 0.99; i += 0.01)
        {
            led.write(i);
            wait_us(100);
        }  
        // for(i = 1; i >= 0; i -= 0.01)
        // {
        //     led.write(i);
        //     wait_us(100);
        // }  
    }

}
