#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include "cle.h"

void verif_fichier() {
    FILE* fich_cle;
    fich_cle = fopen(FICHIER_CLE, "r");
    if (fich_cle == NULL) {
        if (errno == ENOENT) {
            /* on le cree */
            fich_cle = fopen(FICHIER_CLE, "w");
            if (fich_cle == NULL) {
                fprintf(stderr, "Erreur à l'ouverture du fichier clé\n");
                exit(-1);
            }
        } else {
            fprintf(stderr, "Erreur à l'ouverture du fichier clé\n");
            exit(-1);
        }
    } else {
        fclose(fich_cle);
    }
}

int generer_cle(int id){
    int cle;

    verif_fichier();

    cle = ftok(FICHIER_CLE, id);
    if (cle == -1) {
        perror("ftok");
        fprintf(stderr, "Impossible de générer la clé\n");
        exit(-1);
    }
    
    return cle;
}