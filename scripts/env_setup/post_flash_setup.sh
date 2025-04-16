#!/bin/bash


echo "BEGIN: Post-flash setup script"
echo "-----------------------------"

CV_REPO_PATH='/home/fulfil/code/Fulfil.ComputerVision'
SCRIPTS_PATH="${CV_REPO_PATH}/scripts"
USR_BIN_PATH='/usr/bin'
TRAYCOUNT_REPO_PATH='/home/fulfil/code/Fulfil.TrayCountAPI'

# enable useful scripts for the GRU Manage DC/SBC buttons
echo "Enabling useful scripts for the GRU Manage DC/SBC buttons..."
cd ${SCRIPTS_PATH}
cp dc-startup ${USR_BIN_PATH}/dc-startup
cp dc-stop ${USR_BIN_PATH}/dc-stop
cp dc-restart ${USR_BIN_PATH}/dc-restart
echo "Done"
echo " "

# enable auto-startup of the services when the computer is powered on
echo "Enabling auto-startup of the services when the computer is powered on..."
sudo cp bg_redis_worker.service /etc/systemd/system/
sudo systemctl enable --now bg_redis_worker.service
sudo cp tray_count.service /etc/systemd/system/
sudo systemctl enable --now tray_count.service
sudo cp depthcam.service /etc/systemd/system/
sudo systemctl enable --now depthcam.service
echo "Done" 
echo " "

# enable uploading to GCS
FRIENDLY_IP="10.10.103.243"
echo "Enabling uploading to GCS by copying credentials from ${FRIENDLY_IP}..."
scp -r fulfil@${FRIENDLY_IP}:${TRAYCOUNT_REPO_PATH}/credentials ${TRAYCOUNT_REPO_PATH}/credentials

# don't forget to:
#   - verify the serial numbers are correctly setup in the AGX_specific_main.ini file by sending TrayView and FloorView requests and comparing the images to reality!
#   - copy all of the relevant configs to the Fulfil.Dispense/configs repo!
#   - copy and correctly format the tray_calibration_data_{POSITION}.ini file from the Calibration Output Log google doc!
#   - copy the ML models to the correct path!
#   - run the TC API setup script for the most up to date tray-ml env!

echo " "
echo "Script complete!"
