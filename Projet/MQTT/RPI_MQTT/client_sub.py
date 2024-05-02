# MQTT Publisher example

import paho.mqtt.client as mqtt
import sys

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
	global flag_connected
	if rc == 0:
		flag_connected = True
        print("Connected success")
		print("Connected to ", client._host, "port: ", client._port)
    else:
        print(f"Connected fail with code {rc}")

def on_disconnect(client, userdata, rc):
	global flag_connected
	flag_connected = False
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
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
flag_connected = False

# Callback assignment
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message

client.username_pw_set(None, password=None)
if client.connect(MQTT_BROKER, port=1883, keepalive=60) != 0:
	print("Connection failed")
	sys.exit(1)

# Subscribe to topic after connection
client.subscribe("esp32/photoVal")

# client.loop_start()

# while(1):
# 	res = s2f.readline()
# 	if(res):
# 		# Publish message form fifo
# 		client.publish("rpi/broadcast", res)

client.loop_forever() # pour attendre les messages
