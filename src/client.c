/**
 * Processus client
 */
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "config.h"
#include "cle.h"
#include "msg.h"
#include "sem.h"
#include "smp.h"

int numero_client;
int id_smp_taux_occupation_vendeurs;
int *adr_smp_taux_occupation_vendeurs;
int id_mutex_taux_occupation_vendeurs;
int id_sem_dispo_vendeurs;
int id_sem_dispo_caissiers;
int id_file_msg;
int recommencer;
int id_sem_vente;
int id_sem_paiement;
message msg;
sigset_t tout_bloquer, att_sigusr1, att_sigusr2;

void nettoyer(){
    detacher_smp(adr_smp_taux_occupation_vendeurs);
}

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
    int rayon_interet;
    int numero_vendeur;
    int numero_caissier;
    int nb_vendeurs, nb_caissiers;

    if (argc > 0) {
        numero_client = atoi(argv[1]);
        nb_vendeurs = atoi(argv[2]);
        nb_caissiers = atoi(argv[3]);
    }

    atexit(nettoyer);   // vu qu'on exit dans tous les cas (erreur ou normal), ça permet d'assurer le nettoyage dans ttes les situations

    // 0. Gestion des signaux
    sigfillset(&tout_bloquer);
    sigprocmask(SIG_SETMASK, &tout_bloquer, NULL); // On bloque TOUS les signaux
    sigfillset(&att_sigusr1);
    sigdelset(&att_sigusr1, SIGUSR1);

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

    // 1.4  Sémaphores pour synchroniser la vente et le paiement
    id_sem_vente = init_sem(ID_SEM_VENTE, nb_vendeurs, FILS, 1);
    id_sem_paiement = init_sem(ID_SEM_PAIEMENT, nb_caissiers, FILS, 1);

    log("[Client %d] Création...\n", numero_client);

    /**********************
     * ATTENTE DU DEMARRAGE
     **********************/
    sigsuspend(&att_sigusr1);

    // 2. Tirage aléatoire du rayon qui ns intéresse
    srand(time(NULL));
    rayon_interet = rand() % NB_RAYONS; // numéro de rayon entre 0 et NB_RAYONS - 1

    log("[Client %d] Démarrage. Je suis intéréssé par le rayon %d\n", numero_client, rayon_interet);
    
    // 3. Cherche le vendeur le moins occupé
    numero_vendeur = chercher_vendeur_le_moins_occupe(id_smp_taux_occupation_vendeurs, nb_vendeurs);
        // Gérer cas d'erreur ?
    log("[Client %d] Je me dirige vers le vendeur %d...\n", numero_client, numero_vendeur);

    // 4. Attente du vendeur (boucle car il faut recommencer le processus en cas de redirection)
    do {
        // Mise à jour du SMP
        P(id_mutex_taux_occupation_vendeurs, 0, 1);
        adr_smp_taux_occupation_vendeurs[numero_vendeur]++;
        V(id_mutex_taux_occupation_vendeurs, 0, 1);
        log("[Client %d] Je suis dans la file du vendeur %d, j'attends qu'il soit dispo...\n", numero_client, numero_vendeur);

        // Attente de dispo du vendeur
        P(id_sem_dispo_vendeurs, numero_vendeur, 1);

        // 5. Communique au vendeur le rayon qui l'interesse 
        envoyer_msg(id_file_msg, 
                    NUM_TO_MSG_VENDEUR(numero_vendeur), 
                    NUM_TO_MSG_CLIENT(numero_client), 
                    DEMANDER_NUM_RAYON, 
                    rayon_interet);
        log("[Client %d] Je dis au vendeur %d que je suis intéressé par le rayon %d\n", numero_client, numero_vendeur, rayon_interet);
        
        // 6. Attente de réponse du vendeur
        msg = recevoir_msg(id_file_msg, NUM_TO_MSG_VENDEUR(numero_vendeur));
    
        // Peu importe la réponse, le client va quitter la file
        P(id_mutex_taux_occupation_vendeurs, 0, 1);
        adr_smp_taux_occupation_vendeurs[numero_vendeur]--;
        V(id_mutex_taux_occupation_vendeurs, 0, 1);

        if (msg.info == VALIDER_RAYON) {
            recommencer = 0;
            log("[Client %d] Le vendeur %d est intéressé par le même rayon (%d) ! La vente commence\n", numero_client, numero_vendeur, rayon_interet);
        } else {
            recommencer = 1;
            log("[Client %d] Le vendeur %d me redirige vers ", numero_client, numero_vendeur);
            numero_vendeur = MSG_TO_NUM_VENDEUR(msg.valeur);
            log("le vendeur %d\n", numero_vendeur);
        }
    } while (recommencer);

    // La vente commence
    P(id_sem_vente, numero_vendeur, 1);

    if (rand() <= TAUX_PROBA_VENTE){
        // Si la vente aboutie, on se dirige vers un caissier
        log("[Client %d] La vente a abouti !\n", numero_client);

        numero_caissier = chercher_caissier_aleatoire(nb_caissiers);

        log("[Client %d] Je me dirige vers le caissier %d, j'att qu'il soit dispo\n", numero_client, numero_caissier);
        // Attente de dispo du caissier
        P(id_sem_dispo_caissiers, numero_caissier, 1);

        log("[Client %d] Je donne au caissier %d mon numéro de client\n", numero_client, numero_caissier);
        envoyer_msg(id_file_msg,
                    NUM_TO_MSG_CAISSIER(numero_caissier),
                    NUM_TO_MSG_CLIENT(numero_client),
                    ENVOYER_NUMERO,
                    numero_client); // le numéro est déjà stocké dans l'expéditeur mais pas grave

        msg = recevoir_msg(id_file_msg, NUM_TO_MSG_CLIENT(numero_client));
        
        log("[Client %d] Le paiement commence...\n", numero_client);
        P(id_sem_paiement, numero_caissier, 1);

    } else { 
        // Si elle n'aboutie pas
        log("[Client %d] La vente n'a pas abouti ...\n", numero_client);

        envoyer_msg(id_file_msg,
                    NUM_TO_MSG_VENDEUR(numero_vendeur),
                    NUM_TO_MSG_CLIENT(numero_client),
                    ANNULER_VENTE,
                    0
                );
    }

    log("[Client %d] Je termine...\n", numero_client);
    exit(EXIT_SUCCESS);
}
