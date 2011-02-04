/* Name: main.c
 * Author: Stanislav Likavcan (likavcan@gmail.com)
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include "usbdrv.h"
#include "oddebug.h"
#include "sht1x.h"
#include "led.h"

#define CUSTOM_RQ_ECHO          0
/* Request that the device sends back wValue and wIndex. This is used with
 * random data to test the reliability of the communication.
 */
#define CUSTOM_RQ_SET_STATUS    1
/* Set the LED status. Control-OUT.
 * The requested status is passed in the "wValue" field of the control
 * transfer. No OUT data is sent. Bit 0 of the low byte of wValue controls
 * the LED.
 */

#define CUSTOM_RQ_GET_STATUS    2
/* Get the current LED status. Control-IN.
 * This control transfer involves a 1 byte data phase where the device sends
 * the current status to the host. The status is in bit 0 of the byte.
 */
#define CUSTOM_RQ_GET_TEMPERATURE 3
#define CUSTOM_RQ_GET_HUMIDITY 4


usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t    *rq = (void *)data;
    static uchar    dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns, therefore static */
    uint8_t         msgLen = 0;     /* default for not implemented requests: return no data back to host */
    
    DBG1(0x03, &rq->bRequest, 1);
    DBG1(0x04, &rq->wValue.bytes[0], 2);
    
    switch (rq->bRequest) {
        case CUSTOM_RQ_ECHO:
            dataBuffer[0] = rq->wValue.bytes[0];
            dataBuffer[1] = rq->wValue.bytes[1];
            dataBuffer[2] = rq->wIndex.bytes[0];
            dataBuffer[3] = rq->wIndex.bytes[1];
            msgLen = 4;
            break;
        
        case CUSTOM_RQ_SET_STATUS:
            if(rq->wValue.bytes[0] & 1){    /* set LED */
                led_on(LED_RED);
            }else{                          
                led_off(LED_RED);           /* clear LED */
            }            
            break;
            
        case CUSTOM_RQ_GET_STATUS:
            dataBuffer[0] = (led_status(LED_RED) != 0);
            msgLen = 1;                     /* tell the driver to send 1 byte */
            break;
        
        case CUSTOM_RQ_GET_TEMPERATURE:
            sht1x_measure(SHT_CMD_MEASURE_TEMPERATURE, &dataBuffer[0]);
            //sser_getWord(SHT_CMD_MEASURE_TEMPERATURE, &dataBuffer[0]);
            msgLen = 2;
            DBG2(0x05, &dataBuffer[0], 2);
            break;

        case CUSTOM_RQ_GET_HUMIDITY:
            sht1x_measure(SHT_CMD_MEASURE_HUMIDITY, &dataBuffer[0]);
            //sser_getWord(SHT_CMD_MEASURE_HUMIDITY, &dataBuffer[0]);
            msgLen = 2;
            DBG2(0x05, &dataBuffer[0], 2);
            break;
            
        default:
            break;
    }
    usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
    return msgLen;                  /* tell the driver lenght of data to send */
}


int main(void)
{
    led_init();    
    sht1x_init();
    //sser_init();
    
    wdt_enable(WDTO_1S);    
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    odDebugInit();
    DBG1(0x00, 0, 0);       /* debug output: main starts */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    unsigned char i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    DBG1(0x01, 0, 0);       /* debug output: main loop starts */        
    for(;;){
        wdt_reset();
        usbPoll();
    }
    return 0;   /* never reached */
}
