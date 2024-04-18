# Serveur WEB minimaliste

* Lou THIRION 28614980
* Jorge MENDIETA 21304469

## Accès au serveur HTTP de la Raspberry par un tunnel ssh

## Communication par FIFO 

Nous avons besoin d'utiliser une FIFO pour la communication inter-processus

**writer.py**

*Dans quel répertoire est créée la fifo ?*
* La fifo es créée dans le répertoire `/tmp/myfifo`.

*Quelle différence entre mkfifo et open ?*
* La fonction `mkfifo` créé la fifo alors que la fonction open ouvre le flux de données vers la fifo et renvoie le file descripteur.

*Pourquoi tester que la fifo existe ?*
* On teste la présence de la fifo car il peut y avoir plusieurs producteurs qui écrivent dedan.

*À quoi sert flush ?*
* Cette commande force la FIFO à envoyer les données au lieu d'attendre que le buffer soit rempli.

*Pourquoi ne ferme-t-on pas la fifo ?*
* Car un autre processus peut utiliser la FIFO (ex. lecteur).

**reader.py**

*Que fait readline ?*
* Le readline récupère des données de la fifo.

*Vous allez remarquer que lorsque vous lancer un écrivain (en C ou en Python) rien ne se passe tant que vous n'avez pas lancé un lecteur.*

*Expliquez le phénomène.*

* Puisque on travaille avec des tubes, il faut ouvrir les deux côtés afin de l'utiliser correctement. D'un côté, un **écrivain** insére des données dans le tube, qui est vu come un *fichier* du point de vue de la machine. 

* Ces données resteront dans le tube jusqu'à qu'un **lecteur** consomme ces données afin de les utiliser avec la fonction `open()` en mode lecture.

### Commentaires

Nous avons trouvé un problème dans le fichier écrivain en C. À la fin de la boucle, on avait ce morceau de code.

```c
write(fd, "end\n", sizeof("end"));
```

Après la fin de la boucle, on recevoit le message suivant : 

```bash
hello 5 fois from c
end
o 5 fois from c
```

Nous pouvons remarquer qu'il y avait un bug avec l'affichage. En fait, `"end\n"` contient 4 charactères mais on prenait la taille que de trois charactères avec `sizeof("end")`. Puisque on prenait pas le character `\n` le buffer n'était pas flushé. 

Nous avons résolu l'erreur d'affichage en utilisant la taille correcte afin de faire un flush avec le char `\n`.

```c
write(fd, "end\n", sizeof("end\n"));
```

Après la fin de la boucle, on recevoit le message suivant : 

```bash
hello 5 fois from c
end
```

## Création d'un serveur *fake*

![image](https://hackmd.io/_uploads/BknnUG16a.png)

* *Fake* lit une valeur sur stdin et place la valeur lue dans une variable
* Lorsque l'on tape plusieurs valeurs de suite la nouvelle valeur écrase l'ancienne.
* *Fake* est toujours en fonctionnement.
* *Fake* attends aussi un message de la fifo s2f.
* Lorsqu'il reçoit un message, il l'affiche et il renvoie dans la fifo f2s la dernière valeur lue sur stdin.

Nous avons modifié le serveur fake.c afin de lire l'entrée standard et envoyer les données au serveur python.

Nous avons utilisé la fonction `FD_SET` pour inclure le descripteur de fichiers de l'entrée standard dans l'ensemble de descripteur `rfds`.

```c
FD_SET(STDIN_FILENO, &rfds); // wait for STDIN
```

Puis, dans la boucle nous faisons toujours la selection avec le descripteur de fichiers le plus grand + 1 pour prendre en compte le STDIN.

Finalement, nous envoyons les données reçus par STDIN vers le tube `f2s`

```c
if (FD_ISSET(STDIN_FILENO, &rfds))
    { // something to read
        int nbchar;
        if ((nbchar = read(STDIN_FILENO, serverRequest, MAXServerResquest)) == 0)
            break;
        serverRequest[nbchar] = 0;
        fprintf(stderr, "SENT: %s", serverRequest);
        write(f2s, serverRequest, nbchar);
    }
```

## Création d'un serveur web

### Tunnel SSH

Pour lancer le server web, nous devons commencer par nous connecter sur la Raspberry de manière un peu différente par rapport à d'habitude. Il faut exécuter la commande suivante : 

```shell
ssh -p 622xx -L 8001:localhost:8000 pi@peri
```

où :
* **xx** correspond au numéro de notre carte (Numéro 25).
* **8001** est le numéro de port du **client**, on pourrait le modifier.
* **8000** est le numéro de port sur lequel est branché le **Serveur HTTP**. Nous pouvons pas le modifier car cela a été configuré dans le routeur.

Cette commande créer un tunnel pour faire passer les requêtes HTTP par le protocole SSH.

### Lancement du server et du programme fake

Ensuite il faut lancer le server et le programme fake sur la carte qui aura été préalablement compilé par le cross-compilateur.

### Connexion au server 

Enfin pour se connecter au server, il faut entrer dans un navigateur l'adresse suivante :

```
localhost:8001
```

où : 

* localhost correspond à l'adresse IP local (pas internet).
* 8001 est le port qu'on a choisi plus haut lors de la création du tunnel.

## Accès aux leds et au bouton poussoir par le serveur

### Clignotement d'une led

Pour accéder aux leds de la carte, nous avons simplement choisi de rajouter un petit bout de code dans le main du programme **fake**.

```c
if(!strcmp(serverRequest, "w oui\n") && fork()==0){
    printf("blink\n");
    char *args[]={"../../lab1/blink0.x",NULL};
    execvp(args[0],args);
    return -1;
}
```
Ce code est ajouté après avoir récupérer le message envoyé par le client au server sous forme d'un buffer de caractère.
Si ce message est égale à "oui" alors on fait appel à la fonction **fork** qui va créer un nouveau processus puis à la fonction **exec** qui va changer le code qu'exécute le nouveau processus (pour l'instant le code du programme fake) par un code permettant de faire clignoter une led.

**Mesaventures lors de la suppression de tous les processus** : 

Cela marche pas toujours très bien, parfois la LED ne clignote pas. C'est probablement pas la meilleur manière de le faire mais c'était pour ne pas avoir à reécrire du code qui existe déjà.

Il est possible que le processus crée pour faire clignoter la LED ne soit pas tué avec la mort de son père. Dans ce cas il faut le retrouver avec la commande `top` et le tuer manuellement.