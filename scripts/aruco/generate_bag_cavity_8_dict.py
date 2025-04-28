# Run this to generate the 8-ID Aruco tags used on a bag cavity

import cv2

name = "bag_cavity"
max_id = 8
image_size = 500
border_size = 0
rows = 4
dict = cv2.aruco.extendDictionary(max_id, rows)

for i in range(max_id):
    img = cv2.aruco.generateImageMarker(dict, i, image_size - border_size)
    filename = name + "_" + str(i+1) + "_of_" + str(max_id) + "_" + str(rows) + "x" + str(rows) + ".png"
    img = cv2.copyMakeBorder(img, border_size, border_size, border_size, border_size, cv2.BORDER_CONSTANT, None, [255,255,255])

    # Write final result
    cv2.imwrite(filename, img)
    print("Wrote " + filename)
