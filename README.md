# README

A bootloader for the STM32F103C8.  
Based on the usb_dfu for Lisa-M-1 in opencm3-examples [1].  
Needs libopencm3.  

The bootloader waits a little bit for download to happen, than it starts the application.  
For download use dfu-util with the wait option [2] or stm32FWupgrade.  
See also  
https://github.com/j1rie/IRMP_STM32#flashing-the-bootloader-with-a-st-link  
https://github.com/j1rie/IRMP_STM32#flashing-the-firmware  

For Windows WinUSB drivers from zadig are needed.  
The firmware needs to start from APP_ADDRESS.  

Blue and Red are for legacy devices with USB-pulldown resistor, Blue_noHW and Red_noHW are for devices
without them.  

[1] https://github.com/libopencm3/libopencm3-examples/tree/master/examples/stm32/f1/lisa-m-1/usb_dfu  
[2] https://sourceforge.net/p/dfu-util/dfu-util/ci/3133d5367095cd41acc5b2e539d16fd14f354e89/  
