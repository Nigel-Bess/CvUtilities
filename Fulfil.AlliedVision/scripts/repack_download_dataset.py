# Assuming you have Gcloud SDK installed and user access to the factory-media folder in fulfil-web GCP project,
# run this to download all PLM prod Repack images

# Downloads all Repack data, formats it into COCO and allows for fast tool import + review.

from os import makedirs, listdir
from os.path import isdir, exists
import shutil
import json

data = 'data/'
raw = data + 'raw/'
by_id = data + 'by-id/'

# Step 1: Import GCS client and rsync to ./data from cloud bucket
# Messy, but use command line gcloud storage rsync since there's really no other great way
# to get fast rsync behavior, and it should work across all OSes anyway
if not exists(by_id):
    makedirs(by_id)
    print("\033[93m Assuming you have Gcloud SDK installed and authorized for factory-media bucket access... ( https://cloud.google.com/sdk/docs/install ) \033[0m")
    print("\033[92m 1. Download raw PLM Repack dataset from GCS by running: \033[0m")
    print("gcloud storage rsync -r 'gs://factory-media/plm/repack' 'Fulfil.AlliedVision/data/by-id'")
    print("then run this again")
    exit(1)
else:
    print("\033[93m Assuming your raw dataset is up-to-date, if not, re-run after executing: \033[0m")
    print("gcloud storage rsync -r 'gs://factory-media/plm/repack' 'Fulfil.AlliedVision/data/by-id'")
