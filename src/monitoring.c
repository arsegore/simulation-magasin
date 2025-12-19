#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"
#include "msg.h"
#include "sem.h"
#include "smp.h"
#include "cle.h"
#include "logs.h"

#define NETTOYER_ECRAN printf("\e[1;1H\e[2J");

int id_smp_taux_occupation_vendeurs;
int *adr_smp_taux_occupation_vendeurs;
int id_mutex_taux_occupation_vendeurs;
int id_smp_grimoire;
int *adr_smp_grimoire;
int id_file_msg;
int id_smp_transactions;
int *adr_smp_transactions;
int id_sem_dispo_caissiers;

int main(int argc, char **argv) {
    int i;
    int nb_vendeurs;
    int nb_clients;
    int nb_caissiers;
    int montant;

    // 1. Récupération des arguments
    nb_vendeurs = atoi(argv[1]);
    nb_caissiers = atoi(argv[2]);
    nb_clients = atoi(argv[3]);

    // Récupération des SMP utilisés par la simulation
    id_smp_taux_occupation_vendeurs = init_smp(TAILLE_TAB_VENDEUR, FILS, ID_SMP_OCCUPATION_VENDEURS);
    adr_smp_taux_occupation_vendeurs = (int *) attacher_smp(id_smp_taux_occupation_vendeurs);
    id_mutex_taux_occupation_vendeurs = init_sem(ID_SEM_MUTEX_OCCUP_V, 1, FILS, 1);
    id_smp_grimoire = init_smp(TAILLE_TAB_VENDEUR, FILS, ID_SMP_GRIMOIRE);
    adr_smp_grimoire = (int *) attacher_smp(id_smp_grimoire);
    id_smp_transactions = init_smp(TAILLE_TAB_CLIENT, FILS, ID_SMP_TRANSACTIONS);
    adr_smp_transactions = (int *) attacher_smp(id_smp_transactions);
    id_sem_dispo_caissiers = init_sem(ID_SEM_CAISSIERS, nb_caissiers, FILS, 0);

    for (;;) { // boucle infinie, à voir si y a plus propre 
        NETTOYER_ECRAN // nettoyage du terminal pour que l'affichage se fasse tjr sur le "meme" ecran

        printf("Nombre de vendeurs : %d\n", nb_vendeurs);
        printf("Nombre de clients : %d\n", nb_clients);
        printf("Nombre de caissiers : %d\n", nb_caissiers);
        
        printf("\n\n\n");

        // Affichage de l'occupation des vendeurs
        printf("Taille de la file de chaque vendeur :\n");
        for (i = 0; i < nb_vendeurs; i++){
            printf("+-----");
        }
        printf("+\n");
        for (i = 0; i < nb_vendeurs; i++){
            printf("|%5d", adr_smp_taux_occupation_vendeurs[i]);
        }
        printf("|\n");
        for (i = 0; i < nb_vendeurs; i++){
            printf("+-----");
        }
        printf("+\n");
        printf("\n\n\n");

        // Affichage de l'occupation des caissiers
        printf("Taille de la file de chaque caissier :\n");
        for (i = 0; i < nb_caissiers; i++){
            printf("+-----");
        }
        printf("+\n");

        for (i = 0; i < nb_caissiers; i++){
            printf("|%5d", taille_file_sem(id_sem_dispo_caissiers, i));
        }
        printf("|\n");

        for (i = 0; i < nb_caissiers; i++){
            printf("+-----");
        }
        printf("+\n");
        printf("\n\n\n");


        // Affichage des montants de transactions
        printf("Montant des transactions de chaque client :\n");
        for (i = 0; i < nb_clients; i++){
            printf("+-----");
        }
        printf("+\n");
        for (i = 0; i < nb_clients; i++){
            montant = adr_smp_transactions[i];
            if (montant == -1){
                printf("|    X");
            } else {
                printf("|%5d", montant);
            }
        }
        printf("|\n");
        for (i = 0; i < nb_clients; i++){
            printf("+-----");
        }
        printf("+\n");

        usleep(10000);
    }

}