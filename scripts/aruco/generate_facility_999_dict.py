# Run this to generate the 8-ID Aruco tags used by LFRs

import numpy as np
import cv2

name = "facility"
# Facilities use 999 instead of 1000 so they are distinct from Tote markers
max_id = 999
rows = 4
border_size = 40
image_size = 500
dict = cv2.aruco.extendDictionary(max_id, rows)

for i in range(max_id):
    img = cv2.aruco.generateImageMarker(dict, i, image_size - border_size)
    filename = "scripts/aruco/images/" + name + "_" + str(i+1) + "_of_" + str(max_id) + "_" + str(rows) + "x" + str(rows) + ".png"
    img = cv2.copyMakeBorder(img, border_size, border_size, border_size, border_size, cv2.BORDER_CONSTANT, None, [255,255,255])

    # Write final result
    cv2.imwrite(filename, img)
    print("Wrote " + filename)