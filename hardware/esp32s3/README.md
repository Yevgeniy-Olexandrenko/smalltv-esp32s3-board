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

Pin|Signal|Bus|Signal|Component|Pin|Signal|Bus|Signal|Component
-|-|-|-|-|-|-|-|-|-
1|GND|Power|GND|Ground|21|GPIO13|-|DC|Display
2|3V3|Power|+3.3V|Power|22|GPIO14|-|BL|Display backlight circuit
3|EN|-|-|Reset circuit|23|GPIO21|MMC|D1|SD Card
4|GPIO4|-|IO4|Touch button 0|24|GPIO47|-|DET|SD Card detect
5|GPIO5|-|IO5|Toush button 1|25|GPIO48|-|-|N.C.
6|GPIO6|-|IO6|Toush button 2|26|GPIO45|-|-|N.C.
7|GPIO7|-|IO7|Touch button 3|27|GPIO0|-|-|Boot button circuit
8|GPIO15|I2S|DIN|Amplifier|28|GPIO35|-|-|N.C.
9|GPIO16|I2S|BCLK|Amplifier|29|GPIO36|-|-|N.C.
10|GPIO17|I2S|RLCLK|Amplifier|30|GPIO37|-|-|N.C.
11|GPIO18|PDM|CLK|Microphone|31|GPIO38|MMC|CMD|SD Card
12|GPIO8|PDM|DATA|Microphone|32|GPIO39|MMC|D3|SD Card
13|GPIO19|USB|D-|USB connector|33|GPIO40|MMC|D2|SD Card
14|GPIO20|USB|D+|USB connector|34|GPIO41|MMC|CLK|SD Card
15|GPIO3|Power|-|Input Voltage Sense|35|GPIO42|MMC|D0|SD Card
16|GPIO46|-|-|N.C.|36|GPIO43|UART|RXD|Expansion port
17|GPIO9|-|RES|Display|37|GPIO44|UART|TXD|Expansion port
18|GPIO10|SPI|CS|Display|38|GPIO2|I2C|SDA|Expansion port
19|GPIO11|SPI|MOSI|Display|39|GPIO1|I2C|SCL|Expansion port
20|GPIO12|SPI|SCK|Display|40|GND|Power|GND|Ground


