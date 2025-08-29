echo 'Enter name of Fulfil.TCS/assets/ground-truths folder to evaluate against latest 3_eval_fed results (ex. "tcs_init_test"): '
read GROUND_TRUTH_FOLDER
export GROUND_TRUTH_FILE="assets/ground-truths/$GROUND_TRUTH_FOLDER.xml"

python scripts/_4_summarize_bag_clips.py || python3 scripts/_4_summarize_bag_clips.py
