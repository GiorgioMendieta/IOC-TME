# MQTT (Client ESP32)

Board: ESP32 (TTGO-Lora-OLED V1)

Multi tasking approach using timers and structures
Photoresistance publish every 0.5 seconds

First i used a naive approach to publish a message every 2 seconds
Then i used a mix of the multi tasking as seen on TP and my naive approach, however it didn't work as well since the naive approach blocked the task
Finally I settled for the multi tasking approach and publish a message using the task
Then I implemented the OLED screen to show the SSID, IP address and Photoresistance value
I refactored the code by making it modular using header files
