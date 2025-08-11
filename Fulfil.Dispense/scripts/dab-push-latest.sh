#!/bin/bash
set -e

cd /home/fulfil/code/Fulfil.ComputerVision
git checkout main
git pull
docker build . -f Dispense.arm.Dockerfile -t main
docker tag main gcr.io/fulfil-web/cv-dispense/main:latest
docker push gcr.io/fulfil-web/cv-dispense/main:latest
