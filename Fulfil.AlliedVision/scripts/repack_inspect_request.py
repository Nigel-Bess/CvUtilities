# Run this with `python3 scripts/inspect_request.py "<My Request ID>"`

import sys
import json
import subprocess
req_id = sys.argv[1]

dir = "data/test/" + req_id + "/"

label_file = dir + "result.json"
with open(label_file) as f:
    j = json.load(f)
    print("\n\n")
    print(j)
    print("\n\n")

subprocess.run(["open", dir + "color_image.jpeg"])
subprocess.run(["open", dir + "canny_edges.jpeg"])
subprocess.run(["open", dir + "hull_image.jpeg"])
subprocess.run(["open", dir + "markers_selected_LFB-3.2.jpeg"])
subprocess.run(["open", dir + "region_of_interest.jpeg"])
subprocess.run(["open", dir + "rgb_contours.jpeg"]) 
