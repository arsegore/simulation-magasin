#ifndef _MSG_H_
#define _MSG_H_

// Permet de stocker dans mtype le numéro destinataire + son type 
// Pour le vendeur 32, on stockera 1032. Le client qui attend un msg
// du vendeur 32 attendra donc un msg de mtype 32 + MSG_VENDEUR (1000) = 1032
#define MSG_VENDEUR     1000
#define MSG_CAISSIER    2000
#define MSG_CLIENT      3000

// Quelques macros pour éviter de répéter le calcul...
#define NUM_TO_MSG_VENDEUR(num) ((num) + MSG_VENDEUR)
#define NUM_TO_MSG_CLIENT(num) ((num) + MSG_CLIENT)
#define NUM_TO_MSG_CAISSIER(num) ((num) + MSG_CAISSIER)
#define MSG_TO_NUM_VENDEUR(num) ((num) - MSG_VENDEUR)
#define MSG_TO_NUM_CLIENT(num) ((num) - MSG_CLIENT)
#define MSG_TO_NUM_CAISSIER(num) ((num) - MSG_CAISSIER)

// L'opération à laquelle correspond un msg
#define DEMANDER_NUM_RAYON  0
#define VALIDER_RAYON       1
#define REDIRECTION         2
#define VALIDER_VENTE       3
#define ANNULER_VENTE       4
#define ENVOYER_NUMERO      5
#define ANNONCER_PRIX       6
#define CONFIRMER_VENTE     7

// Une seule structure générique pour tous les messages
typedef struct {
    long    mtype;          // Num. destinataire + MSG_X
    int     qui_envoie;
    int     info;
    int     valeur;
} message;

void envoyer_msg(int id_file, int destinataire, int num_exp, int info, int valeur);

message recevoir_msg(int id_file, int destinataire);

int init_file_msg(int id, int qui);

void supprimer_file_msg(int id_file);

#endif