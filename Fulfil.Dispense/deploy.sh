#!/bin/bash
start=$SECONDS
name="$1@$2 -> "
echo "$name Update starting $3"
echo "$name rsync'ing dc-main binary"
echo "----> rsync -------------------------------------------------------->"
rsync -avP ../Fulfil.Dispense/build/app/main fulfil@$2:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main
echo "<-------------------------------------------------------- rsync <----"
echo "$name rsync binary update done"
if [[ $3 ]]; then
    if [[ $3 == "y" ]]; then
        echo "Restarting DC Services"
        (ssh fulfil@$2 " dc-startup kill && dc-startup start")
    elif [[ $3 == "n" ]]; then
        echo "Not restarting FW"
    fi
else
    read -p "$name Restart DC Services? (y/n): " yn
    case $yn in
        y | Y)  echo "$name Restarting DC Services" 
                (ssh fulfil@$2 " dc-startup kill && dc-startup start")
                ;;
        * )
                echo "$name Not restarting services"
                ;;
    esac
fi
duration=$(( SECONDS - start ))
echo "$name G2G in $duration sec"
