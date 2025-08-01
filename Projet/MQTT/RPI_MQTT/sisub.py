import paho.mqtt.client as mqtt

# THE_BROKER = "iot.eclipse.org"
THE_BROKER = "192.168.1.95"
# THE_TOPIC = "$SYS/#"
THE_TOPIC = "/inTopic"
CLIENT_ID = ""


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected to ", client._host, "port: ", client._port)
    print("Flags: ", flags, "returned code: ", rc)
    client.subscribe(THE_TOPIC, qos=0)


# The callback for when a message is received from the server.
def on_message(client, userdata, msg):
    print(
        "sisub: msg received with topic: {} and payload: {}".format(
            msg.topic, str(msg.payload)
        )
    )


# client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=CLIENT_ID, clean_session=True, userdata=None, protocol=mqtt.MQTTv311, transport="tcp")
client = mqtt.Client(
    client_id=CLIENT_ID,
    clean_session=True,
    userdata=None,
    protocol=mqtt.MQTTv311,
    transport="tcp",
)

client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set(None, password=None)
client.connect(THE_BROKER, port=1883, keepalive=60)

# Blocking call that processes network traffic, dispatches callbacks and handles reconnecting.
client.loop_forever()
