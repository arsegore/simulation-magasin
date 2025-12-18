#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "config.h"

FILE *logs = NULL;

void init_log(){
    logs = fopen(FICHIER_LOGS, "w+");
    if (logs == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
}

// Pour écrire à la fois dans le fichier de log et sur le terminal, on va 
// Utiliser cette fonction qui fera deux print (sur stdout et dans le fichier)
void printlog(const char *msg, ...){
    va_list args1, args2; // il faut une copie sinon seg fault jcrois

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