#ifndef _SEM_H_
#define _SEM_H_

/**
 * Prend n * sem dans id_sem
 */
void P(int id_sem, int sem, int n);

/**
 * Rend n * sem dans id_sem
 */
void V(int id_sem, int sem, int n);

int init_sem(int id, int nb_sem, int qui, int val_init);

void supprimer_sem(int id_sem);

#endif