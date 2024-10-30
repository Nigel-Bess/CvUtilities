#!/bin/bash

# check if the expect is installed on the local machine. If not then install it.
/home/fulfil/code/Fulfil.ComputerVision/scripts/check_expect.sh

# define number of dabs in each pack
ambient=4
cold=6
frozen=2

# to deploy to all PLM A pack induction station: ./massxdeploy.sh ambient
# to deploy to all PLM C pack induction station: ./massxdeploy.sh cold
# to deploy to all PLM F pack induction stations: ./massxdeploy.sh frozen
# the first arg is temperature zone
zone=${1}
declare -n zone_ref=$zone
pack_letter=${zone:0:1}
echo About to update $zone_ref machines in ${pack_letter^^} pack - temperature: ${zone^^}

# Hardcoded IP addresses for each zone
declare -A ip_address_map
ip_address_map["ambient"]="10.40.90.25 10.40.90.38"
ip_address_map["cold"]="10.40.90.57 10.40.90.86"
ip_address_map["frozen"]="10.40.90.65"

# Split the IP addresses into an array
IFS=' ' read -r -a ip_addresses <<< "${ip_address_map[$zone]}"

# Loop through the array and run the deploy script for each IP address
for ip in "${ip_addresses[@]}"; do
  echo "Updating $ip now..."
  /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/xdeploy.sh $ip
  echo "Attempts to update $ip complete."
done

