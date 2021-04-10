#include "ThisThread.h"
#include "mbed.h"
#include "mbed_events.h" 

DigitalOut led1(LED1);
InterruptIn sw(USER_BUTTON);
EventQueue queue(32 * EVENTS_EVENT_SIZE); 
// EventQueue queue(2 * EVENTS_EVENT_SIZE); 
Thread t;

void rise_handler(void) {
    // Toggle LED
    led1 = !led1;
    osThreadId_t fall_t_id = ThisThread::get_id(); 
    queue.call(printf, "rise_handler in context %p\n", fall_t_id);
    // printf("rise_handler in context %p\n", ThisThread::get_id());
}

void fall_handler(void) {
    // Toggle LED
    led1 = !led1;
    printf("fall_handler in context %p\n", ThisThread::get_id());
}

int main() {
    // Start the event queue
    t.start(callback(&queue, &EventQueue::dispatch_forever)); 
    printf("Starting in context %p\r\n", ThisThread::get_id()); 
    // The 'rise' handler will execute in IRQ context 
    sw.rise(rise_handler);
    // The 'fall' handler will execute in the context of thread 't' 
    sw.fall(queue.event(fall_handler));
}