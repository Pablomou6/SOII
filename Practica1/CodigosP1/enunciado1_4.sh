#!/bin/bash

cd ~

lineas_antes=$(wc -l passwd_copia.txt)
echo "Lineas antes de copiar: $lineas_antes"

cat /etc/passwd >> passwd_copia.txt

lineas_despues=$(wc -l passwd_copia.txt)
echo "Lineas despues de copiar: $lineas_despues"

#Una vez ejecutado, si lo ejecutamos otra vez ya no será el doble

