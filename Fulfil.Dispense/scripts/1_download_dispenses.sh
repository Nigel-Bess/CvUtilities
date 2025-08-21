#/bin/bash

set -e

echo "Checking pip packages"
pip install -r scripts/requirements.txt || pip3 install -r scripts/requirements.txt

printf "\n\nPIP is up-to-date\n\n"
echo "Enter SCP/SSH target (ex. fulfil@p1-dab.pioneer.fulfil.ai): "
read TARGET

# Download raw data
mkdir -p ./data/raw
rsync -r --exclude "live_image*" --exclude "lane_dispense_dat*" --exclude "saved_video*" --include "saved_images_*" --progress "$TARGET:/home/fulfil/data/" ./data/raw/

echo "Download done! Auto-running 2_split_dispense_sets.sh next..."
source 2_split_dispense_sets.sh