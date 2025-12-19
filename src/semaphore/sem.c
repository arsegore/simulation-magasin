#include <stdlib.h>
#include <sys/sem.h>
#include <stdio.h>
#include <errno.h>
#include "sem.h"
#include "cle.h"

void P(int id_sem, int sem, int n){
    struct sembuf op ={sem, -n , SEM_UNDO };
    if (semop(id_sem, &op, 1) == -1) {
        perror("semop P");
        exit(EXIT_FAILURE);
    }
}

void V(int id_sem, int sem, int n){
    struct sembuf op ={sem, n , SEM_UNDO };
    if (semop(id_sem, &op, 1) == -1) {
        perror("semop V");
        exit(EXIT_FAILURE);
    }
}

int init_sem(int id, int nb_sem, int qui, int val_init) {
    int res, flags;
    key_t cle = generer_cle(id);
    int i;
    
    if (cle == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    
    if (qui == PERE) {
        flags = IPC_CREAT | IPC_EXCL | 0660;
    } else {
        flags = 0660;
    }
    
    res = semget(cle, nb_sem, flags);
    if (res == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    
    if (qui == PERE) {
        for (i = 0; i < nb_sem; i ++){
            if (semctl(res, i, SETVAL, val_init) == -1) {
                perror("semctl SETVAL");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    return res;
}

void supprimer_sem(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

int taille_file_sem(int sem_id, int sem) {
    int res;
    res = semctl(sem_id, sem, GETNCNT);
    if (res == -1) {
        if (errno == EIDRM || errno == EINVAL) {
            printf("[Monitoring] La simulation a pris fin. Fermeture...\n"); // comme cette fonction est utilsiée que dans le monitoring, je me permets
        } else {
            perror("semctl");
        }
        exit(EXIT_FAILURE);
    }
    return res;
}