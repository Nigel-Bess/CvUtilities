#!/bin/bash

# to run this script for example for "plm" facility - ./mass_version_check.sh plm

# parse arguments
#zone=${1}
facility=${1}

# SSH login credential
user="fulfil"
password="FreshEngr"

# IP addresses of induction stations for each zone at Pio
declare -A ip_address_map_ind_pio
ip_address_map_ind_pio["induction"]="10.10.103.62"

# IP addresses of induction stations for each zone at PLM
declare -A ip_address_map_ind_plm
ip_address_map_ind_plm["ambient"]="10.40.90.25 10.40.90.38"
ip_address_map_ind_plm["cold"]="10.40.90.57 10.40.90.86"
ip_address_map_ind_plm["frozen"]="10.40.90.65"

# IP addresses of dispense stations at Pioneer
declare -A ip_address_map_disp_pio
ip_address_map_disp_pio["dispense"]="10.10.103.88 10.10.103.243"

# check if the expect is installed on the local machine. If not then install it.
/home/fulfil/code/Fulfil.ComputerVision/scripts/check_expect.sh

# get the git version from the local machine
local_version_output=$(/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main --version | grep "Dispense API commit details" | awk "{print \$5}")
echo "local version#:$local_version_output"

# dynamic array to store mismatched versions
declare -a mismatched_versions=()
declare -a matched_versions=()

# helper function to check versions
check_version() {
	local ip=$1
	output=$(/home/fulfil/code/Fulfil.ComputerVision/scripts/version_check.sh "$ip" "$local_version_output")
	echo "Output from version_check.sh: $output"
	
	hostname=$(echo $output | cut -d'|' -f1)
	remote_version=$(echo $output | cut -d'|' -f2)
	# check if output contains '|' indicating a mismatch
	if [[ $output == *"|"* ]]; then
		mismatched_versions+=("Hostname: $hostname, IP: $ip, Remote Version: $remote_version")
	else
		matched_versions+=("Hostname: $hostname, IP: $ip, Remote Version: $remote_version")
	fi
}

echo "Finding versions for remote induction stations at $facility..."
ip_addresses_ind=()  # initialize an empty array
# split the IP addresses into an array
if [ "$facility" == "pio" ]; then
	IFS=' ' read -r -a temp_ip_addresses_ind <<< "${ip_address_map_ind_pio["induction"]}"
	# append all temp IP addresses to final array
	ip_addresses_ind+=("${temp_ip_addresses_ind[@]}")
elif [ "$facility" == "plm" ]; then
	zones=("ambient" "cold" "frozen")
	#loop through each zone and process IP address
	for zone in "${zones[@]}"; do
		IFS=' ' read -r -a temp_ip_addresses_ind <<< "${ip_address_map_ind_plm[$zone]}"
		# append all temp IP addresses to final array
		ip_addresses_ind+=("${temp_ip_addresses_ind[@]}")
	done
else
	echo "Error:enter the correct facility! (like- plm, pio)"
	exit 0
fi

# loop through the array and find version# for each IP address for induction stations at facility
for ip in "${ip_addresses_ind[@]}"; do
	check_version "$ip"
done
echo ""
echo "--------------------------------------------------------------------"
echo ""

if [ "$facility" == "plm" ]; then
	echo "Now finding versions for remote DABs at PLM..."
	# define number of dabs in each pack
	ambient=4
	cold=6
	frozen=2

	# define zones
	zones=("ambient" "cold" "frozen")

	# loop through each zone and process IP adresses
	for zone in "${zones[@]}"; do
		echo "Processing zone: $zone"
		
		# loop through the dynamic IP addresses and find version# for each IP address for dispense at facility
		declare -n zone_ref=$zone
		pack_letter=${zone:0:1}
		for i in $(seq 1 ${zone_ref}); do
			ip_address="${pack_letter}${i}-dab.plm.fulfil.ai"
			echo "processing dynamic IP: $ip_address"
			check_version "$ip_address"
		done
	done
	echo ""
	echo "--------------------------------------------------------------------"
	echo ""

elif [ "$facility" == "pio" ]; then
	echo "Now finding versions for remote DABs at Pioneer..."
	# loop through the array and find version# for each IP address for dispense stations at facility
	IFS=' ' read -r -a ip_addresses_disp <<< "${ip_address_map_disp_pio["dispense"]}"
	for ip in "${ip_addresses_disp[@]}"; do
		check_version "$ip" 
	done
	echo ""
	echo "--------------------------------------------------------------------"
	echo ""
fi

echo "local version#:$local_version_output"
# print results if matches were found
if [ ${#matched_versions[@]} -gt 0 ]; then
	echo ""
	echo "Matched versions:"
	echo "------------------------------------------"
	for match in "${matched_versions[@]}"; do
		echo "$match"
	done
else
	echo "No remote versions match the local version."
fi
# print results if mismatches were found
if [ ${#mismatched_versions[@]} -gt 0 ]; then

	echo ""
	echo "Mismatched versions:"
	echo "------------------------------------------"
	for mismatch in "${mismatched_versions[@]}"; do
		echo "$mismatch"
	done
else
	echo "All remote versions match the local version."
fi
echo ""
