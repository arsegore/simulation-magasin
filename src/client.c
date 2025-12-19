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
#include "logs.h"
#include "monitoring.h"

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
int id_smp_monitoring;
monit_infos *adr_smp_monitoring;
int id_mutex_monitoring;

// détache les SMP...
void nettoyer(){
    printlog("[Client %d] Nettoyage...\n", numero_client);
    if (adr_smp_taux_occupation_vendeurs != NULL) {
        detacher_smp(adr_smp_taux_occupation_vendeurs);
    }
    if (adr_smp_monitoring != NULL){
        detacher_smp(adr_smp_monitoring);
    }
}

void handler_sigusr1(){
}

/**
 * Retourne le numéro du vendeur le moins occupé en se basant sur 
 * le SMP passé en paramètre
 */
int chercher_vendeur_le_moins_occupe(int *adr_smp_taux, int nb_vendeurs){
    int i;
    int min = adr_smp_taux[0];
    int id_min = 0;
    for (i = 0; i < nb_vendeurs; i++){
        if (adr_smp_taux[i] < min) {
            min = adr_smp_taux[i];
            id_min = i;
        }
    }
    return id_min;
}

int chercher_caissier_aleatoire(int nb_caissiers){
    return rand() % nb_caissiers;
}

int main(int argc, char *argv[]){
    int rayon_interet;
    int numero_vendeur;
    int numero_caissier;
    int nb_vendeurs, nb_caissiers;
    int ancien_numero_vendeur;

    numero_client = atoi(argv[1]);
    nb_vendeurs = atoi(argv[2]);
    nb_caissiers = atoi(argv[3]);

    atexit(nettoyer);   // vu qu'on exit dans tous les cas (erreur ou normal), ça permet d'assurer le nettoyage dans ttes les situations
    init_log();

    // 0. Gestion des signaux
    sigfillset(&tout_bloquer);
    sigprocmask(SIG_SETMASK, &tout_bloquer, NULL); // On bloque TOUS les signaux
    sigfillset(&att_sigusr1);
    sigdelset(&att_sigusr1, SIGUSR1);
    signal(SIGUSR1, handler_sigusr1);

    // 1. Récupération des IPC
    // 1.1  Le segment pour aller chercher le vendeur le moins occupé 
    //      (un simple tableau avec la taille des files d'attente)
    id_smp_taux_occupation_vendeurs = init_smp(TAILLE_TAB_VENDEUR, FILS, ID_SMP_OCCUPATION_VENDEURS);
    adr_smp_taux_occupation_vendeurs = (int *) attacher_smp(id_smp_taux_occupation_vendeurs);
    id_mutex_taux_occupation_vendeurs = init_sem(ID_SEM_MUTEX_OCCUP_V, 1, FILS, 1);
    
    // 1.2  Les sémaphores pour représenter la file d'attente des vendeurs/caissiers
    id_sem_dispo_vendeurs = init_sem(ID_SEM_VENDEURS, nb_vendeurs, FILS, 0);
    id_sem_dispo_caissiers = init_sem(ID_SEM_CAISSIERS, nb_caissiers, FILS, 0);
    
    // 1.3  La file de message 
    id_file_msg = init_file_msg(ID_FILE_MSG, FILS);

    // 1.4  Sémaphores pour synchroniser la vente et le paiement
    id_sem_vente = init_sem(ID_SEM_VENTE, nb_vendeurs, FILS, 0);
    id_sem_paiement = init_sem(ID_SEM_PAIEMENT, nb_caissiers, FILS, 0);

    // 1.5 Les infos du monitoring 
    id_smp_monitoring = init_smp(sizeof(monit_infos), FILS, ID_SMP_MONITORING);
    adr_smp_monitoring = (monit_infos *) attacher_smp(id_smp_monitoring);
    id_mutex_monitoring = init_sem(ID_MUTEX_MONITORING, 1, FILS, 1);

    printlog("[Client %d] Création...\n", numero_client);

    /**********************
     * ATTENTE DU DEMARRAGE
     **********************/
    sigsuspend(&att_sigusr1);

    // 2. Tirage aléatoire du rayon qui ns intéresse
    srand(time(NULL) * getpid());
    rayon_interet = rand() % NB_RAYONS; // numéro de rayon entre 0 et NB_RAYONS - 1
    P(id_mutex_monitoring, 0, 1);
    adr_smp_monitoring->clients_entres++;
    V(id_mutex_monitoring, 0, 1);

    printlog("[Client %d] Démarrage. Je suis intéréssé par le rayon %s\n", numero_client, nom_rayon(rayon_interet));
    
    // 3. Cherche le vendeur le moins occupé
    numero_vendeur = chercher_vendeur_le_moins_occupe(adr_smp_taux_occupation_vendeurs, nb_vendeurs);
        // Gérer cas d'erreur ?
    printlog("[Client %d] Je me dirige vers le vendeur %d...\n", numero_client, numero_vendeur);

    // 4. Attente du vendeur (boucle car il faut recommencer le processus en cas de redirection)
    do {
        // Mise à jour du SMP
        P(id_mutex_taux_occupation_vendeurs, 0, 1);
        adr_smp_taux_occupation_vendeurs[numero_vendeur]++;
        V(id_mutex_taux_occupation_vendeurs, 0, 1);
        printlog("[Client %d] Je suis dans la file du vendeur %d, j'attends qu'il soit dispo...\n", numero_client, numero_vendeur);

        // Attente de dispo du vendeur
        P(id_sem_dispo_vendeurs, numero_vendeur, 1);

        // 5. Communique au vendeur le rayon qui l'interesse 
        envoyer_msg(id_file_msg, numero_vendeur, MSG_VENDEUR, ACCUEIL, numero_client, rayon_interet);
        printlog("[Client %d] Je dis au vendeur %d que je suis intéressé par le rayon %s\n", numero_client, numero_vendeur, nom_rayon(rayon_interet));
        
        // 6. Attente de réponse du vendeur
        msg = recevoir_msg(id_file_msg, numero_client, MSG_CLIENT, REPONSE);
    
        // Peu importe la réponse, le client va quitter la file
        P(id_mutex_taux_occupation_vendeurs, 0, 1);
        adr_smp_taux_occupation_vendeurs[numero_vendeur]--;
        V(id_mutex_taux_occupation_vendeurs, 0, 1);

        if (msg.valeur == VALIDER_RAYON) {
            recommencer = 0;
            printlog("[Client %d] Le vendeur %d est intéressé par le même rayon (%s) ! La vente commence\n", numero_client, numero_vendeur, nom_rayon(rayon_interet));
        } else {
            recommencer = 1;
            ancien_numero_vendeur = numero_vendeur;
            numero_vendeur = msg.valeur;
            printlog("[Client %d] Le vendeur %d me redirige vers le vendeur %d\n", numero_client, ancien_numero_vendeur, numero_vendeur);
            if (numero_vendeur < 0 || numero_vendeur >= NB_VENDEURS_MAX) {
                printlog("[Client %d] Erreur : redirection vers un vendeur inexistant !\n", numero_client);
                exit(EXIT_FAILURE);
            }
        }
    } while (recommencer);
    // La vente commence
    P(id_sem_vente, numero_vendeur, 1);

    if ((rand() % 100) <= TAUX_PROBA_VENTE){
        // Si la vente aboutie, on se dirige vers un caissier
        printlog("[Client %d] La vente a abouti !\n", numero_client);

        envoyer_msg(id_file_msg, numero_vendeur, MSG_VENDEUR, CHOIX_CLIENT, numero_client, CLIENT_CONFIRME_VENTE);

        // Attente de la validation du vendeur avant d'aller vers un caissier
        msg = recevoir_msg(id_file_msg, numero_client, MSG_CLIENT, FIN_VENTE);
        numero_caissier = chercher_caissier_aleatoire(nb_caissiers);

        printlog("[Client %d] Je me dirige vers le caissier %d, j'att qu'il soit dispo\n", numero_client, numero_caissier);
        // Attente de dispo du caissier
        P(id_sem_dispo_caissiers, numero_caissier, 1);

        printlog("[Client %d] Je donne au caissier %d mon numéro de client\n", numero_client, numero_caissier);
        envoyer_msg(id_file_msg, numero_caissier, MSG_CAISSIER, ENTREE_CAISSE, numero_client, numero_client); // le numéro est déjà stocké dans l'expéditeur mais pas grave

        msg = recevoir_msg(id_file_msg, numero_client, MSG_CLIENT, PAIEMENT);
        
        printlog("[Client %d] Le paiement commence...\n", numero_client);
        P(id_sem_paiement, numero_caissier, 1);

    } else { 
        // Si elle n'aboutie pas
        printlog("[Client %d] La vente n'a pas abouti ...\n", numero_client);

        envoyer_msg(id_file_msg, numero_vendeur, MSG_VENDEUR, CHOIX_CLIENT, numero_client, CLIENT_ANNULE_VENTE);
    }

    P(id_mutex_monitoring, 0, 1);
    adr_smp_monitoring->clients_entres--;
    adr_smp_monitoring->clients_sortis++;
    V(id_mutex_monitoring, 0, 1);

    printlog("[Client %d] Je termine...\n", numero_client);
    exit(EXIT_SUCCESS);
}
