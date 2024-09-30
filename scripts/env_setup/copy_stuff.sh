#!/bin/bash

cd /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense
mkdir -p configs/secret
rsync -avP fulfil@c6-dab.plm.fulfil.ai:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/configs/ configs