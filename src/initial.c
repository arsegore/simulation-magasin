/**
 * Processus initial, démarre la simulation
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "msg.h"
#include "sem.h"
#include "smp.h"
#include "cle.h"

void usage(){
    printf("Usage: ./simulation <nb_clients> <nb_vendeurs> <nb_caissiers>\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
    int nb_vendeurs, nb_caissiers, nb_clients;
    int id_smp_taux_occupation_vendeurs;
    int *adr_smp_taux_occupation_vendeurs;
    int id_mutex_taux_occupation_vendeurs;
    int id_smp_grimoire;
    int *adr_smp_grimoire;
    int id_smp_transactions;
    int *adr_smp_transactions;
    int id_msg_vendeur_client;
    int id_sem_dispo_vendeurs;
    int id_sem_dispo_caissiers;
    int id_sem_vente;
    int id_sem_paiement;
    int id_file_msg;
    
    int i;

    if (argc < 4) {
        usage();
    }

    //1. Récupération des arguments
    nb_vendeurs = atoi(argv[1]);
    nb_caissiers = atoi(argv[2]);
    nb_clients = atoi(argv[3]);

    // 1.1 Vérif
    if (nb_vendeurs < NB_RAYONS) {
        printf("Il faut au moins %d vendeurs. (1 par rayon)\n", NB_RAYONS);
        exit(EXIT_FAILURE);
    }

    // 2. Création des IPC nécessaires
    // 2.1.1 SMP représentant l'occupation de chaque vendeur (lgr de sa file)
    //     Les clients lisent dedans, les vendeurs y écrivent lorsque leur file
    //     est modifiée
    id_smp_taux_occupation_vendeurs = init_smp(TAILLE_TAB_VENDEUR, PERE, ID_SMP_OCCUPATION_VENDEURS);
    adr_smp_taux_occupation_vendeurs = (int *) attacher_smp(id_smp_taux_occupation_vendeurs);
    for (i = 0; i < NB_VENDEURS_MAX; i++){
        adr_smp_taux_occupation_vendeurs[i] = 0;
    }
    // 2.1.2 Besoin d'un mutex pour éviter les conflits sur ce smp..
    id_mutex_taux_occupation_vendeurs = init_sem(ID_SEM_MUTEX_OCCUP_V, 1, PERE, 1);

    // 2.2 SMP Grimoire (info sur la spécialité de chaque vendeur)
    //      Rempli par les vendeurs quand ils s'initialisent, pas besoin de mutex
    id_smp_grimoire = init_smp(TAILLE_TAB_VENDEUR, PERE, ID_SMP_GRIMOIRE);
    adr_smp_grimoire = (int *) attacher_smp(id_smp_grimoire);
    for (i = 0; i < NB_VENDEURS_MAX; i++){
        adr_smp_grimoire[i] = -1;
    }

    // 2.3 SMP Transactions (stocke les montants des transactions des clients)
    id_smp_transactions = init_smp(TAILLE_TAB_CLIENT, PERE, ID_SMP_TRANSACTIONS);
    adr_smp_transactions = (int *) attacher_smp(id_smp_transactions);
    for (i = 0; i < NB_CLIENTS_MAX; i++) {
        adr_smp_transactions[i] = -1;   // initialisation des transactions à -1
    }

    // 2.4 File de messages 
    //     Utilisées pour toutes les communications (V <==> CL, V <==> CA)
    id_file_msg = init_file_msg(ID_FILE_MSG, PERE);

    // 2.5 Ensemble de sémaphores pour les vendeurs & caissiers
    //     Représente la disponibilité de chaque vendeur & caissier
    id_sem_dispo_vendeurs = init_sem(ID_SEM_VENDEURS, nb_vendeurs, PERE, 0);
    id_sem_dispo_caissiers = init_sem(ID_SEM_CAISSIERS, nb_caissiers, PERE, 0);

    // 2.6 Ensemble de sémaphores pour synchroniser les ventes 
    id_sem_vente = init_sem(ID_SEM_VENTE, nb_vendeurs, PERE, 1);

    // 2.7 Ensemble de sémaphores pour les paiments 
    id_sem_paiement = init_sem(ID_SEM_PAIEMENT, nb_caissiers, PERE, 1);



}