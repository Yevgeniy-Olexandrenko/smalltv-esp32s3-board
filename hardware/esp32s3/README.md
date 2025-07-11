<h2>PCB v1.0</h2>

First production version, contains the following hardware:
- ESP32-S3 WI-FI/BT module
- LDO converter +5V to +3.3V
- Onboard programmable LED
- Reset & Boot buttons
- Expansion headers
- Ready for up to 4 touch buttons
- USB Type-C port
- Micro SD Card connector
- LCD ST7789 10p connector
- LCD backlight control circuit
- Input voltage measurement circuit
- I2S decoder and amplifier
- PDM MEMS microphone

Top|Bottom
-|-
<img src="v1.0/SmallTV-ESP32-S3-Board-v1.0_PhotoTop.svg" width="480px">|<img src="v1.0/SmallTV-ESP32-S3-Board-v1.0_PhotoBottom.svg" width="480px">

***NOTE:** If the device is used without any expansion boards, pins VIN and +5V must be shorted with a jumper.*

ESP32-S3 module connections

Pin|Signal|Bus|Signal|Component
-|-|-|-|-
1|GND|Power|GND|-
2|3V3|Power|+3V3|
3|EN|-|-|Reset circuit
4|GPIO4|-|IO4|Touch button 0
5|GPIO5|-|IO5|Toush button 1
6|GPIO6|-|IO6|Toush button 2
7|GPIO7|-|IO7|Touch button 3
8|GPIO15|I2S|DIN|Amplifier
9|GPIO16|I2S|BCLK|Amplifier
10|GPIO17|I2S|RLCLK|Amplifier
11|GPIO18|PDM|CLK|Microphone
12|GPIO8|PDM|DATA|Microphone
13|GPIO19|USB|D-|USB connector
14|GPIO20|USB|D+|USB connector
15|GPIO3|Power|-|Input Voltage Sense
16|GPIO46|-|-|N.C.
17|GPIO9|-|RES|Display
18|GPIO10|SPI|CS|Display
19|GPIO11|SPI|MOSI|Display
20|GPIO12|SPI|SCK|Display


