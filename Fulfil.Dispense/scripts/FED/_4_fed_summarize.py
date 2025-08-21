# Final step after exporting the whole FED CVAT dataset (format: CVAT 1.1) to Fulfil.Dispense/metrics/ground-truth/pioneer.xml

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

height = 0

def parse_cvat_annotations_xml(annotations_file):
    with open(annotations_file) as f:
        return fromstring(f.read())

def record_comparison(ground_truth_img, out_img, out_vs_true_points, correct_count, correct_y_highs, correct_y_lows, false_pos, false_neg, overdispenses, underdispenses):
    out_img_point = ground_truth_img.find("points").attrib["points"].split(",")
    true_point = out_img.find("points").attrib["points"].split(",")
    out_vs_true_points.append([out_img_point, true_point])
    true_y = float(out_img_point[1])
    predicted_y = float(true_point[1])
    global height
    height = float(ground_truth_img.attrib["height"])
    percent_error = abs(true_y - predicted_y) / height

    name = out_img.attrib["name"]

    if percent_error <= MAX_ALLOWED_Y_ERROR:
        #correct_count += 1
        if true_y > predicted_y:
            correct_y_highs.append(percent_error)
        else:
            correct_y_lows.append(percent_error)
        return 1
    else:
        if (name == "68126337ed50ac1baa58bb87.png"):
                print("68126337ed50ac1baa58bb87")
                print("pred: " + str(predicted_y))
                print("true: " + str(true_y))
                print("err: " + str(percent_error))
        if true_y > 2 and predicted_y < 2:
            false_neg.append(name)
        elif true_y < 2 and predicted_y > 2:
            false_pos.append(name)
        else:
            if predicted_y > true_y:
                underdispenses.append([name, percent_error])
            else:
                overdispenses.append([name, percent_error])
    return 0

def eval_output(output_labels_file="metrics/FED-output-labels.xml", \
                truth_labels_file = "metrics/july2025/FED-ground-truth.xml"):
    # Throw if either files is missing
    if not os.path.exists(output_labels_file):
        raise ValueError("Machine test labels file not found: {}".format(output_labels_file))
    if not os.path.exists(truth_labels_file):
        raise ValueError("Ground truth labels file not found: {}".format(truth_labels_file))
    out_vs_true_points = [] # Tuples of machine vs ground points
    count = 0
    # Build confusion matrix
    # - Correct count of edge detections that are accurate to within MAX_ALLOWED_Y_ERROR
    correct_count = 0
    # - Count + Avg Y error for true detections that are slightly high
    correct_y_highs = []
    # - Count + Avg Y error for true detections that are slightly low
    correct_y_lows = []
    # - False positive count (non-negative predicated point should have been -1)
    false_pos = []
    # - False negative count (predicated -1 when there was a true valid edge point)
    false_neg = []
    # - Potential overdispenses (# of detections that were far too high)
    overdispenses = []
    # - Potential underdispense (# of detections that were far too low)
    underdispenses = []

    # Parse both files to JSON
    ground_truth_xml = parse_cvat_annotations_xml(truth_labels_file)
    output_labels_xml = parse_cvat_annotations_xml(output_labels_file)
    output_count = 0
    match_missing = False
    for idx,img_el in enumerate(ground_truth_xml.findall('.//image')):
        img_name = os.path.basename(img_el.attrib["name"])
        count += 1
        match_found = False
        for idx,out_img_el in enumerate(output_labels_xml.findall('.//image')):
            if (img_name == out_img_el.attrib["name"]):
                match_found = True
                output_count += 1
                correct_count += record_comparison(img_el, out_img_el, out_vs_true_points, correct_count, correct_y_highs, correct_y_lows, false_pos, false_neg, overdispenses, underdispenses)
                break
        match_missing = match_missing or not match_found
        if not match_found:
            print("Could not find machine output label for ground truth sample " + img_name)
            raise ValueError("Fix the missing output labels above! Found {} / {}".format(output_count, count))

    if match_missing:
        raise ValueError("Fix the missing output labels above! Found {} / {}".format(output_count, count))
    
    return (count, correct_count, correct_y_highs, correct_y_lows, false_pos, false_neg, overdispenses, underdispenses, out_vs_true_points)

