# iot-temperature-control

The temperature control device used here is described [in this page](https://www.notion.so/Universitas-Pertamina-Temperature-Control-Device-02b5a889e17d4ee9ae5521881e55af0d). [This link](https://github.com/auralius/up_temperature_control_device) leads to the GitHub repository of the device. Details on the PID control can be found [here](https://github.com/auralius/arduino-pid-template/tree/thermal).

Since the temperature control device is built on top of an Arduino Uno, we add an ESP8266 to allow internet communications.

## The Scenario
* We only use one of the two available heaters.
* There are two exchanged variables in the overall system:
  * ```PV```: process value, which is the current temperature of the heater.
  * ```SV```: set value, which is the desired (reference) temperature of the heater.
* A PID control loop is running (1kHz-realtime) in the Arduino Uno.
* Arduino Uno sends ```PV``` to ESP8266. ESP8266 publishes ```PV```.  MQTT server subscribes ```PV```.
* MQTT server publishes ```SV```. ESP8266 subscribes ```SV```. ESP8266 sends ```SV```to Arduino Uno.
* Communication loop between Arduino Uno and ESP8266 runs at a much lower rate (less than 1 Hz, not realtime).

## The System
<img src="https://github.com/auralius/iot-temperature-control/blob/main/images/system.png" alt="Alt Text" style="width:30%; height:auto;">

## The Concepts
![](https://github.com/auralius/iot-temperature-control/blob/main/images/concept.png)

## Arduno Uno - ESP8266 Serial Communication

![](https://github.com/auralius/iot-temperature-control/blob/main/images/wiring.png)
