#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashTab.h"

// Initialize the hash table
struct hashTab *init(int sizeoftab){
    struct hashTab *ht = malloc(sizeof(struct hashTab));
    ht->tab = malloc(sizeoftab*sizeof(struct cell));
    ht->size = sizeoftab;
    return ht;
}

// Hash function
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

// Insert a new cell in the hash table
int insere(struct hashTab *ht, char *nom, char *vote){
    int h = hash(nom, ht->size);
    struct cell *new = malloc(sizeof(struct cell));
    if(!new)
        return -1; // Allocation failed

    // Copy the name and vote 
    new->nom = nom;
    new->vote = vote;

    struct cell *c = (ht->tab)[h];
    if(c){
        // Iterate through the linked list
        while(c->next)
            c = c->next;
        // Insert the new cell at the end of the linked list
        c->next = new;
    } else {
        // Insert the new cell at the beginning of the linked list
        ht->tab[h] = new;
    }
    return 0;
}

// Check if a cell exists in the hash table
int existe(struct hashTab *ht, char *nom){
    int h = hash(nom, ht->size);
    struct cell *c = ht->tab[h];
    if(c){
        // Iterate through the linked list
        do {
            // If the name is found, return 1
            if(!strcmp(c->nom, nom))
                return 1;
            // Otherwise, go to the next cell
            c = c->next;
        } while(c);
    }
    return 0; 
}
