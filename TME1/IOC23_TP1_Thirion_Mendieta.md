# TP1 Driver LED et BP

## Auteurs

- THIRION Lou
- MENDIETA OROZCO Jorge Alberto

## Réponses aux questions du TME (s'il y en a)

- Ecrire un résumé de la question 1 (en moins d'une phrase)
- réponse 1…
- résumé de la question2
- réponse 2
- peut-être sur plusieurs lignes

## Expériences réalisées

### Titre de l'expérience

- Décrire sous forme d'une liste les étapes
- éventuellement les sous-étapes 1
- une sous étape 2
- etc
- Une nouvelle étape de l'expériences
- etc.

# CR

Nous avons eu de problèmes en essayant de nous connecter en utilisant `ssh` avec une machine depuis l'extérieur. Nous avons donc changé à une machine de la salle SESI afin de nous connecter sur la carte Raspberry Pi N° 25

Nous avons ensuite crée le répertoire `thirion_mendieta` dans la racine du Raspberry (`~`)

## 2. Hello World! RaspberryPi

### Questions

1. Pourquoi passer par la redirection des ports ?

Parce que la configuration du réseau a été configuré pour router les paquets entrants sur le port `62225` de l'adresse `132.227.71.43` (configuré par le DNS comme `peri`) depuis le côté du laboratoire, vers la carte RaspberryPi trouvé dans un sous-réseau (adresse`192.168.1.25`) qui a un port interne `25`, celui le serveur ssh écoute les paquets entrants.

2. Pourquoi faut-il que vos fichiers soit dans un répertoire propre sur une RaspberryPi ?

Afin de mantenir une structure propre car plusieurs personnes travailleront sur une même carte.

## 3. Configuration des clés ssh

On a crée un pair de clés publique/private afin de nous connecter à la carte en utilisant le protocole ssh. Après, nous avons copié la clé publique à la carte afin de ne taper pas le mot de passe à chaque fois qu'on se connecte.

- La commande `ssh-keygen -t rsa` crée le pair de clés.
- `ssh-copy-id -i $HOME/.ssh/id_rsa.pub -p 62225 pi@peri` copie la clé publique à la carte

## 4. Prise en mains des outils de développement: Hello World

Nous avons ajouté sur le PATH le chemin `source /users/enseig/franck/E-IOC/export_rpi_toolchain.sh` afin de pouvoir utiliser le compilateur croisé.

Après, nous avons crée un simple programme qui affiche "Hello world" dans la sortie standard. Grâce au fichier makefile, nous avons utilisé le compilateur croisé pour le SoC BCM2708, et en utilisant la commande `scp` (secure copy) nous avons deplacé le binaire dans notre répertoira dans la RPI.

### Questions

1. Qu'est-ce que c'est un compilateur croisé ?

...
