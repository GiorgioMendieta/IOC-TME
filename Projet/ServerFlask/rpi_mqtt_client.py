# MQTT Publisher example

import paho.mqtt.client as mqtt
import sys

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
	if rc != 0:
        print(f"Connected fail with code {rc}")
		return
	print("Connected success")
	print("Connected to ", client._host, "port: ", client._port)
    # Subscribe to topics
	client.subscribe("esp32/photoVal")


def on_disconnect(client, userdata, rc):
	print("Disconnected from ", client._host, "port: ", client._port)
	print("Returned code: ", rc)

# Callback function for when a message is received from the server.
def on_message(client, userdata, msg):
	print("sisub: msg received with topic: {} and payload: {}".format(msg.topic, str(msg.payload)))

MQTT_BROKER = "192.168.1.95"
CLIENT_ID = "RPIClient-1"

# Création de la fifo vers le server
s2fName = '/tmp/s2f_TM'
s2f = open(s2fName,'w+')

# Version 2 of the callback API since the version 1 is deprecated
mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# Callback assignment
mqttc.on_connect = on_connect
mqttc.on_disconnect = on_disconnect
mqttc.on_message = on_message

mqttc.username_pw_set(None, password=None)
if mqttc.connect(MQTT_BROKER, port=1883, keepalive=60) != 0:
	print("Connection failed")
	sys.exit(1)

# Subscribe to topic after connection
mqttc.subscribe("esp32/photoVal")

# client.loop_start()

# while(1):
# 	res = s2f.readline()
# 	if(res):
# 		# Publish message form fifo
# 		client.publish("rpi/broadcast", res)

# mqttc.loop_forever() # pour attendre les messages
