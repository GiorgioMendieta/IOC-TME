import paho.mqtt.client as mqtt

# Création de la fifo vers le server
s2fName = '/tmp/s2f_TM'
s2f = open(s2fName,'w+')
#res = s2f.readline()

# Publishment

def on_connect(client, userdata, flags, rc, truc): # Callback connexion broker
    print("Connecté " + str(rc))
    #client.subscribe("hello")

#def on_message(client, userdata, msg): # Callback subscriber
#    print("Message reçu :", msg.topic, str(msg.payload))

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2) # Création d'une instance client
#client = mqtt.Client()
client.on_connect = on_connect # Assignation des callbacks
#client.on_message = on_message
client.connect("192.168.1.95", 1883, 60) # Connexion au broker

while(1):
	res = s2f.readline()
	if(res):
		client.publish("topic", res) # Publication d'un message
#client.loop_forever() # pour attendre les messages
