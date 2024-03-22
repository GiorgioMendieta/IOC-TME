import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc): # Callback connexion broker
    printf("Connecté " + str(rc))
    client.subscribe("hello")

def on_message(client, userdata, msg): # Callback subscriber
    printf("Message reçu : {msg.topic} {str(msg.payload)}")

client = mqtt.Client() # Création d'une instance client
client.on_connect = on_connect # Assignation des callbacks
client.on_message = on_message
client.connect("adresse_ip_du_broker", 1883, 60) # Connexion au broker

client.publish("hello", "Salut Franck") # Publication d'un message
client.loop_forever() # pour attendre les messages