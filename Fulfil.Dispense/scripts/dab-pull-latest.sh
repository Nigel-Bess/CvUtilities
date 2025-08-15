#!/bin/bash
set -e

echo -n "Enter Branch Name: " 
read BRANCH

cd /home/fulfil/code/Fulfil.ComputerVision

echo "Ensuring Git up-to-date (use your GH username / PAT token): "
# Force compose file to be fresh from GH to avoid Git conflicts
git checkout origin/main -- ../docker-compose.dab.yml 
git fetch
git checkout $BRANCH

if [ "$BRANCH" == "main" ]
    echo "Ensuring docker-compose.dab.yml references main"
else
    echo "Forcing docker-compose.dab.yml to reference $BRANCH in GCP"
fi
vim /home/fulfil/code/Fulfil.ComputerVision/docker-compose.dab.yml -c "%s/image\: gcr\.io\/fulfil-web\/cv-dispense\/.*$/image: gcr.io\/fulfil-web\/cv-dispense\/$BRANCH:latest/g" -c ":wq!"

# Pull latest img in the background for faster restart
docker compose -f docker-compose.dab.yml pull

# Graceful shutdown
docker compose -f docker-compose.dab.yml down

# Start all services again
docker compose -f docker-compose.dab.yml up -d