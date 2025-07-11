# Compte Rendu TME2

## Driver pour les LEDs et le bouton poussoir

Nous allons créer un pilote pour contrôler les LEDs sans devoir exécuter le programme avec droits d'administrateur avec `sudo`. Le pilote sera sauvegardé dans le répertoire des dispositifs `/dev/led0_TM`.

## Étape 1 : création et test d'un module noyau

### Questions

- *Quelle fonction est exécutée lorsqu'on insère le module du noyau ?*
    `mon_module_init()`

- *Quelle fonction est exécutée lorsqu'on enlève le module du noyau ?*
    `mon_module_cleanup()`

Nous avons compilé le module en utilisant le librairie du noyau (`linux-rpi-3.18.y`) de la Raspberry Pi. Ensuite, nous avons inseré le module avec les commandes suivantes :

```sh
sudo insmod ./module.ko : Insére le module compilé au noyau
lsmod : On affiche les modules installés afin de vérifier que notre module est bien là
dmesg : Affiche les messages du système informe du travail réalisé
sudo rmmod module : Décharge le module du noyau
lsmod : On affiche les modules installés afin de vérifier que notre module n'est plus là
dmesg
```

## Étape 2 : ajout des paramètres au module

### Parametre bouton

```c
static int btn;
module_param(btn, int, 0);
MODULE_PARM_DESC(btn, "numéro du port du bouton");

static int __init mon_module_init(void)
{
    printk(KERN_DEBUG "Hello World !\n");
    printk(KERN_DEBUG "btn=%d !\n", btn);
    return 0;
}
```

``` bash session
sudo insmod ./module.ko btn=18
```

Afin de vérifier blah blah

```sh
pi@raspberrypi ~/thirion_mendieta/lab2 $ modinfo module.ko
filename:       /home/pi/thirion_mendieta/lab2/module.ko
description:    Module pour contrôler une LED
author:         Jorge MENDIETA, Lou THIRION
license:        GPL
srcversion:     7277E7B09B4E23DE94A5654
depends:        
vermagic:       3.18.7 preempt mod_unload modversions ARMv6 
parm:           btn:numéro du port du bouton (int)
```

### Tableau de Parametres LEDs

```c
// led parameter
#define NBMAX_LED 32
static int leds[NBMAX_LED];
static int nbled;
module_param_array(leds, int, &nbled, 0);
MODULE_PARM_DESC(LEDS, "tableau des numéros de port LED");
```

dans la fonction init

```c
   int i;
   for (i = 0; i < nbled; i++)
       printk(KERN_DEBUG "LED %d = %d\n", i, leds[i]);

```

Après il faut recompiler et recopier le module, rmmod et insmod avec les nouveaux parametres

``` bash session
sudo insmod ./module.ko btn=18 leds=4,17
```

finalement, on peut bien vérifier le fonctionnement

``` bash session
pi@raspberrypi ~/thirion_mendieta/lab2 $ dmesg
[1029866.613523] Hello Jorge MENDIETA, Lou THIRION !
[1029866.613565] btn=18 !
[1029866.613578] LED 0 = 4
[1029866.613589] LED 1 = 17
```

### Questions

- *Comment voir que le paramètre a bien été lu ?*
    Nous pouvons afficher des messages du kernel en utilisant la fonction `printk()` afin de vérifier que le paramètre a été bien lu

## Étape 3 : création d'un driver qui ne fait rien, mais qui le fait dans le noyau

On a ajouté est 4 fonctions pour intéragir avec les leds, pour l'instant ces fonctions ne font qu'un affichage, et la structure file_operations utile à l'initialisation du module.

Nous avons dû renommer le fichier du module à `led0_MT` pour inclure nos initiaux afin d'eviter des conflicts avec les autres utilisateurs de notre carte RPI.

Après, nous avons enregistré le driver grâce la fonction `register_chrdev()` lors de la initialization du module.

Afin de sortir propement, il faut également enlever le module des dispositifs avec `unregister_chrdev()`

```c
static int __init mon_module_init(void)
{
   major = register_chrdev(0, "led0_MT", &fops_led); // 0 est le numéro majeur qu'on laisse choisir par linux
}

static void __exit mon_module_cleanup(void)
{
   unregister_chrdev(major, "led0_MT");
}
```

En exécutant la commande suivante, nous pouvons voir le numéro majeur :

``` bash session
cat /proc/devices | grep led0_MT
```

Le numéro majeur choisi par linux ete **246** dans notre cas.

Puis, il faut créer le noeud dans `/dev` et le rendre accesible pour tous les utilisateurs afin de ne pas faire sudo a chaque fois qu'on veut exécuter le fichier.

### Questions

- *Comment savoir que le device a été créé ?*
    Nous pouvons chercher sur le répertoire `/dev` et exécuter

``` bash session
pi@raspberrypi /dev $ ls -l led0_MT 
crw-rw-rw- 1 root root 246, 0 Nov 20 11:01 led0_MT
```

``` bash session
echo "rien" > /dev/led0_XY
dd bs=1 count=1 < /dev/led0_XY
dmesg | grep led0_MT
```

Grâce au string qu'on a ajouté au début de chaque message de `printk()` nous pouvons faire un grep pour trouver les messages qui appartiennet a notre module.

Nous avons crée deux scripts bash pour automatizer l'insertion (`insdev.sh`) et suppresion (`rmdev.sh`) du driver, puis nous avons les copiés vers la carte :

''' bash session
[mendieta@josquin lab2]$ scp -P 62225 insdev pi@peri:thirion_mendieta
insdev                                                                             100%  255   101.5KB/s   00:00
[mendieta@josquin lab2]$ scp -P 62225 rmdev pi@peri:thirion_mendieta
rmdev                                                                              100%  106    39.4KB/s   00:00
'''

Nous avons bien vérifié le fonctionnement de ces deux scripts en les exécutant avec `sudo`, avec le nom correct du module (`led0_MT`) et la commande `lsmod`.

### Questions

- *Expliquer comment insdev récupère le numéro major*

  - D'abord le script récupère le nom du module donné par l'utilisateur
  - Le script tilise `awk` pour chercher dans `/proc/devices` un dispositif avec le nom du module (similaire à faire un cat devices | grep module)
  - il fait un print du premier argument qui sera envoyé a la sortie standard
  - Ce premier argument qui corresponde au numero majeur sera sauvegardé dans la variable `major`

## Étape 4 : accès aux GPIO depuis les fonctions du pilote
