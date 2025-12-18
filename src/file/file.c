#include <stdlib.h>
#include <stdio.h>
#include "file.h"

#define TAILLE f[0]
#define CARDINAL f[1]
#define PREMIER f[2]
#define PROCHAIN f[3]

file file_vide(int taille){
    file f = malloc(sizeof(element) * (TAILLE_MAX_FILE + 4));
    if (f == NULL) {
        fprintf(stderr, "File: Erreur d'allocation\n");
        exit(EXIT_FAILURE);
    }
    TAILLE = taille;
    CARDINAL = 0;
    PREMIER = 4;
    PROCHAIN = 4;
    return f;
}

int est_vide(file f){
    return CARDINAL == 0;
}

int ajouter(file f, element e){
    if (CARDINAL < TAILLE){
        f[PROCHAIN] = e;
        CARDINAL++;
        PROCHAIN++;        
        if (PROCHAIN >= TAILLE + 4) {
            PROCHAIN = 4;
        }
        return 1;
    }
    return 0;
}

void retirer(file f){
    if (CARDINAL > 0){
        CARDINAL--;
        PREMIER++;
        if (PREMIER >= TAILLE + 4) {
            PREMIER = 4;
        }
    }
}

element premier(file f){
    return f[PREMIER];
}

void afficher(file f){
    for (int i = 0; i < TAILLE; i++){
        printf("%d ", f[i]);
    }
    printf("\n");
}





