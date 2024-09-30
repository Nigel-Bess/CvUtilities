#!/bin/bash

# define number of dabs in each pack
ambient=4
cold=6
frozen=2

# to deploy to all PLM A pack dabs: ./massxdeploy.sh ambient
# to deploy to all PLM C pack dabs: ./massxdeploy.sh cold
# to deploy to all PLM F pack dabs: ./massxdeploy.sh frozen
# the first arg is temperature zone
zone=${1}
declare -n zone_ref=$zone
pack_letter=${zone:0:1}
echo About to update $zone_ref machines in ${pack_letter^^} pack - temperature: ${zone^^}
for i in $(seq 1 ${zone_ref})
do 
    ip_address="${pack_letter}${i}-dab.plm.fulfil.ai"
    echo Updating $ip_address now...
    ./xdeploy.sh $ip_address
done
echo Attempts to update complete
