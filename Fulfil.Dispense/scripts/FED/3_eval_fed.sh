# Assumes you've run 1_download_dispenses.sh and runs FED (front edge detection) against all appropriate samples
# generating fed_out.json files that can be converted to COCO for export to annotators like CVAT.

echo 'Enter name of dataset evaluate (ex. "by-id"): '
read DATASET_NAME
export DATASET_NAME=$DATASET_NAME

docker compose run -e TEST_SRC="data/$DATASET_NAME" --remove-orphans dispense_eval "Fulfil.Dispense/app/eval" fed all
echo "Evaluated all samples"
rm -rf data/cvat/FED/test || true
python scripts/FED/_3_generate_fed_cvat.py || python3 scripts/FED/_3_generate_fed_cvat.py
rm -f data/cvat/FED.zip || true

zip -r data/cvat/FED.zip data/cvat/FED/test > /dev/null
echo "Ready to upload data/cvat/FED.zip"
