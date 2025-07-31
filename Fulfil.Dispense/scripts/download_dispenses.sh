#/bin/bash

set -e

echo "Checking pip packages"
pip install -r scripts/requirements.txt || pip3 install -r scripts/requirements.txt

printf "\n\nPIP is up-to-date\n\n"
echo "Enter SCP/SSH target (ex. fulfil@p1-dab.pioneer.fulfil.ai): "
read TARGET

# Download raw data
mkdir -p ./data/raw
rsync -r --exclude "c*" --exclude "a*" --exclude "l*" --exclude "t*" --include "saved_images_*" --progress "$TARGET:/home/fulfil/data/" ./data/raw/

python scripts/_download_dispenses.py || python3 scripts/_download_dispenses.py
