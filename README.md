# BtDisplay

ESP32 Based Display Unit use for displaying BG as enhancement of your phone.

![BtDisplay](https://user-images.githubusercontent.com/1705174/209117831-95d8feb6-faae-4609-977f-f2cdfb001687.jpg "BtDisplay in a pill box")

The solution uses the http webserver from xdrip.

The WiFi connection is hard coded and uses the gateway ip (your smartphone) for the connection to the xdrip webserver.

This Code is for the esp32 Wifi kit from heltec.cn.
Heltec esp32 WiFi kit has the display on board.
Using this code with other displays, the display connection must be changed.

## Why the name BtDisplay?
It was planned to use a BT connection to the phone.
Because of short time for the realisation this was changed to use WLAN share of the smartphone.

## What is it doing?
BtDisplay shows your BG values, which it gets from the xdrip webserver.
It shows the trend file and it shows when the values are outdated.

If the value is below 90, the display is inverted to draw attention to it. 

## other interesting projects
For other use cases of BT monitoring, please have a look to M5Stack Nightscout monitor
https://github.com/mlukasek/M5_NightscoutMon

#WeAreNotWaiting


