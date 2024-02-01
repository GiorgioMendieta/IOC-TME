# TP1 Driver LED et BP

## Auteurs

- THIRION Lou
- MENDIETA OROZCO Jorge Alberto

## 2. Hello World! RaspberryPi

Nous avons eu de problèmes en essayant de nous connecter en utilisant `ssh` avec une machine depuis l'extérieur. Nous avons donc changé à une machine de la salle SESI afin de nous connecter sur la carte Raspberry Pi N° 25.

`ssh -p 62225 pi@peri`

Nous avons ensuite crée le répertoire `~/thirion_mendieta/` dans la racine du Raspberry Pi pour sauvegarder les exécutables crées.

### Questions

1. Pourquoi passer par la redirection des ports ?

    Parce que la configuration du réseau a été configuré pour router les paquets entrants sur le port `62225` de l'adresse `132.227.71.43` (configuré par le DNS comme `peri`) depuis le côté du laboratoire, vers la carte RaspberryPi trouvé dans un sous-réseau (adresse`192.168.1.25`) qui a un port interne `25`, celui le serveur ssh écoute les paquets entrants.

2. Pourquoi faut-il que vos fichiers soit dans un répertoire propre sur une RaspberryPi ?

    Afin de mantenir une structure propre car plusieurs personnes travailleront sur une même carte.

## 3. Configuration des clés ssh

On a crée un pair de clés publique/private afin de nous connecter à la carte en utilisant le protocole ssh. Après, nous avons copié la clé publique `id_rsa.pub` à la carte afin de ne taper pas le mot de passe à chaque fois qu'on se connecte.

- La commande `ssh-keygen -t rsa` crée le pair de clés.
- `ssh-copy-id -i $HOME/.ssh/id_rsa.pub -p 62225 pi@peri` copie la clé publique à la carte

## 4. Prise en mains des outils de développement: Hello World

Nous avons ajouté sur le PATH trouvé dans le fichier `.bashrc` le chemin `source /users/enseig/franck/E-IOC/export_rpi_toolchain.sh` afin de pouvoir utiliser le compilateur croisé.

Après, nous avons crée un simple programme qui affiche "Hello world" dans la sortie standard. Grâce au fichier makefile, nous avons utilisé le compilateur croisé pour le **SoC BCM2708**, et en utilisant la commande `scp` (secure copy) nous avons deplacé le binaire dans notre répertoire dans la RPI.

Finalement, pour exécuter le programme nous avons exécuté la commande suivante dans le terminal :
`sudo ./helloworld.x`

### Questions

1. Qu'est-ce que c'est un compilateur croisé ?

    Un compilateur croisé est un outil de développement logiciel qui permet de compiler du code source sur une plateforme ou architecture différente de celle sur laquelle il sera exécuté.

## 5. Contrôle de GPIO en sortie (blink0.c)

Nous avons compilé le fichier blink0.c, et l'executé dans la RPI avec des différents arguments de la periode de clignotement afin de voir si effectivement la LED clignotait plus ou moins rapidement.

### Questions sur le code

1. Expliquez pourquoi, il pourrait être dangereux de se tromper de broche pour la configuration des GPIO.

    Parce que si on se trompe de broche on risque d'endommager la carte. Par exemple, un port GPIO peut être connecté à une composant électronique qui peut demander plus de courant que la carte peut fournir, dans ce cas il y existe un risque d'endommagement pour la Raspberry.

    Un port peut être configué en tant que sortie mais si on branche des composants comme une configuration en entrée
2. A quoi correspond l'adresse `BCM2835_GPIO_BASE` ?

    `BCM2835_GPIO_BASE` est l'adresse `BCM2835_PERIPH_BASE` (`0x2000 0000`), correspondant aux péripheriques (I/O Peripherals) dans l'adresse physique du noyau. L'adresse du bus des péripheriques est `0x7E00 0000`.

    Apès, on ajoute un offset (`0x0020 0000`). On utilise donc l'adresse du bus `0x7E20 0000` (adresse physique `0x2020 0000`), correspondant à la sélection des fonctions pour les ports GPIO.

3. Que représente la structure `struct gpio_s` ?

    Cette structure décrit la carte des registres pour accèder plus proprement aux registres de configuration des GPIOs.

4. Dans quel espace d'adressage est l'adresse `gpio_regs_virt` ?

    L'espace d'adressage de gpio_regs_virt correspond à l'adresse des péripheriques d'entrée/sortie `0x2020 0000`

5. Dans la fonction `gpio_fsel()`, que contient la variable `reg` ?

    Les GPIO Function Select Registers (GPFSELn) contiennent chaqu'un les fonctions de dix GPIOs. Donc une façon facile d'obtenir le registre GPFSELn correspondant au pin est de diviser par 10 et prendre la partie entière.

6. Dans la fonction `gpio_write()`, pourquoi écrire à deux adresses différentes en fonction de la valeur `val` ?

    La Raspberry Pi a deux registres qui servent à mettre un GPIO à 1 (Set) ou à 0 (Clear):

    - GPIO Pin Output Set Registers (GPSETn)
    - GPIO Pin Output Clear Registers (GPCLRn)

7. Dans la fonction `gpio_mmap()`, à quoi correspondent les flags de `open()` ?

    ```c
    mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
    ```

   - `O_RDWR` : Ouvre le répertoire en lecture et écriture
   - `O_SYNC` : Synchronize I/O file integrity

