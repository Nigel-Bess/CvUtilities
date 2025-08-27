#!/bin/bash
set -e

echo -n "Enter Facility Name: " 
read TAG
TAG=${TAG:-none}

echo -n "Warning! This script deploys main:$TAG to all DABs in $TAG at once! Consider running dab-push-latest.sh first."
echo -n "Press any key to continue"
read DUMMY

cd /home/fulfil/code/Fulfil.ComputerVision

raw_file=(`cat Fulfil.Dispense/scripts/machines/$TAG.txt`)
machines=( `echo ${raw_file}` )  
for m in ${machines[@]}
do
  echo "Deploying to $m"
  # For each SSH target, ensure docker compose uses latest facility-specific build and restart,
  # but leave everything else intact
  ssh fulfil@$m "cd /home/fulfil/code/Fulfil.ComputerVision && git checkout origin/main -- docker-compose.dab.yml && docker compose -f docker-compose.dab.yml down depthcam && docker compose -f docker-compose.dab.yml up depthcam -d --remove-orphans"
  echo "Deployed to $m!"
done
