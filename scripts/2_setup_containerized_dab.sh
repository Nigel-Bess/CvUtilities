#!/bin/bash

set -e

# Exit if deps aren't met
if [[ ! -f "/home/fulfil/code/credentials/fulfil-web-dc-sa.json" ]] ; then
    echo '/home/fulfil/code/credentials/fulfil-web-dc-sa.json does not exist! Try copying from another machine in same facility such as running:'
    echo "scp -r fulfil@some-dab.myfacility.fulfil.ai:/home/fulfil/code/credentials ../credentials"
    exit 1
fi

if [[ ! -f "/home/fulfil/code/Fulfil.ComputerVision/alloy/local.env" ]] ; then
    echo '/home/fulfil/code/Fulfil.ComputerVision/alloy/local.env does not exist! You can run the following to copy from a similar machine but be sure to update contents!'
    echo "scp fulfil@some-dab.myfacility.fulfil.ai:/home/fulfil/code/Fulfil.ComputerVision/alloy/local.env" alloy/local.env
    exit 1
fi

if [ ! -d "/home/fulfil/code/Fulfil.TrayCountAPI/models" ]; then
    echo '/home/fulfil/code/Fulfil.TrayCountAPI/models does not exist!? Perhaps copy it from another machine such as:'
    echo "scp -r fulfil@some-dab.myfacility.fulfil.ai:/home/fulfil/code/Fulfil.TrayCountAPI/models" ../Fulfil.TrayCountAPI/models
    exit 1
fi

echo "Setting up GCP now..."
sudo apt-get install -y apt-transport-https ca-certificates gnupg curl
curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo gpg --dearmor -o /usr/share/keyrings/cloud.google.gpg
echo "deb [signed-by=/usr/share/keyrings/cloud.google.gpg] https://packages.cloud.google.com/apt cloud-sdk main" | sudo tee -a /etc/apt/sources.list.d/google-cloud-sdk.list
# locked on version from the TCS setup at Pioneer just in case?
sudo apt-get install -y google-cloud-cli=524.0.0-0

gcloud auth login --cred-file=/home/fulfil/code/credentials/fulfil-web-dc-sa.json
gcloud config set project fulfil-web
echo "Backing up Fulfil.Dispense/configs to gs://cv-configs-backup"
gcloud storage cp -r /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs gs://cv-configs-backup/$HOSTNAME

echo "Setting up docker now..."
echo "Ensuring Docker compose is installed"
# Add Docker's official GPG key:
sudo apt-get update -y
sudo apt-get install ca-certificates curl -y
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

# Add the repository to Apt sources:
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "${UBUNTU_CODENAME:-$VERSION_CODENAME}") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt update -y
sudo apt-get install containerd.io  docker-ce docker-ce-cli docker-buildx-plugin docker-compose-plugin -y
#sudo apt-get install containerd.io  docker-ce docker-ce-cli -y

echo "Hacking docker images etc. to live at /home/fulfil/data/docker instead of /var/lib/docker"
sudo docker rm -f $(docker ps -aq) || echo ""
sudo docker rmi -f $(docker images -q) || echo ""
sudo systemctl stop docker
sudo rm -rf /var/lib/docker
sudo mkdir /var/lib/docker
sudo mkdir /home/fulfil/data/docker -p
sudo mount --rbind /home/fulfil/data/docker /var/lib/docker
sudo systemctl start docker
cat /etc/fstab > stabby
printf "\n/home/fulfil/data/docker /var/lib/docker none rbind,defaults,nofail 0 0" >> stabby
sudo mv stabby /etc/fstab
sudo cat /etc/fstab
echo "/etc/fstab modified for permanent docker nvme storage hack, be sure to test the reboot!"

sudo groupadd docker || echo "docker group already exists"
sudo usermod -aG docker root
sudo usermod -aG docker fulfil
sudo systemctl restart docker
echo "Setup for docker complete!"

echo "Setting up stale file TTL cronjobs"
(crontab -l ; echo "0 9 * * * find /home/fulfil/data/saved_images_* -mtime +60 -type f -exec rm {} \; && find /home/fulfil/data/saved_images_* -empty -type d -delete")| crontab -
(crontab -l ; echo "0 9 * * * find /home/fulfil/data/saved_videos -mtime +60 -type f -exec rm {} \; && find /home/fulfil/data/saved_videos -empty -type d -delete")| crontab -

echo "Everything is nearly setup, lets reboot to make sure there's no disk shenanigans later, enter anything to reboot (and don't forget to run scripts/3_start_services.sh on restart!)"
read ignore
sudo reboot
