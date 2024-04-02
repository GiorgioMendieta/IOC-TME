import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc): # Callback connexion broker
    printf("Connecté " + str(rc))
    client.subscribe("hello")

def on_message(client, userdata, msg): # Callback subscriber
    printf("Message reçu : {msg.topic} {str(msg.payload)}")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2) # Création d'une instance client
#client = mqtt.Client()
client.on_connect = on_connect # Assignation des callbacks
client.on_message = on_message
client.connect("192.168.43.173", 1883, 60) # Connexion au broker

#client.publish("hello", "Salut ma poule") # Publication d'un message
client.loop_forever() # pour attendre les messages
