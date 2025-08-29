# Assumes you've run 1_download_dispenses.sh and runs bag_clips against all appropriate samples
# generating bag_clips.json machine label files that can be converted to COCO for export to annotators like CVAT.

set -e

echo 'Enter name of dataset evaluate (ex. "by-id"): '
read DATASET_NAME
export DATASET_NAME=$DATASET_NAME

#docker compose --profile tcs_bag_clips up --build
docker compose run -e TEST_SRC="data/$DATASET_NAME" --remove-orphans tcs_eval_bag_clips "Fulfil.TCS/build/eval" bag_clips all
echo "Evaluated samples"
rm -rf data/cvat/bag_clips/test || true
python3 scripts/_3_eval_bag_clips.py || python scripts/_3_eval_bag_clips.py
rm -f data/cvat/bag_clips.zip || true

zip -r data/cvat/bag_clips.zip data/cvat/bag_clips/test > /dev/null
echo "Ready to upload data/cvat/bag_clips.zip"