8. Dans la fonction `gpio_mmap()`, commentez les arguments de `mmap()`.

    ```c
    mmap_result = mmap(NULL, RPI_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, BCM2835_GPIO_BASE);
    ```

    - `NULL` : Commence à l'adresse 0
    - `RPI_BLOCK_SIZE` : Utilise une taille de block de 4 KB
    - `PROT_READ | PROT_WRITE` : Permet lire et écrire le contenu de la zone mémoire
    - `MAP_SHARED` : Les modifications sont partagées
    - `mmap_fd` : Descripteur de fichier obtenu par `open()`
    - `BCM2835_GPIO_BASE` : Offset correspondant au adresse base des périphériques de la RPI

9. Que fait la fonction `delay()` ?

    Cette fonction fait appel au `nanosleep()` (cf. <https://man7.org/linux/man-pages/man2/nanosleep.2.html>) afin d'attendre la durée specifié en milisecondes.

10. Pourquoi doit-on utiliser `sudo` ?

    Parce que les droits d'accès des utilisateurs normales ne sont suffisantes pour éxecuter les programmes crées. Par exemple, la fonction memory map (`mmap()`) tape sur une zone mémoire restreint aux utilisateurs qui n'ont pas des droits élevés car cela peut être dangereuse pour le système.

## 6. Contrôle de plusieurs GPIO en mode "sortie" (blink0_pt.c, blink01_pt.c)

En premier lieu, nous devons faire quelques changements au code pour utiliser des threads avec les normes POSIX ([source](https://franckh.developpez.com/tutoriels/posix/pthreads/)):

Pour pouvoir compiler un projet (tous systèmes) avec `pthread`, il faut pour commencer :

- Ajouter l'en-tête :
    `#include <pthread.h>`
- Ajouter à l'éditeur de lien la bibliothèque :
    `-lpthread`
- Spécifier au compilateur la constante :
    `-D_REENTRANT`

### blink0_pt.c

Nous avons utilisé le [site suivante](https://hpc-tutorials.llnl.gov/posix/passing_args/) (Cf. Partie 2) afin de comprendre mieux comment passer en argument des plusieurs parametres à un thread.

En premier lieu, nous avons crée une structure globale qui contient les paramètres pour le thread de clignotement :

```c
// Structure to send parameters to threads
struct blink_params
{
    int led;
    int half_period;
};
```

Ensuite, nous avons crée un thread (`pthread`) en lui envoyant une structure (`blink1_args`) en tant qu'argument dans le `main()`.

```c
// Create the thread and pass the structure containing the values as argument
pthread_t thread;
int ret = pthread_create(&thread, NULL, blink, (void *)&blink1_args);
```

Ensuite nous avons configuré le pin en tant que sortie pour le LED 1 (GPIO 17) et nous avons éxecuté la fonction de clignotement avec la fonction de clignotement `blink()`.

```c
void *blink(void *threadarg)
{
    // Each thread receives a unique instance of the structure
    struct blink_params *args;
    args = (struct blink_params *)threadarg;
    int led = args->led;
    int half_period = args->half_period;

    // Setup GPIO to output
    gpio_fsel(led, GPIO_FSEL_OUTPUT);

    // Begin alternating the value
    uint32_t is_on = 0;
    while (1)
    {
        gpio_write(led, is_on);
        delay(half_period);
        is_on = 1 - is_on; // is_on = not is_on
    }
}
```

### blink01_pt.c

Pour les deux threads, le principe restait le même sauf que désormais on utilisait un thread par LED, en les envoyant deux structures avec des paramètres differentes pour varier la fréquence de clignotement de chaque GPIO.

## 7. Lecture de la valeur d'une entrée GPIO

### read_bp.c

En nous basant sur le code, nous avons crée une fonction pour lire la valeur d'un GPIO. On lit sur le GPIO Pin Level Registers (GPLEVn), et on fait une masque afin d'obtenir la valeur du GPIO souhaité.

```c
value = gpio_regs_virt->gplev[reg] & (1 << bit);
```

### blink01_bp_pt.c

Au début on avait éxecutait les instructions suivantes dans le thread principal `main()`. Néanmoins, cela a fait que la LED s'allumait seulement quand le bouton restait pressé. Si on relachaît le bouton, la LED se éteint.

```c
while(1)
{
    if(BP_ON){
        BP_ON = 0;
        gpio_write(GPIO_LED0, 1);
    }
    if(BP_OFF){
        BP_OFF = 0;
        gpio_write(GPIO_LED0, 1);
    }
}
```

Puis on l'a corrigé pour que la LED change de valeur a chaque "front montant" du bouton et pas à chaque changement d'état.

```c
uint32_t led_on = 0;
while(1)
{
    if(BP_ON){
        BP_ON = 0;
        gpio_write(GPIO_LED0, led_on);
        led_on = 1 - led_on;
    }
    if(BP_OFF){
        BP_OFF = 0;
    }
}
```

## Lab1+

- `blink02_pt.c` clignote deux LEDs à deux fréquences différentes définis par l'utilisateur en tant qu'arguments
- `blink03_pt.c` clignote deux LEDs avec la même fréquence en utilisant le bouton poussoir pour changer la phase du deuxième LED
