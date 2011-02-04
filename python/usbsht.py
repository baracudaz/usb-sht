#!/usr/bin/env python
r"""Python library for USB-SHT Device
"""

__author__ = 'Stanislav Likavcan'

import usb.core
import random, math

# Standard USB requests (libsub - usb.h)
USB_TYPE_STANDARD       = (0x00 << 5)
USB_TYPE_CLASS          = (0x01 << 5)
USB_TYPE_VENDOR         = (0x02 << 5)
USB_TYPE_RESERVED       = (0x03 << 5)

USB_RECIP_DEVICE        = 0x00
USB_RECIP_INTERFACE     = 0x01
USB_RECIP_ENDPOINT      = 0x02
USB_RECIP_OTHER         = 0x03

# Various libusb API related stuff
USB_ENDPOINT_IN         = 0x80
USB_ENDPOINT_OUT        = 0x00

# USB Device vendor and product id
USB_DEVICE_VID = 0x16C0
USB_DEVICE_PID = 0x05DC

CUSTOM_RQ_ECHO = 0
r"""Request that the device sends back wValue and wIndex. 
This is used with random data to test the reliability of the communication.
"""

CUSTOM_RQ_SET_STATUS = 1
r"""Set the LED status. Control-OUT.
The requested status is passed in the "wValue" field of the control
transfer. No OUT data is sent. Bit 0 of the low byte of wValue controls
the LED.
"""

CUSTOM_RQ_GET_STATUS = 2
r"""Get the current LED status. Control-IN.
This control transfer involves a 1 byte data phase where the device sends
the current status to the host. The status is in bit 0 of the byte.
"""

CUSTOM_RQ_GET_TEMPERATURE = 3
CUSTOM_RQ_GET_HUMIDITY    = 4

class USBSHT:
    def __init__(self):
        self.device = usb.core.find(idVendor = USB_DEVICE_VID, idProduct = USB_DEVICE_PID)
        if self.device is None:
            raise ValueError('USB device not found')
            
        self.device.set_configuration()
        
    def _set(self, request, value, index = 0):
        self.device.ctrl_transfer(bmRequestType = USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, 
                                  bRequest = request, wValue = value, wIndex = index)
        
    def _get(self, request, value = 0, index = 0, length = 2):
        ret = self.device.ctrl_transfer(bmRequestType = USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                                        bRequest = request, wValue = value, wIndex = index, data_or_wLength = length)
        
        if (len(ret) != length):
            print 'ERROR: Incorrect data message length'
            return -1
            
        return ret
            
    def version(self):
        ver = self._get(LCD_GET_FWVER)
        print 'Firmware version %d.%d' % (ver & 0xff, ver >> 8);

    def echo(self, ECHO_NUM = 100):
        errors = 0
        for i in range(0, ECHO_NUM):
            value1 = random.randint(0, 0xffff)
            value2 = random.randint(0, 0xffff) 
            
            ret = self._get(CUSTOM_RQ_ECHO, value = value1, index = value2, length = 4)
            
            echo1 = (ret[1] << 8) | ret[0]
            echo2 = (ret[3] << 8) | ret[2]
            
            if ((value1 != echo1) or (value2 != echo2)):
                errors =+ 1
                        
        if (errors):
            raise ValueError('ERROR: %d out of %d echo transfers failed!', errors, ECHO_NUM)
        else:
            print 'Echo test successful!'
            
    def led_status(self):        
        ret = self._get(CUSTOM_RQ_GET_STATUS, length = 1)
        
        return (ret[0] & 1)
        
    def led_on(self):
        self._set(CUSTOM_RQ_SET_STATUS, 1)

    def led_off(self):
        self._set(CUSTOM_RQ_SET_STATUS, 0)
            
    def temperature(self):
        ret = self._get(CUSTOM_RQ_GET_TEMPERATURE, length = 2)
        val = ret[0] << 8 | ret[1]
        
        # Temperature conversion formula
        # T = d1 + d2 * val ['C] (degree Celsius)
        d1 = -40.1
        d2 =   0.01
        # Above coefficients valid for Vdd = 5V and 14bit resolution

        temperature = d1 + d2 * val
        return temperature
        
    def humidity(self):
        ret = self._get(CUSTOM_RQ_GET_HUMIDITY, length = 2)
        val = ret[0] << 8 | ret[1]
        
        # Relative Humidity conversion formula
        # RH = c1 + c2 * val + c3 * val^2 [%RH]
        c1 = -4.0
        c2 =  0.0405
        c3 = -2.8000E-6
        # Above coefficients valid for 12bit resolution
        
        t1 =  0.01
        t2 =  0.00008
        # Above coefficients valid for 14bit resolution
        
        humidity_linear = c1 + c2 * val + c3 * pow(val, 2)
        humidity_corrected = (self.temperature() - 25.0 ) * (t1 + t2 * val) + humidity_linear;

        # Cut if the value is outside of the physical possible range
        if(humidity_corrected > 100):
            humidity_corrected = 100 
        if(humidity_corrected < 0.1):
            humidity_corrected = 0.1

        return humidity_corrected
    
    def dew_point(self):
        temperature = self.temperature()
        humidity    = self.humidity()

        k = (math.log10(humidity)-2)/0.4343 + (17.62*temperature)/(243.12+temperature)
        dew_point = 243.12*k/(17.62-k)
        
        return dew_point;
        
    
if __name__ == "__main__":
    
    import sys
    
    if len(sys.argv) != 2:
        print 'Error: Missing option'
        sys.exit(-1)
    
    sensor = USBSHT()
    
    if sys.argv[1] == 'echo':
        sensor.echo()
    elif sys.argv[1] == 'on':
        sensor.led_on()
    elif sys.argv[1] == 'off':
        sensor.led_off()
    elif sys.argv[1] == 'status':
        status = sensor.led_status()
        if (status):
            print 'LED is on'
        else:
            print 'LED if off'
        
    elif sys.argv[1] == 'temp':
        temperature = sensor.temperature()
        print 'Temperature : %.2f' % temperature

    elif sys.argv[1] == 'dew':
        dew_point = sensor.dew_point()
        print 'Dew Point : %.2f' % dew_point
        
    elif sys.argv[1] == 'hum':
        humidity = sensor.humidity()
        print 'Relative Humidity : %.2f' % humidity
        
    else:
        print 'ERROR: Unrecognized option: %s' % sys.argv[1]
        sys.exit(-1)
        
    sys.exit(0)
        