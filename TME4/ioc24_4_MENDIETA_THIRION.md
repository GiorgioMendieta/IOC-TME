# Programmation sur ESP32

* Lou THIRION 28614980
* Jorge MENDIETA 21304469

L'objectif de cette séance et de prendre en main l'ESP32 et les outils necéssaires pour la contrôler, dont l'environnement de développement pour arduino qui nous servira pour écrire les programmes pour l'ESP32.

Pour ce faire, nous allons commencer par essayer de contrôler différents périphériques, certains sont présents directment sur la carte de l'ESP32 et d'autres sont à rajouter :
* Une LED
* Un écran OLED
* Une photorésistance (sur la broche 36)
* Un buzzer (sur la broche 17)

## Installation de l'ESP32

Nous avons crée un répertoire sur le disque locale de l'ordinateur afin d'installer la librairie de l'ESP32 (esp32 by Expressif Systems v1.0.6).

Afin de programmer la carte, il faut sélectionner la version `TTGO-Lora-OLED V1`.

## Exécution multi-tâches

Les variables statiques ne peuvent pas être initialisées dans la fonction `setup_Tache()` puisque elles seront pas visibles aux autres fonctions.

En plus, quand on travaille avec plusieurs threads qui apellent à la même fonction, on aura des problèmes avec la cohérence de la valeur de la variable.



### Tâches standards

Chaque tâche est représentée par : 
* Une fonction `loop_Tache()` qui code son comportement qui sera appelée dans la fonction loop().
* Une seconde fonction `setup_Tache()` qui initialise les ressources de la tâche (périphériques) et l'état interne. 
* Une structure contenant l'état interne et le contexte d'exécution représenté par une variable globale sous forme d'une structure `Tache_t`. Cette structure est passée en argument de la tâche des fonctions `setup_Tache()` et `loop_Tache()`. 

### Gestion des tâches standard périodiques

Pour que les tâches soient périodiques nous allons utiliser la fonction waitFor. Cette fonction prend deux paramètres. Le premier est un entier nommé timer permettant d'identifier chaque tâche. Le second est un unsigned long nommé period, il correspond au temps en microsecondes que la tâche doit s'exécuter. waitFor rend la valeur 0 si il ne s'est pas écoulé une période depuis le dernier appel sinon elle rend le nombre de périodes qui se sont écoulées depuis le dernier appel (très souvent 1).

Nous avons crée une tâche supplémentaire qui affichait un message chaque 2 secondes. Au début il y avait un problème car cette troisième tâche ne fonctionnait pas correctement. 

Le problème était que on n'avait pas mis à jour le nombre de timer (3 au lieu de 2). Une fois reglé cet paramètre, la troisième tâche fonctionnait correctment. 

**Questions:**

*Que contient le tableau waitForTimer[] et à quoi sert-il ?*

Ce tableau contient le nombre de période qui s'est écoulé depuis l'allumage de la carte pour chaque tâche.

*Si on a deux tâches indépendantes avec la même période, pourquoi ne peut-on pas utiliser le même timer dans waitFor() ? *

Si deux tâches ont la même période, on ne peut pas utiliser le même timer car ces deux tâches ne n'appellent pas la fonction waitFor au même moment donc les valeurs contenus dans le tableau waitForTimer sont très probablement différentes.

*Dans quel cas la fonction waitFor() peut rendre 2 ? *

La fonction waitFor() peut renvoyer 2 si il s'est écoulé plus de 2 périodes et moins de 3 périodes depuis le dernier appel à la fonction.


## Utilisation de l'écran OLED

Pour commencer à utiliser l'écran OLED, on importe la librairie Adafruit correspondant à notre écran. Puis on va chercher un fichier d'exemple qui nous montrera comment utiliser la librairie.

Chaque programme qui voudra afficher quelque chose à l'écran devra comporter le code ci-dessous dans son prologue.
```C=
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
```

Ensuite lorsqu'on veut afficher du texte :
* d'abord effacer ce qui est afficher sur l'écran avec **clearDisplay**
* définir la couleur du texte avec **setTextColor** (sinon rien n'est afficher, j'imagine que par défault le texte est en noir sur du noir)
* placer le cureur avec **setCursor** (sinon on passe à la ligne suivante pour chaque nouvel affichage -> pas exactement à revoir TODO ) 
* afficher notre texte avec **println**
* et enfin de rafraichir l'écran avec **display** (important sinon on ne voit pas le nouvel affichage)


Exemple pour afficher un compteur :
```C=
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(cpt, DEC);
  display.display();
  cpt++;
```

## Communications inter-tâches

Pour communiquer entre les tâches, on utilise une boîte aux lettres implémenté avec une structure **mailbox** à deux champs : 
* un champs state qui correspond à l'état de la boîte aux lettres (EMPTY ou FULL)
* un champs val qui contient la valeur à communiquer

```C=
enum {EMPTY, FULL};

struct mailbox_s {
  int state;
  int val;
};
```

Pour utiliser cette boîte aux lettres il suffit de soit tester si la boîte est vide dans le cas où l'on souhaite communiquer une donnée, soit tester si elle est pleine dans le cas où on l'on souhaite consommer une donnée.

```C=
struct mailbox_s mb = {.state = EMPTY};

void loop_producteur(... mailbox_t * mb ...) {
  if (mb->state != EMPTY) return; // attend que la mailbox soit vide
  mb->val = 42;
  mb->state = FULL;
}

void loop_consomateur(... mailbox_t * mb ...) {
  if (mb->state != FULL) return; // attend que la mailbox soit pleine
  // usage de mb->val
  mb->state = EMPTY;
}
```