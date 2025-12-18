/**
 * Processus vendeur
 */
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "config.h"
#include "cle.h"
#include "msg.h"
#include "sem.h"
#include "smp.h"
#include "logs.h"

int numero_vendeur;
int id_smp_taux_occupation_vendeurs;
int *adr_smp_taux_occupation_vendeurs;
int id_mutex_taux_occupation_vendeurs;
int id_smp_grimoire;
int *adr_smp_grimoire;
int id_sem_dispo_vendeurs;
int id_sem_dispo_caissiers;
int id_file_msg;
int recommencer = 1;
int id_sem_vente;    
int id_smp_transactions;
int *adr_smp_transactions;
message msg;
sigset_t tout_bloquer, att_sigusr1, att_sigusr2;

void handler_sigusr2(){
    recommencer = 0;
}

void handler_sigusr1(){
}

void nettoyer(){
    log("[Vendeur %d] Nettoyage...\n", numero_vendeur);
    detacher_smp(adr_smp_taux_occupation_vendeurs);
    detacher_smp(adr_smp_grimoire);
    detacher_smp(adr_smp_transactions);
}

// parcours du grimoire à partir d'un point aléatoire
// pour ne pas tout le temps envoyer les clients vers le mm vendeur
int chercher_vendeur_specialise(int num_rayon, int *grimoire){
    int depart;
    int i;

    depart = rand() % NB_VENDEURS_MAX;
    for (i = depart; i < NB_VENDEURS_MAX; i++){
        if (grimoire[depart + i % NB_VENDEURS_MAX] == num_rayon) {
            return grimoire[depart + i % NB_VENDEURS_MAX];
        }
    }
}

int determiner_prix(){
    return rand() % (PRIX_MAX - PRIX_MIN) + PRIX_MIN;
}

int main(int argc, char *argv[]){
    int numero_client;
    int rayon_interet;
    int numero_caissier;
    int nb_vendeurs, nb_caissiers;
    int num_rayon_client;
    int num_rayon_vendeur;
    int num_redirection;
    int attente;
    int prix;

    if (argc > 0) {
        numero_vendeur = atoi(argv[1]);
        nb_vendeurs = atoi(argv[2]);
        nb_caissiers = atoi(argv[3]);
    
    }
    srand(time(NULL));

    atexit(nettoyer);   // vu qu'on exit dans tous les cas (erreur ou normal), ça permet d'assurer le nettoyage dans ttes les situations

    // 0. Gestion des signaux
    sigfillset(&tout_bloquer);
    sigprocmask(SIG_SETMASK, &tout_bloquer, NULL); // On bloque TOUS les signaux
    sigfillset(&att_sigusr1);
    sigdelset(&att_sigusr1, SIGUSR1);
    sigfillset(&att_sigusr2);
    sigdelset(&att_sigusr2, SIGUSR2);
    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);

    // 1. Récupération des IPC    
    id_file_msg = init_file_msg(ID_FILE_MSG, FILS);
    id_sem_vente = init_sem(ID_SEM_VENTE, nb_vendeurs, FILS, 1);
    id_sem_dispo_vendeurs = init_sem(ID_SEM_VENDEURS, nb_vendeurs, FILS, 0);
    id_smp_transactions = init_smp(TAILLE_TAB_CLIENT, FILS, ID_SMP_TRANSACTIONS);
    adr_smp_transactions = (int *) attacher_smp(id_smp_transactions);
    
    // 1. Détermine son rayon de spécialité
    if (numero_vendeur < NB_RAYONS) {
        num_rayon_vendeur = numero_vendeur;
    } else {
        num_rayon_vendeur = rand() % NB_RAYONS;
    }
    adr_smp_grimoire[numero_vendeur] = num_rayon_vendeur;

    log("[Vendeur %d] Création...\n", numero_vendeur);

    /**********************
     * ATTENTE DU DEMARRAGE
     **********************/
    sigsuspend(&att_sigusr1);

    log("[Vendeur %d] Démarrage. Je suis spécialisé dans le rayon %d\n", numero_vendeur, num_rayon_vendeur);

    while (recommencer) {
        // 2. Le vendeur est prêt à accueillir un client
        V(id_sem_dispo_vendeurs, numero_vendeur, 1);

        msg = recevoir_msg(id_file_msg, NUM_TO_MSG_VENDEUR(numero_vendeur));
        
        // vérifier qu'il s'agit du bon type de msg ?
        // 3. Le client demande le rayon, on lui répond
        numero_client = MSG_TO_NUM_CLIENT(msg.qui_envoie);
        num_rayon_client = msg.valeur;

        log("[Vendeur %d] Le client %d est là, il me dit qu'il est intéressé par le rayon %d\n", numero_vendeur, numero_client, num_rayon_client);

        // Si c'est pas bon, redirige le client
        if (num_rayon_client != num_rayon_vendeur) {
            num_redirection = chercher_vendeur_specialise(num_rayon_client, adr_smp_grimoire);
            envoyer_msg(id_file_msg,
                        NUM_TO_MSG_CLIENT(numero_client),
                        NUM_TO_MSG_VENDEUR(numero_vendeur),
                        REDIRECTION,
                        num_redirection
                        );
            log("[Vendeur %d] Je redirige le client %d vers le vendeur %d, spécialiste du rayon %d\n", numero_vendeur, numero_client, num_rayon_client);
            continue;
        } else { // Début de la vente
            attente = rand() % TEMPS_MAX_ATTENTE;
            log("[Vendeur %d] La vente commence, elle va durer %d ns\n", numero_vendeur, attente);
            usleep(attente); // Attente aléatoire
            
            // Fin de la vente
            V(id_sem_vente, numero_vendeur, 1);
            log("[Vendeur %d] La vente s'est terminée\n", numero_vendeur);
            
        }

        // 5. Attente de la décision du client
        msg = recevoir_msg(id_file_msg, NUM_TO_MSG_VENDEUR(numero_vendeur));
        if (msg.info == ANNULER_VENTE) {
            log("[Vendeur %d] Le client %d a annulé la vente.\n", numero_vendeur);
            continue;
        } else {
            // La vente se fait, le montant de la transaction est renseigné dans le SMP
            prix = determiner_prix();
            adr_smp_transactions[numero_client] = prix;
            log("[Vendeur %d] Le client %d a confirmé la vente, pour un montant de %d", numero_vendeur, numero_client, prix);
        }

        // Et au suivant !
        log("[Vendeur %d] Au suivant !\n", numero_vendeur);
    }

    exit(EXIT_SUCCESS);

}