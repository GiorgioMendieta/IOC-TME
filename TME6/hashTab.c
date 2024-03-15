#include <stdlib.h>
#include <string.h>
#include "hashTab.h"

struct hashTab *init(int sizeoftab){
    struct hashTab *ht = malloc(sizeoftab*sizeof(struct cell));
    ht->size = sizeoftab;
    return ht;
}

int hash(char *nom, int sizeoftab){
    int res = 0;
    for(; nom++; (*nom)!='\0'){
        res = (res ^ ((int) *nom));
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

    struct cell *c = ht->tab[h];
    if(c){
        while(c->next)
            c = c->next;
        c->next = new;
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
