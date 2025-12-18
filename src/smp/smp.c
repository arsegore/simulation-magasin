#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "smp.h"
#include "cle.h"

int init_smp(int taille, int qui, int quel){
    int res, flags;
    if (qui == PERE) {
        flags = IPC_CREAT | IPC_EXCL | 0660;
    } else {
        flags = 0660;
    }
    if ((res = shmget(generer_cle(quel), taille, flags)) == -1){
        perror("shmget");
        fprintf(stderr, "Impossible de créer le SMP.\n");
        exit(-1);
    } else return res;
}

void *attacher_smp(int id){
    void *res;
    if ((res = shmat(id, NULL, 0)) == NULL) {
        perror("shmat");
        fprintf(stderr, "Impossible d'attacher le SMP.\n");
        exit(-1);
    } else return res;
}

void detacher_smp(void *adr){
    if (shmdt(adr) == -1){
        perror("shmdt");
        fprintf(stderr, "Impossible de détacher le SMP.\n");
        exit(-1);
    }
}

void supprimer_smp(int id){
    if (shmctl(id, IPC_RMID, NULL) == -1){
        perror("shmctl");
        fprintf(stderr, "Impossible de supprimer le SMP.\n");
        exit(-1);
    }
}