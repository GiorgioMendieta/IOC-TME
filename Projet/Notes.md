# Notes

## MQTT (Client ESP32)

**Board**: ESP32 (TTGO-Lora-OLED V1)
**Configuration**: Allow anonymous (pas de login)

Multi tasking approach using timers and structures
Photoresistance publish every 0.5 seconds

First i used a naive approach to publish a message every 2 seconds
Then i used a mix of the multi tasking as seen on TP and my naive approach, however it didn't work as well since the naive approach blocked the task
Finally I settled for the multi tasking approach and publish a message using the task
Then I implemented the OLED screen to show the SSID, IP address and Photoresistance value
I refactored the code by making it modular using header files

I have some difficulties with the MQTT broker on my Macbook since I still haven't flashed the RPI

## MQTT (Raspberry Pi)

I flashed the RPI using the RPI imager
I then used SSH to configure the RPI with my Macbook
I noticed i could ssh with the RPI IP address or using `raspberrypi.local`
Then i installed `mosquitto`and `mosquitto-clients` with `apt get`
I started the service with `systemctl start mosquitto.service`
Then I verified the hostname with `hostname` and `hostname -I`

I configured the `mosquitto.conf` file by adding the following options:

```
allow_anonymous true
listener 1883
````

Then I reset the RPI, and the MQTT broker worked correctly!!!

I verified with `sudo netstat -tlnp | grep 1883`
The result was:

```sh
tcp        0      0 0.0.0.0:1883            0.0.0.0:*               LISTEN      684/mosquitto
tcp6       0      0 :::1883                 :::*                    LISTEN      684/mosquitto
```

I tested this using an app on my phone called MyMQTT

## HTTP Server

**Board**: Raspberry Pi 3
**OS**: RPI OS 64-bit
**Framework**: Flask

Created virtual environment named venv with `python3 -m venv ./venv`
Then I activated the virtual environment to be able to install packages without breaking the main python installation with `source ./venv/bin/activate`
Installed flask with `sudo pip3 install flask`
Installed paho-mqtt with `sudo pip3 install paho-mqtt`

## SQLite3 Database

Installed with `sudo apt install sqlite3`

## SSH key on RPI

I created a SSH key with `ssh-keygen -b 2048 -t rsa` to be able to clone the repository from my github account
I had some trouble logging in via ssh config, then i noticed I was sharing the public key (.pub) when i needed to share the private key

```sh
Host rpi
  User giorgio
  Hostname raspberrypi.local
  AddKeysToAgent yes
  UseKeychain yes
  IdentityFile ~/.ssh/id_ed25519
```
