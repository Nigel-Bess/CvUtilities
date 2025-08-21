# Translate CV results from the download scripts into a CVAT format that's easily viewable

from os import makedirs, listdir
from os.path import basename, exists, join
import shutil
import json
from xml.etree.ElementTree import Element, SubElement, tostring

def generate_fed_cvat_from_test(test_instance="test", verbose = False, img_width=1280, img_height=720):
    data = "data"
    test_dir = join(data, test_instance, "Tray_Camera")

    if not exists(test_dir):
        print("No CVAT annotations file / test output at " + test_dir)
        exit(1)

    cvat_root_dataset_dir = join(data, "cvat")
    cvat_dataset_dir = join(cvat_root_dataset_dir, "FED", test_instance)
    cvat_img_dir = join(cvat_dataset_dir, "images")
    makedirs(cvat_root_dataset_dir, exist_ok=True)
    makedirs(cvat_img_dir, exist_ok=True)
    makedirs(cvat_dataset_dir, exist_ok=True)

    # Post-process by-id dataset into CVAT-friendly dataset for quick eyeball review of accuracy
    if verbose:
        print("\033[92m 2. Post-processing results to CVAT format for coco annotators \033[0m")
    output_labels = []
    sorted_reqs = []
    for req_id in listdir(test_dir):
        sorted_reqs.append(req_id)
    sorted_reqs.sort() # this ensures id/name matching is consistent in CVAT
    for req_id in sorted_reqs:
        result_file = join(test_dir, req_id, "PreFrontEdgeDistance", "fed_out.json")
        if verbose:
            print(result_file)
        if exists(result_file):
            with open(result_file) as f:
                output_labels.append(json.load(f))
            shutil.copy(join(test_dir, req_id, "PreFrontEdgeDistance", "color_image.png"), join(cvat_img_dir, req_id + ".png"))
        elif verbose:
            print("\033[93m Warning: No fed_out.json labels found for req_id, skipping CVAT-ification: " + req_id + "\033[0m")
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
    task_name_el.set("name", "FED")
    labels_el = SubElement(task_el, "labels")
    label_el = SubElement(labels_el, "label")
    label_name_el = SubElement(label_el, "name")
    label_name_el.text = "front_edge"
    type_el = SubElement(label_el, "type")
    type_el.text = "points"

    cvat_count = 0
    for output_label in output_labels:
        image = SubElement(cvat_xml_annotations, "image")
        image.set("width", str(img_width))
        image.set("height", str(img_height))
        image.set("id", str(cvat_count))
        image.set("name", output_label["request_id"] + ".png")

        points = SubElement(image, "points")
        points.set("label", "front_edge")
        points.set("occluded", "0")
        points.set("points", 
            str(output_label["px_first_item_x"]) + "," + 
            str(output_label["px_first_item_y"]))
        points.set("z_order", "1")
        cvat_count += 1

    with open(join(cvat_dataset_dir, "annotations.xml"), "w") as f:
        f.write(tostring(cvat_xml_annotations, encoding="unicode").replace("><", ">\n<"))

if __name__ == '__main__':
    generate_fed_cvat_from_test(test_instance="test", verbose=False)
