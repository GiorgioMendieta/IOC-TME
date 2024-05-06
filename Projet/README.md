# Projet IOC

Dans le cadre du projet final de l'UE du M1S2, Interface pour objets communiquants nous avons développé un serveur web embarqué dans une carte Raspberry Pi 3, qui communique avec le protocole MQTT aux clients à travers du broker Mosquitto, tout en sauvegardant ces données à l'aide d'une base de données SQLite3.

## Installation

- Cloner le repo avec `git clone`
- Remplacer les valeurs du SSID, Mot de passe et l'adresse du broker dans `/IOC-TME/Projet/MQTT/ESP32_MQTT/include/credentials.h`
- Flasher l'ESP32 avec PlatformIO avec les fichiers dans `./IOC-TME/Projet/MQTT/ESP32_MQTT/`
- Créer la base de données

## Démarrage

### Web Server

- Activer l'environnement virtuel python

```shell
source ./venv/bin/activate
```

- Exécuter le serveur web

```shell
python app.py
```

- Acceder au site web <http://192.168.1.3:8181>.

**Note**: Si l'adresse IP ne fonctionne pas, vérifier l'adresse sur la carte RPI avec `hostname -I` et remplacer l'adresse IPv4.

**Note:** Les clients ESP32 et la Raspberry Pi doivent être connectés au même SSID !

- Voilà !
