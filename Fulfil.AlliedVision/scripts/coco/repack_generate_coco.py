# Translate CV results from the download scripts into a COCO format that's easily viewable in the
# "coco-annotator" project: https://github.com/jsbroks/coco-annotator/wiki/Getting-Started

# Run from Fulfil.AlliedVision: `python3 scripts/coco/repack_generate_coco.py`

from os import makedirs, listdir
from os.path import isdir, basename, exists, abspath
import shutil
import json
import time
from _coco_client import CocoApiClient

coco_annotator_username = "fulfil"
coco_annotator_password = "fulfil"
coco_client = CocoApiClient()

coco_client.login(coco_annotator_username, coco_annotator_password)
coco_client.destroy_undos() # These are just an annoyance

data = "data/"
test = data + "test/"

if not exists(data + "test"):
    print("No COCO annotations file / test output, see README.md's Testing + Debugging section to download Repack's dataset first")
    exit(1)

coco_dataset_dir = "/coco-annotator/datasets/cv-repack"
if not exists("/coco-annotator/datasets"):
    print("\033[93m Expected /coco-annotator sibling folder of Fulfil.ComputerVision doesn't exist, where to save coco-annotator cv-repack data?... \033[0m")
    print("...or maybe you want to create an empty Repack Dataset in coco-annotator first? ( See ../README.md's 'Testing + Debugging' section )")
    coco_dataset_dir = input("COCO dir: (default: " + abspath(coco_dataset_dir) + ")")

# Destroy everything for this dataset in coco-annotator and start fresh
print("\033[92m 1. Deleting and re-creating cv-repack COCO dataset \033[0m")
coco_client.delete_dataset_by_name("cv-repack")
print("cv-repack dataset deleted")
#coco_client.delete_category_by_name("NotEmpty")
#print("NotEmpty category deleted")
ds = coco_client.create_dataset("cv-repack")
ds_id = ds["id"]
print("cv-repack dataset recreated")

# Post-process by-id dataset into COCO-friendly dataset for quick eyeball review of accuracy
print("\033[92m 2. Post-processing results to COCO format for coco-annotator \033[0m")
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
with open("scripts/coco/_coco_template.json") as f:
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

time.sleep(1) # Wait for coco-annotator to catch up before uploading json
coco_client.upload_coco_file(open(data + "plm.json", "rb"), ds_id)
print(data + "plm.json uploaded to cv-repack COCO dataset")
coco_client.refresh_images(ds)
coco_client.upload_coco_file(open(data + "plm.json", "rb"), ds_id)

print(coco_dataset_dir + ": " + str(coco_count) + " images, " + str(coco_annotation_count) + " non-empty annotations")
print("\033[92m Success! View Repack PLM outputs at http://localhost:5000/#/dataset/" + str(ds_id) + "  \033[0m")
