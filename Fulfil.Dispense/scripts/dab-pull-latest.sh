#!/bin/bash
set -e

# Read in local env
source /home/fulfil/code/Fulfil.ComputerVision/.env

echo -n "Warning! This script deploys builds and will restart CV at the end!"

echo -n "Enter Branch Name (or blank for main): " 
read BRANCH
TAG=${BRANCH:-main}

echo -n "Enter Facility Name (or blank for $LOCATION): " 
read TAG
TAG=${TAG:-$LOCATION}

cd /home/fulfil/code/Fulfil.ComputerVision

echo "Ensuring Git up-to-date (use your GH username / PAT token): "
# Force compose file to be fresh from GH to avoid Git conflicts
git checkout origin/$BRANCH -- ../docker-compose.dab.yml
git fetch
git checkout $BRANCH

if [ "$BRANCH" != "main" ]; then
  echo "Hacking docker-compose.dab.yml to reference $BRANCH:$TAG in GCP"
  vim /home/fulfil/code/Fulfil.ComputerVision/docker-compose.dab.yml -c "%s/image\: gcr\.io\/fulfil-web\/cv-dispense\/.*$/image: gcr.io\/fulfil-web\/cv-dispense\/$BRANCH:$TAG/g" -c ":wq!"

  echo "Edit made: "
  cat /home/fulfil/code/Fulfil.ComputerVision/docker-compose.dab.yml | grep depthcam -A 3 | grep image -A 3 -B 3
fi

# Pull latest img in the background for min downtime
docker compose -f docker-compose.dab.yml pull

# Graceful shutdown
docker compose -f docker-compose.dab.yml down

# Start all services again
docker compose -f docker-compose.dab.yml up -d --remove-orphans
