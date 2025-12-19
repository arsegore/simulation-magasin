# Coding Party 2025
_REDACTED_


### Utilisation 
* Compiler avec `make`
* Pour lancer la simulation, l'éxécutable est `bin/initial`qui prend trois paramètres `<nb_vendeurs> <nb_caissiers> <nb_clients>`
* Quand une simulation est en cours, on peut lancer le monitoring avec `bin/monitoring`
* Puisque les logs sont (absolument) illisibles dès qu'on a + de 2 clients, il y a un petit script `logs.sh` prenant les mm paramètres que `bin/initial` et qui coupe le gros fichier de logs en un sous-fichier par caissier/vendeur/client

### Nettoyage
* `make clean` pour nettoyer les objets et éxécutables
* `make clean_logs` pour nettoyer les logs
