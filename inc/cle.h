#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdlib.h>
#include "config.h"

#define FICHIER_CLE "./cle"

/**
 * Ids utilisés par ftok() pour générer les id d'IPC
 */
#define ID_SMP_OCCUPATION_VENDEURS  0
#define ID_SMP_FILES_VENDEURS       1
#define ID_SMP_FILES_CAISSIERS      2
#define ID_SMP_GRIMOIRE             3

#define ID_SEM_MUTEX_OCCUP_V        4
#define ID_SEM_VENDEURS             5
#define ID_SEM_CAISSIERS            6
#define ID_FILE_MSG                 7
#define ID_SMP_TRANSACTIONS         8
#define ID_SEM_VENTE                9
#define ID_SEM_PAIEMENT             10

#define TAILLE_TAB_VENDEUR          sizeof(int) * NB_VENDEURS_MAX
#define TAILLE_TAB_CLIENT           sizeof(int) * NB_CLIENTS_MAX
#define TAILLE_TAB_CAISSIER         sizeof(int) * NB_CAISSIERS_MAX

#define ID_SEM 2

#define PERE 1
#define FILS 2

int generer_cle(int id);

#endif
