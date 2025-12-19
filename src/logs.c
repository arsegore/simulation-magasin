#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "config.h"

FILE *logs = NULL;

// Ouvre le fichier de logs
void init_log(){
    logs = fopen(FICHIER_LOGS, "a");
    if (logs == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
}

// pour afficher les noms des rayons plutot que leur numéro dans les logs
// c'est un peu plus parlant...
const char *nom_rayon(int num){
    switch(num){
        case R_PEINTURE: return "Peinture";
        case R_MENUISERIE: return "Menuiserie";
        case R_QUINCAILLERIE: return "Quincaillerie";
        case R_PLOMBERIE: return "Plomberie";
        case R_CHAUFFAGE: return "Chauffage";
        case R_SOUDURE: return "Soudure";
        case R_JARDIN: return "Jardin";
        case R_OUTILLAGE: return "Outillage";
        case R_LUMINAIRES: return "Luminaries";
        case R_DECO: return "Déco";
        default: return "???";
    }
}

// Pour écrire à la fois dans le fichier de log et sur le terminal, on va 
// Utiliser cette fonction qui fera deux print (sur stdout et dans le fichier)
void printlog(const char *msg, ...){
    va_list args1, args2; // il faut une copie sinon seg fault je crois 

    va_start(args1, msg);
    va_copy(args2, args1);
    vprintf(msg, args1);
    if (logs != NULL){
        vfprintf(logs, msg, args2);
        fflush(logs);   // sinon pb avec plusieurs process qui écrivent en mm temps
    }
    va_end(args1);
    va_end(args2);

}

void fin_log(){
    fclose(logs);
}