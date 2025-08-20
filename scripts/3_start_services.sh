#!/bin/bash

set -e

echo "Linking google auth to Docker"
gcloud config set project fulfil-web-staging-296222
gcloud auth configure-docker --quiet
gcloud config set project fulfil-web
gcloud auth configure-docker --quiet
echo "Setup for GCP complete!"

# start the depthcam docker
cd ~/code/Fulfil.ComputerVision && docker compose -f docker-compose.dab.yml up -d

echo "CV services started!  Starting local docker log stream, exit with ctrl+c..."
docker compose -f docker-compose.dab.yml logs -f
