#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashTab.h"

struct hashTab *init(int sizeoftab){
    struct hashTab *ht = malloc(sizeof(struct hashTab));
    ht->tab = malloc(sizeoftab*sizeof(struct cell));
    ht->size = sizeoftab;
    return ht;
}

int hash(char *nom, int sizeoftab){
    int res = 1;
    int sizeof_nom = sizeof(nom);
    //for(; nom++; (*nom)!='\0'){
    int i=0;
    for(; i<sizeof_nom; i++) {
        //res = (res ^ ((int) *nom));
        res = res + (int)nom[i];
    }
    return res % sizeoftab;
}

int insere(struct hashTab *ht, char *nom, char *vote){
    int h = hash(nom, ht->size);
    struct cell *new = malloc(sizeof(struct cell));
    if(!new)
        return -1;

    new->nom = nom;
    new->vote = vote;

    struct cell *c = (ht->tab)[h];
    if(c){
        while(c->next)
            c = c->next;
        c->next = new;
    } else {
        ht->tab[h] = new;
    }
    return 0;
}

int existe(struct hashTab *ht, char *nom){
    int h = hash(nom, ht->size);
    struct cell *c = ht->tab[h];
    if(c){
        while(c->next){
            if(strcmp(c->nom, nom))
                return 1;
            c = c->next;
        }
    }
    return 0; 
}
