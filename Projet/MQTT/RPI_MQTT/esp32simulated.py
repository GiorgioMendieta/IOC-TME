# Simulated ESP32 that sends messages to the RPI

import paho.mqtt.client as mqtt
import sys
import random
import time  # sleep


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected to ", client._host, "port: ", client._port)
    print("Flags: ", flags, "returned code: ", rc)


# The callback for when a message is received from the server.
def on_message(client, userdata, msg):
    print(
        "sisub: msg received with topic: {} and payload: {}".format(
            msg.topic, str(msg.payload)
        )
    )


MQTT_BROKER = "192.168.1.95"
CLIENT_ID = "ESP32Client-1"

# Version 2 of the callback API since the version 1 is deprecated
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# Assignation des callbacks
client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set(None, password=None)
if client.connect(MQTT_BROKER, port=1883, keepalive=60) != 0:
    print("Connection failed")
    sys.exit(1)

# Subscribe to topic after connection
client.subscribe("rpi/broadcast", qos=0)

while 1:
    val = random.randint(0, 100)
    # Publish message form fifo
    client.publish("esp32/photoVal", val)
    time.sleep(5)  # sleep for 5 seconds

client.loop_forever()  # pour attendre les messages
