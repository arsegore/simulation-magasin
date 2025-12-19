/**
 * Processus caissier
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

int numero_caissier;
int id_sem_dispo_vendeurs;
int id_sem_dispo_caissiers;
int id_file_msg;
int recommencer = 1;
int id_sem_paiement;    
int id_smp_transactions;
int *adr_smp_transactions;
message msg;
sigset_t tout_bloquer, att_sigusr1, att_sigusr2;
int id_smp_monitoring;
monit_infos *adr_smp_monitoring;
int id_mutex_monitoring;


void handler_sigusr2(){
    printlog("[Caissier %d] SIGUSR2 reçu\n", numero_caissier);
    exit(EXIT_SUCCESS);
}

void handler_sigusr1(){
}

void nettoyer(){
    printlog("[Caissier %d] Nettoyage...\n", numero_caissier);
    if (adr_smp_transactions != NULL) {
        detacher_smp(adr_smp_transactions);
    }
    if (adr_smp_monitoring != NULL){
        detacher_smp(adr_smp_monitoring);
    }
}

int determiner_prix(){
    return rand() % (PRIX_MAX - PRIX_MIN) + PRIX_MIN;
}

int main(int argc, char *argv[]){
    int numero_client;
    int nb_caissiers;
    int montant;
    int attente;

    numero_caissier = atoi(argv[1]);
    nb_caissiers = atoi(argv[3]);

    srand(time(NULL) * getpid());

    atexit(nettoyer);   // vu qu'on exit dans tous les cas (erreur ou normal), ça permet d'assurer le nettoyage dans ttes les situations
    init_log();

    // 0. Gestion des signaux
    sigfillset(&tout_bloquer);
    sigdelset(&tout_bloquer, SIGUSR2);
    sigprocmask(SIG_SETMASK, &tout_bloquer, NULL); // On bloque TOUS les signaux
    sigfillset(&att_sigusr1);
    sigdelset(&att_sigusr1, SIGUSR1);
    sigfillset(&att_sigusr2);
    sigdelset(&att_sigusr2, SIGUSR2);
    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);

    // 1. Récupération des IPC    
    id_file_msg = init_file_msg(ID_FILE_MSG, FILS);
    id_sem_paiement = init_sem(ID_SEM_PAIEMENT, nb_caissiers, FILS, 0);
    id_sem_dispo_caissiers = init_sem(ID_SEM_CAISSIERS, nb_caissiers, FILS, 0);
    id_smp_transactions = init_smp(TAILLE_TAB_CLIENT, FILS, ID_SMP_TRANSACTIONS);
    adr_smp_transactions = (int *) attacher_smp(id_smp_transactions);
    id_smp_monitoring = init_smp(sizeof(monit_infos), FILS, ID_SMP_MONITORING);
    adr_smp_monitoring = (monit_infos *) attacher_smp(id_smp_monitoring);
    id_mutex_monitoring = init_sem(ID_MUTEX_MONITORING, 1, FILS, 1);

    printlog("[Caissier %d] Création...\n", numero_caissier);

    /**********************
     * ATTENTE DU DEMARRAGE
     **********************/
    sigsuspend(&att_sigusr1);

    printlog("[Caissier %d] Démarrage. \n", numero_caissier);

    while (recommencer) {
        // 2. Le caissier est prêt à accueillir un client
        V(id_sem_dispo_caissiers, numero_caissier, 1);

        msg = recevoir_msg(id_file_msg, numero_caissier, MSG_CAISSIER, ENTREE_CAISSE);
        numero_client = msg.valeur;
        // vérifier qu'il s'agit du bon type de msg ?

        printlog("[Caissier %d] Le client %d est arrivé à ma caisse.\n", numero_caissier, numero_client);

        // 3. Récupére le montant de la transaction
        montant = adr_smp_transactions[numero_client];
        adr_smp_transactions[numero_client] = -1;

        printlog("[Caissier %d] J'ai trouvé une transaction d'un montant de %d pour le client %d\n", numero_caissier, montant, numero_client);

        // 4. On communqiue le prix au client
        envoyer_msg(id_file_msg, numero_client, MSG_CLIENT, PAIEMENT, numero_caissier, montant);

        // 5. Paiement
        attente = rand() % DUREE_MAX_PAIEMENT;
        printlog("[Caissier %d] Début du paiement, qui va durer %d ns\n", numero_caissier, attente);
        usleep(attente); // Attente aléatoire
            
        // Fin du paiement
        V(id_sem_paiement, numero_caissier, 1);

        P(id_mutex_monitoring, 0, 1);
        adr_smp_monitoring->total_ventes+= montant;
        V(id_mutex_monitoring, 0, 1);

        // Et au suivant !
        printlog("[Caissier %d] Paiement terminé, au suivant !\n", numero_caissier);
    }

    exit(EXIT_SUCCESS);

}