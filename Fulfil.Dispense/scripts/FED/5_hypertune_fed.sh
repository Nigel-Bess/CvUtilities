# Find the best hyperparameters for FED by feeding in many different values via env vars

echo 'Enter name of dataset to evaluate in Fulfil.Dispense/data/ (ex. "by-id"): '
read DATASET_NAME
export DATASET_NAME=$DATASET_NAME

python scripts/FED/_5_hypertune_fed.py || python3 scripts/FED/_5_hypertune_fed.py
