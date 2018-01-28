# DHT 11 sensor on ESP32
## DHT
Cheap temperature & humidity sensor for IOT. [Data sheet](https://akizukidenshi.com/download/ds/aosong/DHT11.pdf)

![](http://www.circuitbasics.com/wp-content/uploads/2015/12/DHT11-Pinout-for-three-pin-and-four-pin-types-2.jpg)

## ESP32
ESP32 is a series of low cost, low power system on a chip microcontrollers with integrated Wi-Fi and dual-mode Bluetooth. [ESP32 Wiki](https://en.wikipedia.org/wiki/ESP32) 
![](https://camo.githubusercontent.com/fe6b89251ae4df2628b1a4c86c57976f22d6d5ba/687474703a2f2f692e696d6775722e636f6d2f34436f584f75522e706e67)

## Setup
1. [Arduino Studio](https://www.arduino.cc/en/Main/Software) installed
2. [DHT Library](https://github.com/adafruit/DHT-sensor-library) added
3. Specify Wifi settings in code
4. Upload source to Arduino or similar
5. Wiring 
    - 5V
    - GND
    - Data (Code expects PIN 16)