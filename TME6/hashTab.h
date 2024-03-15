#pragma once

struct cell {
    char *nom;
    char *vote;
    struct cell *next;
};

struct hashTab {
    int size;
    struct cell **tab;
};

struct hashTab *init(int sizeoftab);
int hash(char *nom, int sizeoftab);
int insere(struct hashTab *ht, char *nom, char *vote);
int existe(struct hashTab *ht, char *nom);
