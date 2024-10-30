#!bin/bash

# arguments
ip=$1
local_version_output=$2

# get git version on remote
remote_version_output=$(ssh fulfil@$ip "/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main --version | grep 'Dispense API commit details' | awk '{print \$5}'")
# get hostname
hostname=$(ssh fulfil@$ip "hostname")

# compare the local and remote versions
#if [[ "$local_version_output" != "$remote_version_output" ]]; then
#	echo "Version mismatched on machine with IP: $ip"
#	echo "Local version: $local_version_output"
#	echo "Remote version: $remote_version_output"
#else
#	echo "Version matched on machine with IP: $ip"
#fi

# output
echo "$hostname|$remote_version_output"

