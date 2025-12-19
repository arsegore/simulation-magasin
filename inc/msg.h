#ifndef _MSG_H_
#define _MSG_H_


// Type du message = NUMERO + TYPE + OPERATION 
// Ca permet d'avoir un code unique pour attendre un message
// Par ex, le vendeur 21 attend le choix d'un client
// il va attendre un msg de mtype = 21 + MSG_VENDEUR + FIN_VENTE
#define MSG_VENDEUR     1000
#define MSG_CAISSIER    2000
#define MSG_CLIENT      3000

// Le type d'opération du message
#define ACCUEIL                  10000
#define REPONSE                  20000
#define CHOIX_CLIENT             30000
#define FIN_VENTE                40000
#define ENTREE_CAISSE            50000
#define PAIEMENT                 60000

// Valeur de l'opération (soit une des macros, soit un numéro de vendeur, soit un prix)
#define CLIENT_CONFIRME_VENTE    1
#define CLIENT_ANNULE_VENTE      0
#define VALIDER_RAYON            -1


// Une seule structure générique pour tous les messages
typedef struct {
    long    mtype;
    int     qui_envoie;
    int     valeur;
} message;

void envoyer_msg(int id_file, int numero_destinataire, int qui, int quoi, int numero_expediteur, int valeur);

message recevoir_msg(int id_file, int numero_destinataire, int qui, int quoi);

int init_file_msg(int id, int qui);

void supprimer_file_msg(int id_file);

#endif