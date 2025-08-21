
echo 'Enter name of Fulfil.Dispense/metrics folder to evaluate against latest 3_eval_fed results (ex. "plm"): '
read GROUND_TRUTH_FOLDER
export GROUND_TRUTH_FILE="metrics/$GROUND_TRUTH_FOLDER/FED-ground-truth.xml"

python scripts/FED/_4_fed_summarize.py || python3 scripts/FED/_4_fed_summarize.py
#open "data/cvat/FED/vis.png"