# Translate CV results from the download scripts into a COCO format that's easily viewable in the
# "coco-annotator" project: https://github.com/jsbroks/coco-annotator/wiki/Getting-Started

# Run from Fulfil.AlliedVision: `python3 scripts/coco/repack_generate_coco.py`

from os import makedirs, listdir
from os.path import basename, exists, join
import shutil
import json

coco_annotator_username = "fulfil"
coco_annotator_password = "fulfil"

data = "Fulfil.AlliedVision/data/"
test = data + "test/"

if not exists(test):
    print("No COCO annotations file / test output, see README.md's Testing + Debugging section to download Repack's dataset first")
    exit(1)

coco_dataset_dir = join(data, "coco")
if not exists(coco_dataset_dir):
    print("Created AlliedVision annotation folder")
    makedirs(coco_dataset_dir)

# Post-process by-id dataset into COCO-friendly dataset for quick eyeball review of accuracy
print("\033[92m 2. Post-processing results to COCO format for coco annotators \033[0m")
coco_count = 0
coco_annotation_count = 0
for req_id in listdir(test):
    result_file = test + req_id + "/result.json"
    if exists(result_file):
        shutil.copy(result_file, coco_dataset_dir + "/" + req_id + ".json")
        shutil.copy(test + req_id + "/color_image.jpeg", coco_dataset_dir + "/" + req_id + ".jpeg")
    else:
        print("\033[93m Warning: No output label found for req_id, skipping COCO-ification: " + req_id + "\033[0m")
# Generate unified COCO file by scanning over all result files
coco_labels = {}
with open("Fulfil.AlliedVision/scripts/coco/_coco_template.json") as f:
    coco_labels = json.load(f)
for file in listdir(coco_dataset_dir):
    if file.endswith(".json"):
        req_id = basename(file).replace(".json", "")
        with open(coco_dataset_dir + "/" + file) as f:
            coco_count += 1
            labels = json.load(f)

            # If not a proper Repack raw label file, skip
            if not "isEmpty" in labels:
                continue

            # Always write the COCO image metadata
            coco_labels["images"].append({
                "id": coco_count,
                "path": "/datasets/cv-repack/" + req_id + ".jpeg",
                "width": 2592,
                "height": 1944,
                "file_name": req_id + ".jpeg",
                "metadata": {},
                "deleted": False
            })
            # Only write COCO labels if not empty / item(s) detected
            if labels["isEmpty"] == False:
                coco_annotation_count += 1
                coco_labels["annotations"].append({
                    "id": coco_annotation_count,
                    "image_id": coco_count,
                    "category_id": 1,
                    "area": 1165554,
                    "bbox": [668, 641, 1326, 879],
                    "segmentation": [
                        [
                        1993.6,
                        641.1,
                        1993.6,
                        1519.9,
                        668.3,
                        1519.9,
                        668.3,
                        641.1
                        ]
                    ],
                    "iscrowd": False,
                    "isbbox": True,
                    "color": "#9eaa1f",
                    "metadata": {}
                })
with open(data + "plm.json", "w") as f:
    json.dump(coco_labels, f)

print("Success! View Repack PLM outputs in data/coco")
