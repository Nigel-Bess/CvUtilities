# Translate CV results from the download scripts into a CVAT format that's easily viewable

from os import makedirs, listdir
from os.path import basename, exists, join
import shutil
import json
from xml.etree.ElementTree import Element, SubElement, tostring

def append_first_seen_dir(base_dir):
    for first in listdir(base_dir):
        return join(base_dir, first)

def generate_bag_clips_cvat_from_test(test_instance="test", verbose = False, img_width=1280, img_height=720):
    data = "data"
    test_dir = join(data, test_instance, "bag_clips")

    if not exists(test_dir):
        print("No test output at " + test_dir)
        exit(1)

    cvat_root_dataset_dir = join(data, "cvat")
    cvat_dataset_dir = join(cvat_root_dataset_dir, "bag_clips", test_instance)
    cvat_img_dir = join(cvat_dataset_dir, "images")
    makedirs(cvat_root_dataset_dir, exist_ok=True)
    makedirs(cvat_img_dir, exist_ok=True)
    makedirs(cvat_dataset_dir, exist_ok=True)

    # Post-process by-id dataset into CVAT-friendly dataset for quick eyeball review of accuracy
    if verbose:
        print("\033[92m 2. Post-processing results to CVAT format for annotation \033[0m")
    output_labels = []
    sorted_reqs = []
    for req_id in listdir(test_dir):
        sorted_reqs.append(req_id)
    sorted_reqs.sort() # this ensures id/name matching is consistent in CVAT
    for req_id in sorted_reqs:
        test_req_dir = join(test_dir, req_id)
        prefix_folder = join(test_req_dir, "Floor_View_Image")
        main_folder = append_first_seen_dir(prefix_folder)
        labels_file = join(test_req_dir, "bag_clips.json")
        img_file = join(main_folder, "color_image.png")
        if verbose:
            print(labels_file)
        if exists(labels_file):
            with open(labels_file) as f:
                output_labels.append(json.load(f))
            shutil.copy(img_file, join(cvat_img_dir, req_id + ".png"))
        elif verbose:
            print("\033[93m Warning: No bag_clips.json labels found for req_id, skipping CVAT-ification: " + req_id + "\033[0m")
    # Generate unified CVAT file by scanning over seen output labels
    cvat_xml_annotations = Element("annotations")
    version_el = SubElement(cvat_xml_annotations, "version")
    version_el.text = "1.1"
    meta_el = SubElement(cvat_xml_annotations, "meta")
    task_el = SubElement(meta_el, "task")
    z_order_el = SubElement(task_el, "z_order")
    z_order_el.text = "True"
    mode_el = SubElement(task_el, "mode")
    mode_el.text = "annotation"
    task_name_el = SubElement(task_el, "name")
    task_name_el.set("name", "bag_clips")
    labels_el = SubElement(task_el, "labels")

    label_el = SubElement(labels_el, "label")
    type_el = SubElement(label_el, "type")
    type_el.text = "tag"
    label_name_el = SubElement(label_el, "name")
    label_name_el.text = "top_left_open"
    type_el = SubElement(label_el, "color")
    type_el.text = "#0d37fb"

    label_el = SubElement(labels_el, "label")
    type_el = SubElement(label_el, "type")
    type_el.text = "tag"
    label_name_el = SubElement(label_el, "name")
    label_name_el.text = "top_right_open"
    type_el = SubElement(label_el, "color")
    type_el.text = "#e366a1"

    label_el = SubElement(labels_el, "label")
    type_el = SubElement(label_el, "type")
    type_el.text = "tag"
    label_name_el = SubElement(label_el, "name")
    label_name_el.text = "top_left_open"
    type_el = SubElement(label_el, "color")
    type_el.text = "#e366a1"

    label_el = SubElement(labels_el, "label")
    type_el = SubElement(label_el, "type")
    type_el.text = "tag"
    label_name_el = SubElement(label_el, "name")
    label_name_el.text = "top_left_open"
    type_el = SubElement(label_el, "color")
    type_el.text = "#71a899"

    cvat_count = 0
    for output_label in output_labels:
        image = SubElement(cvat_xml_annotations, "image")
        image.set("width", str(img_width))
        image.set("height", str(img_height))
        image.set("id", str(cvat_count))
        image.set("name", output_label["requestId"] + ".png")

        # Flag as open if there's any indication of it at all for easy metrics' sake
        if (not output_label["topLeftIsClosed"] or not (output_label["topLeftStatus"] == 0) ):
            tag = SubElement(image, "tag")
            tag.set("label", "top_left_open")
            tag.set("source", "manual")
        if (not output_label["topRightIsClosed"] or not (output_label["topRightStatus"] == 0) ):
            tag = SubElement(image, "tag")
            tag.set("label", "top_right_open")
            tag.set("source", "manual")
        if (not output_label["bottomLeftIsClosed"] or not (output_label["bottomLeftStatus"] == 0) ):
            tag = SubElement(image, "tag")
            tag.set("label", "bottom_left_open")
            tag.set("source", "manual")
        if (not output_label["bottomRightIsClosed"] or not (output_label["bottomRightStatus"] == 0) ):
            tag = SubElement(image, "tag")
            tag.set("label", "bottom_right_open")
            tag.set("source", "manual")
        cvat_count += 1

    with open(join(cvat_dataset_dir, "annotations.xml"), "w") as f:
        f.write(tostring(cvat_xml_annotations, encoding="unicode").replace("><", ">\n<"))

if __name__ == '__main__':
    generate_bag_clips_cvat_from_test(test_instance="test", verbose=True)