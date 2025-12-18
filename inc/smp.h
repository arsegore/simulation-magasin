#ifndef _FILS_C_
#define _FILS_C_

#include <sys/shm.h>

int init_smp(int taille, int qui, int quel);

void *attacher_smp(int id);

void detacher_smp(void *adr);

void supprimer_smp(int id);

#endif