def score_results(count, correct_y_highs, correct_y_lows, overdispenses, underdispenses, false_neg):
    # Now count through all the observed cases and adjust the score accordingly
    # Roughly +1 point for every correct count but tiny penalty for minor % off center
    #score = correct_count
    score = 0.0
    # Generally +1 for correct-ish answers
    for correct_high_y in correct_y_highs:
        score += (1.0 - (correct_high_y*10))  # A bit bigger penalty for tiny bit high
    for correct_low_y in correct_y_lows:
        score += (1.0 - (correct_low_y/10.0)) # Very tiny penalty for tiny bit low
    # overdispenses are very bad
    score += -(10.0 * len(overdispenses))
    # false positives also pretty bad
    score += -(2.0 * len(false_neg))
    # Points that are faaaaar too low dispense very slowly, penalize
    for under in underdispenses:
        score += -(under[1] * 2.0)

    # Normalize score by count so it's relatively consistent across different eval set sizes
    normalized = score / count
    return normalized

def show_histo(out_vs_true_points):
    global height
    fig, ax = plt.subplots()
    x = [float(tup[0][0]) - float(tup[1][0]) for tup in out_vs_true_points]
    y = [min(9.9, (100 * ((float(tup[0][1]) - float(tup[1][1])) / height))) for tup in out_vs_true_points]
    ax.hist2d(x, y, bins=[1, 100])
    ax.set(xlim=(-10, 10), ylim=(-10, 10))
    fig.savefig('data/cvat/FED/vis.png')

if __name__ == '__main__':
    ground_truth_file = os.environ['GROUND_TRUTH_FILE']
    print(ground_truth_file)
    print("data/cvat/FED/test/annotations.xml")
    count, correct_count, correct_y_highs, correct_y_lows, false_pos, false_neg, overdispenses, underdispenses, out_vs_true_points = \
        eval_output(output_labels_file="data/cvat/FED/test/annotations.xml", truth_labels_file=ground_truth_file)
    offset_high_avg = (float(sum(correct_y_highs)) / len(correct_y_highs)) * 100.0
    offset_low_avg = (float(sum(correct_y_lows)) / len(correct_y_lows)) * 100.0
    print("\nPotential over-dispense request IDs: " + ", ".join([o[0] for o in overdispenses]) + "\n")
    print("\nUnder-dispense request IDs: " + ", ".join([u[0] for u in underdispenses]) + "\n")
    print("\nEmpty lanes with detections: " + ", ".join(false_pos) + "\n")
    print("\nValid lanes missing detections: " + ", ".join(false_neg) + "\n")

    # Print final summary!
    print("\n\nOverall FED accuracy: {0} / {1} = {2:.4f}%".format(correct_count, count, ((correct_count * 100.0) / count) ))
    print("{0} valid detections that were above ground truth averaged {1:.4f}% of image height".format(len(correct_y_highs), offset_high_avg))
    print("{0} valid detections that were beneath ground truth averaged {1:.4f}% of image height".format(len(correct_y_lows), offset_low_avg))
    print("{0} detections seen when there should not have been a prediction".format(false_pos))
    print("{0} empty detections where there was a valid front edge".format(len(false_neg)))
    print("{0} detected edges were far too above ground truth, potential over-dispense!".format(len(overdispenses)))
    print("{0} detected edges were far too beneath ground truth".format(len(underdispenses)))

    score = score_results(count, correct_y_highs, correct_y_lows, overdispenses, underdispenses, false_neg)
    print("\nOverall score (one is perfect): " + str(score))

    to_see = input("What do you want to view? (graph, underdispenses, overdispenses)")
    print(to_see)
    to_see = "over"
    if to_see == "graph":
        # Generate histogram img
        show_histo(out_vs_true_points)
    elif to_see == "under":
        for under in underdispenses:
            os.open("data/test/Tray_Camera/" + under[0].split(".")[0] + "/PreFrontEdgeDistance/color_image.png", os.O_RDWR, 0o666)
    elif to_see == "over":
        for over in overdispenses:
            os.open("data/test/Tray_Camera/" + over[0].split(".")[0] + "/PreFrontEdgeDistance/color_image.png", os.O_RDWR, 0o666)

