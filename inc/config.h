#ifndef _CONFIG_H_
#define _CONFIG_H_

// Les rayons du magasin 
#define NB_RAYONS           10

#define R_PEINTURE          0
#define R_MENUISERIE        1
#define R_QUINCAILLERIE     2
#define R_PLOMBERIE         3
#define R_CHAUFFAGE         4
#define R_SOUDURE           5
#define R_JARDIN            6
#define R_OUTILLAGE         7
#define R_LUMINAIRES        8
#define R_DECO              9

// Bornes pour les tirages 
#define TEMPS_MAX_ATTENTE   15000  // en nanosecondes
#define TAUX_PROBA_VENTE    0.7
#define PRIX_MIN            15
#define PRIX_MAX            300

#define NB_VENDEURS_MAX     200
#define NB_CLIENTS_MAX      200
#define NB_CAISSIERS_MAX    200

#endif