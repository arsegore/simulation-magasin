#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "config.h"

FILE *logs = NULL;

void init_log(){
    if ((logs = fopen(FICHIER_LOGS, "w+") == NULL)){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
}

// Pour écrire à la fois dans le fichier de log et sur le terminal, on va 
// Utiliser cette fonction qui fera deux print (sur stdout et dans le fichier)
void log(const char *msg, ...){
    va_list args1, args2; // il faut une copie sinon seg fault jcrois

    va_start(args1, msg);
    va_copy(args1, args2);
    vprintf(msg, args1);
    vfprintf(logs, msg, args2);

    va_end(args1);
    va_end(args2);
}

void fin_log(){
    fclose(logs);
}