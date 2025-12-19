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
#include "monitoring.h"

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
int id_smp_monitoring;
monit_infos *adr_smp_monitoring;

// détachemebt des SMP
void nettoyer(){
    if (adr_smp_grimoire != NULL){
        detacher_smp(adr_smp_grimoire);
    }
    if (adr_smp_monitoring != NULL){
        detacher_smp(adr_smp_monitoring);
    }
    if (adr_smp_taux_occupation_vendeurs != NULL){
        detacher_smp(adr_smp_taux_occupation_vendeurs);
    }
    if (adr_smp_transactions != NULL){
        detacher_smp(adr_smp_transactions);
    }
}

// Affichages de valeurs intéressantes... (plus ou moins ?)
void afficher_param(monit_infos *smp){
    printf("Nombre de clients : %d\n", smp->nb_clients);
    printf("Nombre de vendeurs : %d\n", smp->nb_vendeurs);
    printf("Nombre de caissiers : %d\n", smp->nb_caissiers);
}

void afficher_stats(monit_infos *smp){
    printf("Nombre de clients entrés : %d\n", smp->clients_entres);
    printf("Nombre de clients sortis : %d\n", smp->clients_sortis);
    printf("Montant total des ventes : %d\n", smp->total_ventes);
}

// Affiche le contenu d'un smp (seulement pour les smp étant gérés comme des tableaux
// avec un nombre de cases par ligne défini en config. Ca permet de pas briser l'affichage 
// dès qu'on a pas mal de clients 
void afficher_contenu_smp(int *smp, int nb, const char *titre) {
    int i, j;
    int limite;

    printf("%s :\n", titre);
    for (j = 0; j < nb; j += NB_VAL_LIGNES_MONITORING) { 
        limite = j + NB_VAL_LIGNES_MONITORING;
        if (limite > nb) {
            limite = nb;
        }
        for (i = j; i < limite; i++) printf("+-----");
        printf("+\n");
        for (i = j; i < limite; i++){
            if (smp[i] == -1){
                printf("|    X");
            } else {
                printf("|%5d", smp[i]);
            }
        }
        printf("|\n");
        for (i = j; i < limite; i++) printf("+-----");
        printf("+\n\n");
    }
}
// Idem pour la file de sémaphores (notamment ici les files des caissiers)
void afficher_contenu_sem(int id_sem, int nb, const char *titre) {
    int i, j;
    int limite;

    printf("%s :\n", titre);
    for (j = 0; j < nb; j += NB_VAL_LIGNES_MONITORING) {
        limite = j + NB_VAL_LIGNES_MONITORING;
        if (limite > nb) {
            limite = nb;
        }
        for (i = j; i < limite; i++) printf("+-----");
        printf("+\n");
        for (i = j; i < limite; i++) printf("|%5d", taille_file_sem(id_sem, i));
        printf("|\n");
        for (i = j; i < limite; i++) printf("+-----");
        printf("+\n\n");
    }
}

int main(int argc, char **argv) {
    int nb_vendeurs;
    int nb_clients;
    int nb_caissiers;

    atexit(nettoyer);

    // Récupération des SMP utilisés par la simulation
    id_smp_monitoring = init_smp(sizeof(monit_infos), FILS, ID_SMP_MONITORING);
    adr_smp_monitoring = (monit_infos *) attacher_smp(id_smp_monitoring);

    // On récupère les valeurs dans le SMP 
    nb_vendeurs = adr_smp_monitoring->nb_vendeurs;
    nb_clients = adr_smp_monitoring->nb_clients;
    nb_caissiers = adr_smp_monitoring->nb_caissiers;

    // Le reste des ipc...
    id_smp_taux_occupation_vendeurs = init_smp(TAILLE_TAB_VENDEUR, FILS, ID_SMP_OCCUPATION_VENDEURS);
    adr_smp_taux_occupation_vendeurs = (int *) attacher_smp(id_smp_taux_occupation_vendeurs);
    id_mutex_taux_occupation_vendeurs = init_sem(ID_SEM_MUTEX_OCCUP_V, 1, FILS, 1);
    id_smp_grimoire = init_smp(TAILLE_TAB_VENDEUR, FILS, ID_SMP_GRIMOIRE);
    adr_smp_grimoire = (int *) attacher_smp(id_smp_grimoire);
    id_smp_transactions = init_smp(TAILLE_TAB_CLIENT, FILS, ID_SMP_TRANSACTIONS);
    adr_smp_transactions = (int *) attacher_smp(id_smp_transactions);
    id_sem_dispo_caissiers = init_sem(ID_SEM_CAISSIERS, nb_caissiers, FILS, 0);


    // Boucle infinie tant qu'on interrompt pas le monitoring
    for (;;) { 
        NETTOYER_ECRAN // replace l'affichage au centre de l'écran pour ne pas voir le rafraichissement

        // Affichage des infos...
        afficher_param(adr_smp_monitoring);
        printf("\n\n\n");

        afficher_contenu_smp(adr_smp_taux_occupation_vendeurs, nb_vendeurs, "Taille de la file de chaque vendeur");

        afficher_contenu_sem(id_sem_dispo_caissiers, nb_caissiers, "Taille de la file de chaque caissier");

        afficher_contenu_smp(adr_smp_transactions, nb_clients, "Montant des transactions de chaque client");

        afficher_stats(adr_smp_monitoring);

        usleep(10000); 
    }
    
    exit(EXIT_SUCCESS);

}