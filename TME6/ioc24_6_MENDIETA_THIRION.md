# TP 6

* Lou THIRION 28614980
* Jorge MENDIETA 21304469

# Modèle Client-Serveur

Le but de cette séance est d'expérimenter le modèle client-serveur. Pour cela nous allons créer une nouvelle application permettant de recueillir le vote de personne concernant le choix de l’heure d’hiver ou d’été.

# Mis en oeuvre du modèle client-serveur

Pour la mis en oeuvre du modèle il nous est fourni deux programmes  : 

**Application "client"**
* Paramètres
    * adresse IP du serveur
    * numéro de port de l'application du serveur
    * le nom de la personne 
    * le vote (été ou hiver)
* Comportement attendu
    * le *client* crée un socket, se connecte et envoie son vote au serveur
    * le *serveur* lui envoie un acquittement de son vote

**L'application "serveur"**
* Paramètres 
    * Le numéro de port d'écoute
* Comportement attendu
    * Le *serveur* crée un socket et écoute
    * Lorsqu'un *client* se connecte, il vérifie que le client n'a pas encore voté
      * si il n'a pas encore voté, il envoie le message "a voté"
      * sinon "erreur, vote déjà réalisé"

**Remarque sur le choix du port d'écoute**

* Le port d'écoute est imposé par le serveur et le client doit connaître ce numéro pour lui envoyer des messages.
* Si c'est une communication local : 
  * Dans un terminal : ```./server 32000```
  * Dans un autre terminal : ```./client localhost 32000```
* Si c'est une communication distante : 
  * Il suffit de remplacer localhost par l'adresse IP du server

**Note:** Nous ne pouvons pas choisir un numéro de port au dessous de 1023 car ils sont considerés comme ports "système" et requièrent donc droit super utilisateur (root).

![schéma_client_server_IOC](https://hackmd.io/_uploads/Bka4Dib1A.png)


# Application de vote pour l'heure d'été ou d'hiver

Les programmes qui nous sont fournis nous permettent d'établir une connexion entre un server et un client mais pas encore de gérer les données du client envoyées au server.

Pour ce faire nous avons décider d'implémenter une table de hachage dont la clé sera le nom du votant pour retrouver rapidement si une personne a déjà voté et si oui qu'est ce qu'elle a voté.

## Table de Hachage

Nous avons choisi d'implémenter une table de hachage contenant des listes chainées. Ces listes permettront de gérer les éventuels collisions que pourrait provoquer le hachage.

![hashTabIOC](https://hackmd.io/_uploads/Bkd_toZk0.png)

### Hachage

En ce qui concerne le hachage, qui calcule l'indice d'un élément dans la table à partir d'une clé propre à cet élément, le calcul est simplement fait à partir de la chaine de caractères correspondant au nom du votant et en additionnant les caractères de la chaine modulo la taille de la table.

```c=
int hash(char *nom, int sizeoftab){
    int res = 1;
    int sizeof_nom = sizeof(nom);
    int i=0;
    for(; i<sizeof_nom; i++) {
        res = res + (int)nom[i];
    }
    return res % sizeoftab;
}
```

C'est une manière assez naïve d'effectuer le hachage car cela va probablement créer beaucoup de collisions si le nombre de votant est important.

### Les structures de données

La structure de notre table est composée d'un tableau de cellule. Une cellule correspond à un votant et contient donc son nom et son vote mais également un pointeur vers la cellule suivante car chaque cellule est également une chaine de la liste contenant tous les éléments dont le calcul du hachage a donné le même résultat.

```c=
struct cell {
    char *nom;
    char *vote;
    struct cell *next;
};

struct hashTab {
    int size;
    struct cell **tab;
};
```

### L'API

Enfin nous avons créer les 3 fonctions suivantes pour intéragir avec notre table de hachage.

```c=
struct hashTab *init(int sizeoftab);
int insere(struct hashTab *ht, char *nom, char *vote);
int existe(struct hashTab *ht, char *nom);
```

* La fonction **init** alloue la mémoire nécessaire pour la table et renvoie un pointeur vers une nouvelle table.
* La fonction **insere**, comme son l'indique, insère un élément dans la table.
* Et la fonction **existe** renvoie 1 si l'élément donné en paramètre est présent dans la table et 0 sinon.


# Améliorations possibles et limites du choix de la table de hachage

## Les améliorations

Pour l'instant notre table de hachage ne permet de maintenir les données que de manière volatile, c'est à dire que si l'on arrête le server les données disparaîssent. 

Pour pallier à ce problème, nous pourrions intégrer à notre table de hachage un mécanisme de chargement et de sauvegarde des données dans un fichier texte au format json par exemple. 

## Les limites 

Nous avons choisi d'utiliser une table de hachage car elle est assez facile à implémenter et nous avions déjà les compétences pour le faire.

Les problèmes de ce choix sont les suivant : 
* Premièrement nous passons beaucoup de temps sur une solution que d'autres ont probablement mieux implémenter que nous (Ce n'est pas forcement un problème car c'est dans le cadre de notre formation de passé du temps sur des choses que d'autres on fait avant nous).
* Notre solution n'est pas généralisable à toutes les applications nécessitant une base donnée.

Pour ces raisons, peut-être qu'il serait préférable que nous utilisions dans notre projet les vrais outils de base de données. Néanmoins nous avons là une solution robuste que nous maîtrisons.