#!/bin/bash
set -e

# Read in local env
source /home/fulfil/code/Fulfil.ComputerVision/.env

echo -n "Enter Branch Name (or blank for main): " 
read BRANCH
BRANCH=${BRANCH:-main}

echo -n "Enter Facility Name (or blank for $LOCATION): " 
read TAG
TAG=${TAG:-$LOCATION}

cd /home/fulfil/code/Fulfil.ComputerVision
echo "Ensuring Git up-to-date (use your GH username / PAT token): "
git fetch
git checkout $BRANCH
echo "Checked out $BRANCH, building and pushing to gcr.io/fulfil-web/cv-dispense/$BRANCH:$TAG"
docker build . -f Dispense.arm.Dockerfile -t $BRANCH
docker tag $BRANCH gcr.io/fulfil-web/cv-dispense/$BRANCH:$TAG
docker push gcr.io/fulfil-web/cv-dispense/$BRANCH:$TAG

