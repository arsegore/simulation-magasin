#ifndef _FILE_H_
#define _FILE_H_

#define TAILLE_MAX_FILE 300
#define TAILLE_MEM_FILE sizeof(element) * (TAILLE_MAX_FILE + 4)

typedef int element;
typedef element *file;

file file_vide(int taille);

int est_vide(file f);

/**
 * Renvoie 1 si l'insertion a réussi, 0 sinon
 */
int ajouter(file f, element e);

void retirer(file f);

element premier(file f);

void afficher(file f);

#endif