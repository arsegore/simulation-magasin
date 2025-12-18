/**
 * Processus client
 */
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "config.h"
#include "cle.h"
#include "msg.h"
#include "sem.h"
#include "smp.h"

/**
 * Retourne le numéro du vendeur le moins occupé en se basant sur 
 * le SMP passé en paramètre
 */
int chercher_vendeur_le_moins_occupe(int *smp_taux, int nb_vendeurs){
    int i;
    int min = 9999999;
    int id_min = -1;
    for (i = 0; i < nb_vendeurs; i++){
        if (smp_taux[i] < min) {
            min = smp_taux[i];
            id_min = i;
        }
    }
    return i;
}

int chercher_caissier_aleatoire(nb_caissiers){
    return rand() % nb_caissiers;
}

int main(int argc, char *argv[]){
    int numero_client;
    int rayon_interet;
    int id_smp_taux_occupation_vendeurs;
    int *adr_smp_taux_occupation_vendeurs;
    int numero_vendeur;
    int id_mutex_taux_occupation_vendeurs;
    int id_sem_dispo_vendeurs;
    int id_sem_dispo_caissiers;
    int id_file_msg;
    int recommencer;
    int numero_caissier;
    int nb_vendeurs, nb_caissiers;
    int id_sem_vente;
    message msg;

    if (argc > 0) {
        numero_client = atoi(argv[1]);
        nb_vendeurs = atoi(argv[2]);
        nb_caissiers = atoi(argv[3]);
    }

    // 1. Récupération des IPC
    // 1.1  Le segment pour aller chercher le vendeur le moins occupé 
    //      (un simple tableau avec la taille des files d'attente)
    id_smp_taux_occupation_vendeurs = init_smp(TAILLE_TAB_VENDEUR, FILS, ID_SMP_OCCUPATION_VENDEURS);
    adr_smp_taux_occupation_vendeurs = (int *) attacher_smp(id_smp_taux_occupation_vendeurs);
    id_mutex_taux_occupation_vendeurs = init_sem(ID_SEM_MUTEX_OCCUP_V, 0, FILS, 1);
    
    // 1.2  Les sémaphores pour représenter la file d'attente des vendeurs/caissiers
    id_sem_dispo_vendeurs = init_sem(ID_SEM_VENDEURS, 0, FILS, 0);
    id_sem_dispo_caissiers = init_sem(ID_SEM_CAISSIERS, 0, FILS, 0);
    
    // 1.3  La file de message 
    id_file_msg = init_file_msg(ID_FILE_MSG, FILS);

    // 1.4  Sémaphores pour synchroniser la vente
    id_sem_vente = init_sem(ID_SEM_VENTE, nb_vendeurs, FILS, 1);

    // 2. Tirage aléatoire du rayon qui ns intéresse
    srand(time(NULL));
    rayon_interet = rand() % NB_RAYONS; // numéro de rayon entre 0 et NB_RAYONS - 1

    // 3. Cherche le vendeur le moins occupé
    numero_vendeur = chercher_vendeur_le_moins_occupe(id_smp_taux_occupation_vendeurs, nb_vendeurs);
        // Gérer cas d'erreur ?

    // 4. Attente du vendeur (boucle car il faut recommencer le processus en cas de redirection)
    do {
        // Mise à jour du SMP
        P(id_mutex_taux_occupation_vendeurs, 0, 1);
        adr_smp_taux_occupation_vendeurs[numero_vendeur]++;
        V(id_mutex_taux_occupation_vendeurs, 0, 1);

        // Attente de dispo du vendeur
        P(id_sem_dispo_vendeurs, numero_vendeur, 1);

        // 5. Communique au vendeur le rayon qui l'interesse 
        envoyer_msg(id_file_msg, 
                    NUM_TO_MSG_VENDEUR(numero_vendeur), 
                    NUM_TO_MSG_CLIENT(numero_client), 
                    DEMANDER_NUM_RAYON, 
                    rayon_interet);
        
        // 6. Attente de réponse du vendeur
        msg = recevoir_msg(id_file_msg, NUM_TO_MSG_VENDEUR(numero_vendeur));
    
        // Peu importe la réponse, le client va quitter la file
        P(id_mutex_taux_occupation_vendeurs, 0, 1);
        adr_smp_taux_occupation_vendeurs[numero_vendeur]--;
        V(id_mutex_taux_occupation_vendeurs, 0, 1);

        if (msg.info == VALIDER_RAYON) {
            recommencer = 0;
        } else {
            recommencer = 1;
            numero_vendeur = MSG_TO_NUM_VENDEUR(msg.valeur);
        }
    } while (recommencer);

    // La vente commence
    P(id_sem_vente, numero_vendeur, 1);

    if (rand() <= TAUX_PROBA_VENTE){
        // Si la vente aboutie, on se dirige vers un caissier
        numero_caissier = chercher_caissier_aleatoire(nb_caissiers);
        // Attente de dispo du caissier
        P(id_sem_dispo_caissiers, numero_caissier, 1);

        envoyer_msg(id_file_msg,
                    NUM_TO_MSG_CAISSIER(numero_caissier),
                    NUM_TO_MSG_CLIENT(numero_client),
                    ENVOYER_NUMERO,
                    numero_client); // le numéro est déjà stocké dans l'expéditeur mais pas grave

        msg = recevoir_msg(id_file_msg, NUM_TO_MSG_CLIENT(numero_client));
        if (msg.info == ANNONCER_PRIX) {
            usleep(rand() % TEMPS_MAX_ATTENTE);
        }

        exit(EXIT_SUCCESS);
    } else { 
        // Si elle n'aboutie pas
        envoyer_msg(id_file_msg,
                    NUM_TO_MSG_VENDEUR(numero_vendeur),
                    NUM_TO_MSG_CLIENT(numero_client),
                    ANNULER_VENTE,
                    0
                );
        exit(EXIT_SUCCESS);
    }

    

    

}
