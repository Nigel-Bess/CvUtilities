#!/bin/bash
set -e

cd /home/fulfil/code/Fulfil.ComputerVision

# Pull latest img in the background for faster restart
docker compose -f docker-compose.dab.yml pull

# Graceful shutdown
docker compose -f docker-compose.dab.yml down

# Start all services again
docker compose -f docker-compose.dab.yml up -d