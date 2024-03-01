#!/bin/bash

# This scrip will mount the SD card to $DIR
# It assumes only one SD card per system


# this is janky but the device has shown up as SR128
# instead of SD. We should revist this later with other cards
DEVS=$(ls -l /dev/disk/by-id/mmc-S*128*)

DIR=/home/fulfil/data/
echo " ... MOUNTING SD ...."
while IFS= read -r line; do
    if [[ "$line" == *"part1"* ]]; then
        DEV=${line:(-9)}
        #echo " ... $DEV ...."
        DEV=/dev/$DEV
        echo will attempt to mount SD found here: $DEV to $DIR
        mkdir -p $DIR
        mount $DEV $DIR
        if [ $? -eq 0 ]; then
            echo "$DEV Mounted"
            chown -R fulfil:fulfil $DIR
        else
            echo " $DEV not Mounted"
        fi
    fi
done <<< "$DEVS"
