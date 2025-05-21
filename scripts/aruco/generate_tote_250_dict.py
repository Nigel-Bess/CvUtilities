# Run this to generate the 250-ID Aruco tags used by Totes
# We start at 1 index not 0

import cv2

name = "tote"
ids_to_generate = 4
max_id = 250
rows = 5
border_size = 0
image_size = 500
dict = cv2.aruco.extendDictionary(max_id, rows)

for i in range(1, ids_to_generate+1):
    img = cv2.aruco.generateImageMarker(dict, i, image_size - border_size)
    filename = name + "_" + str(i+1) + "_of_" + str(max_id) + "_" + str(rows) + "x" + str(rows) + ".png"
    #img = cv2.copyMakeBorder(img, border_size, border_size, border_size, border_size, cv2.BORDER_CONSTANT, None, [255,255,255])

    # Write final result
    cv2.imwrite(filename, img)
    print("Wrote " + filename)
