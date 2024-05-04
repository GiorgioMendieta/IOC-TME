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

## SSH key on RPI

I flashed the RPI using the RPI imager
I then used SSH to configure the RPI with my Macbook
I noticed i could ssh with the RPI IP address or using `raspberrypi.local`

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

## MQTT (Raspberry Pi)

Then i installed `mosquitto`and `mosquitto-clients` with `apt get`
I started the service with `systemctl start mosquitto.service`
Then I verified the hostname with `hostname` and `hostname -I`

I configured the `mosquitto.conf` file located at `/etc/mosquitto` by adding the following options:

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

On the raspberry, I ran into a problem using MQTTv5. Apparently on_connect() needed 5 arguments but there were only 4 being given.
While researching I stumbled upon this (<https://github.com/eclipse/paho.mqtt.python/issues/575>)
The problem was fixed by passing an empty properties *props* parameter

```python
def on_connect(client, userdata, flags, rc, props=None):
```

## HTTP Server

**Board**: Raspberry Pi 3
**OS**: RPI OS 64-bit
**Framework**: Flask

Created virtual environment named venv with `python3 -m venv ./venv`
Then I activated the virtual environment to be able to install packages without breaking the main python installation with `source ./venv/bin/activate`
Installed flask with `sudo pip3 install flask`
Installed paho-mqtt with `sudo pip3 install paho-mqtt`

When testing the server on my local machine I had some problems with socket timeouts. I debugged and the problem was the MQTT connection since I had not started the mosquitto service on my Macbook

I then used Bootstrap 5.3 to use its embedded CSS styles to simplify the webpage design.

## SQLite3 Database

Installed with `sudo apt install sqlite3`
I'm going to use phpliteadmin to make managing the DB easier
Dependencies `php-mbstring`, `apache2`, `php`, `libapache2-mod-php`
Enabled and started the apache2 service `sudo systemctl enable apache2` and `sudo systemctl start apache2`
Entered the default directory for the server `cd /var/www/html/` and created a `database` directory
I then downloaded phpLiteAdmin from bitbucket using `sudo wget https://bitbucket.org/phpliteadmin/public/downloads/phpLiteAdmin_v1-9-8-2.zip`, unzipped the file and copied the default configuration file

Default password for the DB is "admin" without quotes, and I specified the DB path `/home/giorgio/Developer/IOC-TME/Projet/Database` on the phpLiteAdmin config file located at `/var/www/html/database/phpliteadmin.config.php`

I then accessed the page using the RPI local IP.
(Note: The local IP can be known with the following command `hostname -I`), in this case it was **192.168.1.3**

<http://192.168.1.3/database/phpliteadmin.php>

However there was an error, I was getting a blank page.
I checked out the logs in `/var/log/apache2/error.log` and after googling the error apparently I was using the wrong phpLiteAdmin version that is not compatible with **PHP8.2**, so I downloaded the correct one using `sudo winget http://www.phpliteadmin.org/phpliteadmin-dev.zip`, redid the installation steps and the page now worked correctly.

I logged in using the default password and begun creating the DB table structure by executing the following SQL query.

```sql
CREATE TABLE "IOTSensors" ( ID INTEGER PRIMARY KEY, deviceName TEXT,'sensor' TEXT,'reading' INT ,'timestamp' DATETIME)
```

I then verified the table schema with the `.fullschema` command inside the sqlite3 cli

```sh
giorgio@raspberrypi:/databases $ sqlite3 ioc_project.db
SQLite version 3.40.1 2022-12-28 14:03:47
Enter ".help" for usage hints.
sqlite> .fullschema
CREATE TABLE IF NOT EXISTS "IOTSensors" ( ID INTEGER PRIMARY KEY, deviceName TEXT,'sensor' TEXT,'reading' INT ,'timestamp' DATETIME);
/*No STAT tables available*/
```

After running the server using `python app.py` I ran into some trouble regarding the database. I got a problem regarding "attempt to write to a readonly database". After looking on the internet apparently it was a problem regarding the permissions of the database file AND the directory where it was located.

I then created another directory and changed the permissions and then it worked correctly
