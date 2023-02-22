## Overview ##

I'm testing connecting the ESP32 to the 2.8" ili9341 TFT
SPI display (320x240) which contains a xpt2046 touchscreen
for use within ESPHome and Home Assistant.
At this time, I'm not tring to support the SD card.

Link to tested product https://www.amazon.com/gp/product/B09XHJ9KRX

## Wiring ##

The SPI pins, MOSI, MISO, and SCK, are shared between the 
display and the touchscreen (and SD card reader). TOUCH_IRQ
is optional. If you don't wire it, make sure you comment out 
touchscreen.interrupt_pin. The SD card pins are not
connected.

These may not be optimum wiring, but I've tested them.

LCD to ESP32 (30 pin or d1 mini) wiring:
* SCK/CLK to 18
* MISO/DO/SDO to 19
* MOSI/DIN/SDI to 23
* DISP_CS to 33
* DISP_LED to 17
* TOUCH_CS to 32
* DISP_DC to 5
* DISP_RESET to 16
* TOUCH_IRQ 21
* VCC to 3V3
* GND to GND
