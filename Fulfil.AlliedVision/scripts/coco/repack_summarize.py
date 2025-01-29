# Final step after manually labeling the whole COCO dataset to assets/ground-truths/<facility>.json

import os
import json

test_dir = "data/test"
labelset_name = input("Enter assets/ground-truths label set name (ex. 'plm' for plm.json): ")

machine_truth_file = "assets/test-outputs/{}.json".format(labelset_name)
ground_truth_file =  "assets/ground-truths/{}.json".format(labelset_name)

# Throw if either files is missing
if not os.path.exists(machine_truth_file):
    raise ValueError("Machine test labels file not found: {}".format(machine_truth_file))
if not os.path.exists(ground_truth_file):
    raise ValueError("Ground truth labels file not found: {}".format(ground_truth_file))

# parse a COCO json file to a boolean is_bag_empty map keyed by request ID
def parse_coco_label_map(json_file):
    print("open " + json_file)
    with open(json_file) as f:
        coco = json.load(f)
        img_to_labelled = {}

        labeled_img_ids = {}
        for label in coco["annotations"]:
            labeled_img_ids[label["image_id"]] = True

        for img in coco["images"]:
            req_id = img["file_name"][0:img["file_name"].index(".")]
            if img["id"] in labeled_img_ids:
                img_to_labelled[req_id] = False
            else:
                img_to_labelled[req_id] = True
        return img_to_labelled

# Parse both files to JSON
ground_truth_map = parse_coco_label_map(ground_truth_file)
machine_truth_map = parse_coco_label_map(machine_truth_file)

# Build confusion matrix
count = len(machine_truth_map) + 0.0
pos_count = 0
neg_count = 0

false_pos = []
false_neg = []
true_pos = []
true_neg = []

for req_id in machine_truth_map.keys():
    machine_is_empty = machine_truth_map[req_id]
    ground_truth_is_empty = False if req_id in ground_truth_map else True

    if ground_truth_is_empty:
        pos_count += 1
    else:
        neg_count += 1

    if (machine_is_empty and ground_truth_is_empty):
        true_pos.append(req_id)
    elif (not machine_is_empty and not ground_truth_is_empty):
        true_neg.append(req_id)
    elif (machine_is_empty and not ground_truth_is_empty):
        false_pos.append(req_id)
    elif (not machine_is_empty and ground_truth_is_empty):
        false_neg.append(req_id)
    else:
        raise Exception("wuit")

if (len(false_pos) > 0):
    print("\nFalse Positive request IDs ({}): {}".format(len(false_pos), " ".join(false_pos)))
if (len(false_neg) > 0):
    print("\nFalse Negative request IDs ({}): {}".format(len(false_neg), " ".join(false_neg)))

print("\n\n=== Confusion Matrix ({0:.4f}% of {1} Samples) ===".format(100 * (count - len(false_neg) - len(false_pos)) / count, int(count)))
print("                 |  Is Empty | Is Not Empty ")
print("___________________________________________________")
print("Predict Empty    |  {0:.4f}% | {1:.4f}% ".format(100 * len(true_pos) / pos_count, 100 * len(false_pos) / neg_count))
print("Predict Nonempty |  {0:.4f}% | {1:.4f}% ".format(100 * len(false_neg) / pos_count, 100 * len(true_neg) / neg_count))
print("===================================================")

print("\n\n=== Confusion Matrix ({0} pass of {1} Samples) ===".format(len(true_neg) + len(true_pos), int(count)))
print("                 |  Is Empty | Is Not Empty ")
print("___________________________________________________")
print("Predict Empty    |  {0}     | {1} ".format(len(true_pos), len(false_pos)))
print("Predict Nonempty |  {0}     | {1} ".format(len(false_neg), len(true_neg)))
print("===================================================")
