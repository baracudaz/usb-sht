#ifndef __led_h_included__
#define __led_h_included__

#define LED_DDR         DDRB
#define LED_PORT        PORTB
#define LED_RED         PB1
#define LED_GREEN       PB5

// Set pin to output
//#define pin_out(x)     LED_DDR  |=  (1 << (x))

// Set pin to input
//#define pin_in(x)      LED_DDR  |=  (1 << (x))

// Set LED pins to output 
#define led_init()      LED_DDR  |=  (1 << LED_RED) | (1 << LED_GREEN) 

// Set output pin to high
#define led_on(x)       LED_PORT |=  (1 << (x))

// Set output pin to low
#define led_off(x)      LED_PORT &= ~(1 << (x))

// Toggle (invert) current status of pin
#define led_toggle(x)   LED_PORT ^= (1 << (x))

// Test pin for status
#define led_status(x)   (LED_PORT &  (1 << (x)))


#endif /* __led_h_included__ */