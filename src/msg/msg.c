#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "cle.h"
#include "msg.h"

void envoyer_msg(int id_file, int destinataire, int num_exp, int info, int valeur){
    message msg;

    msg.mtype = destinataire;
    msg.qui_envoie = num_exp;
    msg.info = info;
    msg.valeur = valeur;

    if (msgsnd(id_file, &msg, sizeof(message) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

message recevoir_msg(int id_file, int destinataire) {
    message msg;

    if (msgrcv(id_file, &msg, sizeof(message) - sizeof(long), destinataire, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    return msg;
}

int init_file_msg(int id, int qui) {
    int id_file;
    int flags;
    key_t cle = generer_cle(id);

    if (cle == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    if (qui == PERE) {
        flags = IPC_CREAT | IPC_EXCL | 0660;
    } else {
        flags = 0660;
    }

    id_file = msgget(cle, flags);
    if (id_file == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    return id_file;
}

void supprimer_file_msg(int id_file) {
    if (msgctl(id_file, IPC_RMID, NULL) == -1) {
        perror("msgctl IPC_RMID");
        exit(EXIT_FAILURE);
    }
}