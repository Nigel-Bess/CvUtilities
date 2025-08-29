# Final step after exporting the whole bag_clips CVAT dataset (format: CVAT 1.1) to Fulfil.Dispense/metrics/ground-truth/pioneer.xml

# Detected edge's Y can only be off by this percentage of the image height
# 7% here comes from about 50 out of 720 image height pixels
MAX_ALLOWED_Y_ERROR = 0.07

# Summarizes the following metrics:

# - True positive count of edge detections that are accurate to within MAX_ALLOWED_Y_ERROR
# - Count + Avg Y error for true detections that are slightly high
# - Count + Avg Y error for true detections that are slightly low
# - False positive count (non-negative predicated point should have been -1)
# - False negative count (predicated -1 when there was a true valid edge point)
# - Potential overdispenses (# of detections that were far too high)
# - Potential underdispense (# of detections that were far too low)

import os
from xml.etree.ElementTree import fromstring
import matplotlib.pyplot as plt
import json

def parse_cvat_annotations_xml(annotations_file):
    with open(annotations_file) as f:
        return fromstring(f.read())

def label_to_clip_index(label):
    if label == "top_left_open":
        return 0
    if label == "top_right_open":
        return 1
    if label == "bottom_left_open":
        return 2
    if label == "bottom_right_open":
        return 3
    raise Exception("huh?")

def xml_img_to_clip_labels(xml_img):
    tags = []
    for idx,tag_el in enumerate(xml_img.findall('.//tag')):
        tags.append(tag_el.attrib["label"])
    tags.sort()
    return tags

def record_comparison(ground_truth_img, out_img, correct_counts, false_pos, false_neg, low_confidence):
    name = out_img.attrib["name"]
    true_open_labels = xml_img_to_clip_labels(ground_truth_img)
    machine_open_labels = xml_img_to_clip_labels(out_img)

    error_clips = []
    if "68769500424b9cf190e1dd3a" in name:
        print(true_open_labels)
        print(machine_open_labels)
    for true_label in true_open_labels:
        if not (true_label in machine_open_labels):
            false_neg[label_to_clip_index(true_label)].append(name)
            error_clips.append(label_to_clip_index(true_label))
    for machine_label in machine_open_labels:
        if not (machine_label in true_open_labels):
            false_pos[label_to_clip_index(machine_label)].append(name)
            error_clips.append(label_to_clip_index(machine_label))
    for i in range(4):
        if i not in error_clips:
            correct_counts[i] += 1

def eval_output(output_labels_file, truth_labels_file):
    # Throw if either files is missing
    if not os.path.exists(output_labels_file):
        raise ValueError("Machine test labels file not found: {}".format(output_labels_file))
    if not os.path.exists(truth_labels_file):
        raise ValueError("Ground truth labels file not found: {}".format(truth_labels_file))
    count = 0
    # Build confusion matrix
    # - Correct count of edge detections that are accurate to within MAX_ALLOWED_Y_ERROR
    correct_counts = [0, 0, 0, 0]
    # - False positive count (non-negative predicated point should have been -1)
    false_pos = [[], [], [], []]
    # - False negative count (predicated -1 when there was a true valid edge point)
    false_neg = [[], [], [], []]
    # Reqs with too low of confidence to answer
    low_confidence = [[], [], [], []]

    # Parse both files to JSON
    ground_truth_xml = parse_cvat_annotations_xml(truth_labels_file)
    output_labels_xml = parse_cvat_annotations_xml(output_labels_file)
    output_count = 0
    match_missing = False
    for idx,img_el in enumerate(ground_truth_xml.findall('.//image')):
        count += 1
        img_name = os.path.basename(img_el.attrib["name"])
        match_found = False
        for idx,out_img_el in enumerate(output_labels_xml.findall('.//image')):
            if (img_name == out_img_el.attrib["name"]):
                match_found = True
                output_count += 1
                record_comparison(img_el, out_img_el, correct_counts, false_pos, false_neg, low_confidence)
                break
        match_missing = match_missing or not match_found
        if not match_found:
            print("Could not find machine output label for ground truth sample " + img_name)
            #raise ValueError("Fix the missing output labels above! Found {} / {}".format(output_count, count))

    if match_missing:
        raise ValueError("Fix the missing output labels above! Found {} / {}".format(output_count, count))

    return (count, correct_counts, false_pos, false_neg, low_confidence)

if __name__ == '__main__':
    ground_truth_file = os.environ['GROUND_TRUTH_FILE']
    print(ground_truth_file)
    print("data/cvat/bag_clips/test/annotations.xml")
    count, correct_counts, false_pos, false_neg, low_confidence = \
        eval_output(output_labels_file="data/cvat/bag_clips/test/annotations.xml", truth_labels_file=ground_truth_file)

    print("\n========= Bag clip results ===========")
    for i in range(4):
        print("\n\n=== Clip " + str(i) + " ===")
        print("Accuracy: {0} / {1} = {2:.4f}%".format(correct_counts[i], count, ((correct_counts[i] * 100.0) / count) ))
        print("False Positives (Falsy open): {}".format(false_pos[i]))
        print("False Negatives (Falsy closed): {}".format(false_neg[i]))
        #print("Low Confidence: {}".format(low_confidence[i]))
