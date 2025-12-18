/**
 * Processus caissier
 */
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "config.h"
#include "cle.h"
#include "msg.h"
#include "sem.h"
#include "smp.h"



int determiner_prix(){
    return rand() % (PRIX_MAX - PRIX_MIN) + PRIX_MIN;
}

int main(int argc, char *argv[]){
    int numero_client;
    int rayon_interet;
    int id_smp_taux_occupation_vendeurs;
    int *adr_smp_taux_occupation_vendeurs;
    int numero_vendeur;
    int id_mutex_taux_occupation_vendeurs;
    int id_smp_grimoire;
    int *adr_smp_grimoire;
    int id_sem_dispo_vendeurs;
    int id_sem_dispo_caissiers;
    int id_file_msg;
    int recommencer;
    int numero_caissier;
    int nb_vendeurs, nb_caissiers;
    int num_rayon_client;
    int num_rayon_vendeur;
    int id_sem_paiement;    
    int id_smp_transactions;
    int *adr_smp_transactions;
    int montant;
    message msg;

    if (argc > 0) {
        numero_caissier = atoi(argv[1]);
        nb_vendeurs = atoi(argv[2]);
        nb_caissiers = atoi(argv[3]);
    
    }
    srand(time(NULL));

    // 1. Récupération des IPC    
    id_file_msg = init_file_msg(ID_FILE_MSG, FILS);
    id_sem_paiement = init_sem(ID_SEM_PAIEMENT, nb_vendeurs, FILS, 1);
    id_sem_dispo_caissiers = init_sem(ID_SEM_CAISSIERS, nb_caissiers, FILS, 0);
    id_smp_transactions = init_smp(TAILLE_TAB_CLIENT, FILS, ID_SMP_TRANSACTIONS);
    adr_smp_transactions = (int *) attacher_smp(id_smp_transactions);

    while (recommencer) {
        // 2. Le caissier est prêt à accueillir un client
        V(id_sem_dispo_caissiers, numero_caissier, 1);

        msg = recevoir_msg(id_file_msg, NUM_TO_MSG_CAISSIER(numero_caissier));
        numero_client = msg.valeur; // pas besoin de la macro car on a doublé l'info
        // vérifier qu'il s'agit du bon type de msg ?

        // 3. Récupére le montant de la transaction
        montant = adr_smp_transactions[numero_client];
        adr_smp_transactions[numero_client] = -1;

        // 4. On communqiue le prix au client
        envoyer_msg(id_file_msg,
                    NUM_TO_MSG_CLIENT(numero_client),
                    NUM_TO_MSG_CAISSIER(numero_caissier),
                    ANNONCER_PRIX,
                    montant
                    );

        // 5. Paiement
        usleep(rand() % TEMPS_MAX_ATTENTE); // Attente aléatoire
            
        // Fin du paiement
        V(id_sem_paiement, numero_caissier, 1);

        // Et au suivant !
    }

}