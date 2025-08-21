#/bin/bash

set -e

echo 'Enter name of dataset (use "by-id" if using all available requests in data/raw): '
read DATASET_NAME
export DATASET_NAME=$DATASET_NAME

echo "data/raw will be formatted into data/$DATASET_NAME"

# Potentially filter
echo 'Optionally enter a filename=contents whitelist filter or blank to keep from data/raw: (ex. PreFrontEdgeDistance/json_request.json="Shiny":false) '
read REQUEST_FILTER
export REQUEST_FILTER=$REQUEST_FILTER

# Potentially trim 
echo "Enter max number of (filtered) random sample requests to copy into data/$DATASET_NAME: "
read REQUEST_COUNT
export REQUEST_COUNT=$REQUEST_COUNT

python scripts/_2_split_dispense_sets.py || python3 scripts/_2_split_dispense_sets.py
