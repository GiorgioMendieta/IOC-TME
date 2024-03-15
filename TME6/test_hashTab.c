#include <stdio.h>

#include "hashTab.h"

int main(){
    struct hashTab *ht = init(100);
    insere(ht, "jean", "hiver");
    insere(ht, "michel", "ete");
    printf("test existe %d\n", existe(ht, "jean"));
    return 0;
}