# CookerCheck-ESP32
A mini house device to alarm when users forget to turn the heater off. Works with inductive, IR and gas heaters.
Components:

- ESP32
- DHT11 humidity & temperature sensor
- PIR proximity sensor
- RGB LED diode showing working/temperature status
- Buzzer

Wiring:
![image](https://github.com/vasjamarkic/CookerCheck-ESP32/blob/main/Sheme_ESP32_CC.png)\
Pics of the device:

<a href="url"><img src="https://github.com/vasjamarkic/CookerCheck-ESP32/blob/main/PIC1.jpg" align="right" width="450" ></a>

Functions:
- automatic connect to WiFi hotspot
- alarms when the temperature rises above starting reference temperature (blinking RED led)
- alarm with sound when the user is not in the visual area of the PIR sensor
- auto deep sleep the device when the temperature goes above treshold (heaters are off)
- power via USB cable (testing version)
- blue LED status on ESP32 board for WiFi connection


More pics:
![image](https://github.com/vasjamarkic/CookerCheck-ESP32/blob/main/PIC2.jpg)
![image](https://github.com/vasjamarkic/CookerCheck-ESP32/blob/main/PIC3.jpg)
![image](https://github.com/vasjamarkic/CookerCheck-ESP32/blob/main/PIC4.jpg)
