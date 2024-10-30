#!/bin/bash

# define number of dabs in each pack
ambient=4
cold=6
frozen=2

# to deploy to all PLM A pack dabs: ./massxdeploy.sh ambient plm
# to deploy to all PLM C pack dabs: ./massxdeploy.sh cold plm
# to deploy to all PLM F pack dabs: ./massxdeploy.sh frozen plm
# the first arg is temperature zone and second argument is the facility
zone=${1}
facility=${2}

# check if the correct input is provided
if [ -z "$zone" ] || [ -z "$facility" ]; then
	echo "Error: provide correct input for zone & facility..."
	echo "For example, to run script for ambient zone at PLM- ./massxdeploy.sh ambient plm"
	exit 1
fi

if [ "$zone" == "induction" ]; then
	if [ "$facility" == "pio" ]; then
		# hardcoded IP addresses for induction stations for all zones at plm
		ip_address_induction_pio_map["induction"]="10.10.103.62"
		# run deply script for induction station at pio
		for ip in ${ip_address_induction_pio_map["induction"]}; do
			echo Updating $ip now...
			./xdeploy.sh $ip
		done
	else
		# hardcoded IP addresses for induction stations for all zones at plm
		ip_address_induction_plm_map["induction"]="10.40.90.25 10.40.90.38 10.40.90.57 10.40.90.86 10.40.90.65"
		# run deply script for induction station at plm
		for ip in ${ip_address_induction_plm_map["induction"]}; do
			echo Updating $ip now...
			./xdeploy.sh $ip
		done
	fi
else
	if [ "$facility" == "pio" ]; then
		ip_address_dispense_pio_map["dispense"]="10.10.103.88 10.10.103.243"
		# run deploy script for DABs at pio
		for ip in ${ip_address_dispense_pio_map["dispense"]}; do
			echo Updating $ip now...
			./xdeploy.sh $ip
		done
	else
		# run deploy script for DABs at plm
		declare -n zone_ref=$zone
		pack_letter=${zone:0:1}
		echo About to update $zone_ref machines in ${pack_letter^^} pack - temperature: ${zone^^}
		for i in $(seq 1 ${zone_ref}); do 
    			ip_address="${pack_letter}${i}-dab.plm.fulfil.ai"
    			echo Updating $ip_address now...
    			./xdeploy.sh $ip_address
		done
	fi
fi
echo Attempts to update complete
