#ifndef _MONITORING_H_
#define _MONITORING_H_

// qlq valeurs utiles pour le monitoring
typedef struct {
    int     nb_clients;
    int     nb_vendeurs;
    int     nb_caissiers;
    int     total_ventes;
    int     clients_entres;
    int     clients_sortis;
} monit_infos;

#endif