#!/bin/bash

set -e

echo "BEGIN: CV & Traycount setup script"
echo "-----------------------------"

# Exit if deps aren't met
if [ ! -d "/home/fulfil/data" ]; then
    echo '/home/fulfil/data does not exist!? Needs the NVMe disk mounted before CV install!'
    exit 1
fi

echo "Disabling any existing systemctl / tmux-based CV services"
sudo systemctl disable depthcam || echo "no depthcam service found to disable"
sudo systemctl disable tray_count || echo "no tray_count service found to disable"
sudo systemctl disable bg_redis_worker || echo "no bg_redis_worker service found to disable"
sudo systemctl disable redis || echo "no redis service found to disable"


echo "Updating repos using your Git username + PAT as password"
if [[ ! -d /home/fulfil/code/Fulfil.TrayCountAPI ]] ; then
  echo "Cloning TrayCount, leveraging your personal Git credentials..."
  mkdir /home/fulfil/code -p
  cd /home/fulfil/code
  git clone https://github.com/Fulfil0518/Fulfil.TrayCountAPI.git
else
  cd /home/fulfil/code/Fulfil.TrayCountAPI
  git checkout master
  git pull
fi

cd /home/fulfil/code/Fulfil.ComputerVision
#git checkout main
git pull

if [[ -d /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs ]] ; then
   echo "Dispense configs folder found! Automatically proceeding to run 2_setup_containerized_dab.sh"
   source /home/fulfil/code/Fulfil.ComputerVision/scripts/2_setup_containerized_dab.sh
   exit 0
fi

echo "Repos are setup, ensure Fulfil.Dispense/configs exists or copy from the most similar machine before continuing to the last step by running"
printf "\n\nbash scripts/2_setup_containerized_dab.sh\n\n"

echo "You can copy from some DAB to your local, then local to this machine with something like:"
echo "scp -r fulfil@p1-dab.pioneer.fulfil.ai:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs/ myconfigs/"
echo "scp -r myconfigs/ fulfil@CHANGEME-dab.pioneer.fulfil.ai:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs/"
