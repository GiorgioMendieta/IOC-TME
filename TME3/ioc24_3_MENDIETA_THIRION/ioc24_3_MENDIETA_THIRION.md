# TP3 - Pilotage d'un écran LCD en mode utilisateur et par un driver

## Auteurs

- Lou THIRION
- Jorge MENDIETA

## 1. Configuration des GPIO pour le pilotage de l'écran
Dans le tableau ci-dessous, nous pouvons voir le mapping des GPIO pour les différents signaux de contrôle de l'écran LCD :


| Signal LCD | GPIO    |
| ---------- | --------|
|  RS        |   7     |
|  E         |  27     |
| D4, D5, D6, D7     | 22, 23, 24, 25     |


- Le signal **RS** correspond au contrôle de l'écriture sur le LCD, il vaut 1 pour l'envoie d'une donnée et 0 pour une commande.
- Le signal **E** permet de valider les données qui se trouve sur le bus, il est pris en compte lors de son front descendant.
- **D4, D5, D6, D7** sont les signaux du bus de données.

### Questions

*Comment faut-il configurer les GPIOs pour les diférents signaux de l'afficheur LCD ?*


*Comment écrire des valeurs vers le LCD ?*

Pour écrire des valeurs vers le LCD, on doit d'abord mettre le signal RS à 1, puis mettre nos données sur le bus via les signaux D4, D5, D6 et D7 et enfin mettre le signal E à 1 puis à 0 avec un certain délais pour générer un front déscendant.


*Quelles valeurs doivent être envoyées vers l'afficheur pour réaliser l'initialisation ?*



*Comment demander l'affichage d'un caractère ?*

*Comment envoyer des commandes telles que : l'effacement de l'écran, le déplacement du curseur, etc. ?*



## 2. Fonctionnement de l'écran et fonctions de base

Nous avons toutes les fonctions dont nous avons besoin. Maintenant regardons d'un peu plus près la phase d'initialisation de l'afficheur LCD. Au démarrage, l'afficheur est dans un mode non défini (8 bits ou 4 bits). Il faut donc le forcer en mode 4 bits.

### Questions

*Q1: A quoi sert le mot clé volatile*
Le mot clé volatile sert à signifier au compilateur de n'effectuer aucune optimisation.

*Q2: Expliquer la présence des drapeaux dans `open()` et `mmap()`.*

Pour la fonction **open()**:

```c
int mmap_fd = open("/dev/mem", O_RDWR | O_SYNC);
```

- Le flag `O_RDWR` signifie qu'on ouvre le fichier en lecture et écriture. 
- Le flag `O_SYNC` permet de garantir l'intégrité du fichier. Au moment où `write()` (cf. `man 2 write` ou un appel similaire) renvoie une donnée, cette donnée et les métadonnées associées au fichier ont été transmises au matériel sur lequel s'exécute l'appel.

Pour la fonction **mmap()**: 

```c
gpio_regs = mmap(NULL,
                 RPI_BLOCK_SIZE,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED,
                 mmap_fd,
                 RPI_GPIO_BASE);
```

- Flag `PROT_READ | PROT_WRITE`: On peut lire et écrire le contenu de la zone mémoire.
- Flag`MAP_SHARED`: Cet indicateur est utilisé pour partager le mappage avec tous les autres processus mappés sur cet objet. Les modifications apportées à la région de mappage seront réécrites dans le fichier.

(Cf. [cette page pour plus d'information](https://linuxhint.com/using_mmap_function_linux/))

*Q3: pourquoi appeler `munmap()` ?*

Pour supprimer le mapping créé par la fonction gpio_setup et ainsi éviter des fuites de mémoire en libérant des ressources.

*Q4: Expliquer le rôle des masques : `LCD_FUNCTIONSE`, `LCD_FS_4BITMOD`, etc.*

Ces différents masques permmettent d'envoyer différentes commandes au contrôleur LCD.

|      Masques des commandes      |             Descriptions                                    |
|-----------------------|-------------------------------------------------------------|
|    LCD_FUNCTIONSET    |             fonctions de configuration du LCD               |
|    LCD_FS_4BITMODE    | sélectionne le mode 4 bits pour le bus (bit DL dans la doc) |
|      LCD_FS_2LINE     | sélectionne les deux lignes (bit N dans la doc)             |
|     LCD_FS_5x8DOTS    |        (bit F dans la doc)                                  |
|   LCD_DISPLAYCONTROL  |             commandes de contrôle du LCD                    |
|    LCD_DC_DISPLAYON   |          écran allumé (bit D dans la doc)                   |
|    LCD_DC_CURSOROFF   |          écran éteint (bit D dans la doc)                   |
|    LCD_ENTRYMODESET   |               configuration du mode du LCD              |
|      LCD_EM_RIGHT     |               déplacement du curseur                  |
| LCD_EM_DISPLAYNOSHIFT |              l'écran reste fixe (pas de shift)           |


*Q5: Expliquez comment fonctionne cette fonction.*

```c
void lcd_message(const char *txt){}
```

L'objectif est d'afficher une chaine de caractère à l'écran.
On commence par définir l'adresse de la DDRAM à laquelle on va écrire notre message,
cette adresse est en réalité sous la forme d'un tableau de 4 adresses correspondant aux 4 lignes de l'afficheur LCD.
Pour chacune de ces adresses, on appelle la commande SETDDRAMADDR pour siginifier au contrôleur que l'on va écrire ou lire à cette adresse. Puis, pour chaque caractère de la chaine, on envoit la donnée à afficher. Lorsque on arrive à la fin de la ligne, on change d'addresse pour passer à ligne suivante et on continue jusqu'à la fin de la chaine ou bien qu'il n'y est plus de place sur l'afficheur.

## 3. Driver pour le LCD

Nous avons repris le pilote des leds (TP2) et puis, nous avons adapté la commande `write` afin de pouvoir écrire dans l'écran LCD branché a la carte RPI.

```shell
$ echo "Bonjour" > /dev/lcd_MT
```
