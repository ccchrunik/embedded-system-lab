#include "PinNames.h"
#include "ThisThread.h"
#include "mbed.h"
#include "mbed_events.h" 
#include <cstdio>

DigitalOut led(LED1);
InterruptIn button(USER_BUTTON);
Timeout  press_threhold;
EventQueue *queue = mbed_event_queue();


void button_release_detecting()
{
    button.enable_irq();
}

void button_pressed()
{
    button.disable_irq();
    queue->call(printf, "pressed\n");
    press_threhold.attach(button_release_detecting, 3.0);
    queue->call(printf, "start timer...\n");
}

void button_released()
{
    led = !led;
    queue->call(printf, "released\n");
}


int main() {
    // The 'rise' handler will execute in IRQ context 
    button.rise(button_released);
    // The 'fall' handler will execute in the context of thread 't' 
    button.fall(button_pressed);
    queue->dispatch_forever();
}