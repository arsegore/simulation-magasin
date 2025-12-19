#!/usr/bin/env bash

# Petit script pour découper le grooos fichier de logs en un fichier par client/vendeur/caissier
# (c juste des cat|grep en boucle)

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <nb_vendeurs> <nb_caissiers> <nb_clients>"
    exit 1
fi

NB_VENDEURS=$1
NB_CAISSIERS=$2
NB_CLIENTS=$3
FICHIER_LOG="logs.txt"

mkdir -p logs

echo "> Découpage des logs..."

for ((i=0; i<NB_VENDEURS; i++)); do
    cat "$FICHIER_LOG"|grep "\[Vendeur $i\]" > "logs/vendeur$i.txt"
done

for ((i=0; i<NB_CAISSIERS; i++)); do
    cat "$FICHIER_LOG"|grep "\[Caissier $i\]" > "logs/caissier$i.txt"
done

for ((i=0; i<NB_CLIENTS; i++)); do
    cat "$FICHIER_LOG"|grep "\[Client $i\]" > "logs/client$i.txt"
done

echo "> Terminé, tous les fichiers sont dans logs/"