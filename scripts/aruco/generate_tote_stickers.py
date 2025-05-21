import cv2
import numpy as np

IMAGE_BORDER_PX = 0
# NOTE: must change this for future facility generation
FACILITY_ID = 1

### GENERAL HELPERS ###
def mm_to_pixel(mm: float) -> int:
    return round(3.779 * mm)

def get_pixel_x_location(tag_type):
    # x distance from the leftmost point 
    # note that 10.5 is roughly marker size/2, shouldn't be hardcoded
    marker_x_locations_mm = [19.56, 94.56, 169.56, 244.56, 319.56 + 10.5, 1.3, 1.3]
    marker_order = ["cv_left", "cv_right", "tote_id", "facility_id", "human_readable", "cv_high", "cv_low"]
    print(f"Getting pixel x location for {tag_type}: at index {marker_order.index(tag_type)}")
    return mm_to_pixel(marker_x_locations_mm[marker_order.index(tag_type)])

def get_pixel_y_location(tag_type):
    # y distance from the topmost point
    # note that 10.5 is roughly marker size/2, shouldn't be hardcoded
    marker_y_locations_mm = [3.5, 3.5, 3.5, 3.5, 3.5 + 10.5, 20.43, 95.43] #prev 3.5 not 2
    marker_order = ["cv_left", "cv_right", "tote_id", "facility_id", "human_readable", "cv_high", "cv_low"]
    return mm_to_pixel(marker_y_locations_mm[marker_order.index(tag_type)])

def place_tag(image, tag_to_place, x_target, y_target):
    tag_to_place = cv2.cvtColor(tag_to_place, cv2.COLOR_GRAY2BGR)
    tag_height, tag_width, tag_third = tag_to_place.shape[:3]
    # target_roi = image[y_target + IMAGE_BORDER_PX:y_target + tag_height + IMAGE_BORDER_PX, x_target + IMAGE_BORDER_PX:x_target + tag_width + IMAGE_BORDER_PX, 2]
    print(f"tag height: {tag_height}, tag width: {tag_width}")
    print(f"tag to place: {tag_to_place.shape}")
    print(f"y_target: {y_target}, x_target: {x_target}")
    print(f"y_target + IMAGE_BORDER_PX: {y_target + IMAGE_BORDER_PX}, y_target + tag_height + IMAGE_BORDER_PX: {y_target + tag_height + IMAGE_BORDER_PX}")
    print(f"x_target + IMAGE_BORDER_PX: {x_target + IMAGE_BORDER_PX}, x_target + tag_width + IMAGE_BORDER_PX: {x_target + tag_width + IMAGE_BORDER_PX}")
    print(f"image shape: {image.shape}")

    if (y_target + IMAGE_BORDER_PX + tag_height <= image.shape[0] and
        x_target + IMAGE_BORDER_PX + tag_width <= image.shape[1]):
        image[y_target + IMAGE_BORDER_PX : y_target + tag_height + IMAGE_BORDER_PX,
            x_target + IMAGE_BORDER_PX : x_target + tag_width + IMAGE_BORDER_PX] = tag_to_place
    else:
        print("Tag won't fit in the image. Skipping or repositioning.")

    # image[y_target + IMAGE_BORDER_PX:y_target + tag_height + IMAGE_BORDER_PX, x_target + IMAGE_BORDER_PX:x_target + tag_width + IMAGE_BORDER_PX] = tag_to_place

    # Get the region of interest (ROI) on the background
    # y1, y2 = y_target, y_target + tag_to_place.shape[0]
    # x1, x2 = x_target, x_target + tag_to_place.shape[1]

    # # Simply copy the overlay into the ROI
    # image[y1:y2, x1:x2] = tag_to_place

def place_cv_tag(image, cv_idx, left_x_location, y_location, marker_size):
    dict = cv2.aruco.extendDictionary(8, 4)
    tag = cv2.aruco.generateImageMarker(dict, cv_idx, marker_size)
    place_tag(image, tag, left_x_location, y_location)

def generate_top_bottom_stickers(image, sticker_name, cv_left_x_location, cv_right_x_location, cv_left_y_location, cv_right_y_location, marker_size):
    # duplicate image for top and bottom
    # bottom_image = image.copy() # actually does it need to be duplicated or is overwritten sufficient since same location
    starter_idx = 3  if "vertical" in sticker_name else 1
    # generate correct CV ID for marker location
    idx = 0
    for cavity in ["top", "bottom"]:
        tag_idx = (idx*4) + starter_idx - 1
        print(f"Placing CV tag with idx {tag_idx}")
        place_cv_tag(image, tag_idx, cv_left_x_location, cv_left_y_location, marker_size)

        tag_idx+=1
        print(f"Placing CV tag with idx {tag_idx}")
        place_cv_tag(image, tag_idx, cv_right_x_location, cv_right_y_location, marker_size)
        idx+=1

        filename = f"tote_{sticker_name}_{cavity}.png"
        if image.shape[2] != 4:
            image_transparent = cv2.cvtColor(image, cv2.COLOR_BGR2BGRA)

            # Define the target BGR color
            target_color = [100, 100, 100]  # Example BGR
            mask = cv2.inRange(image[:, :, :3], np.array(target_color), np.array(target_color))

            # Set alpha to 0 where the mask is true
            image_transparent[mask > 0, 3] = 0
            # Set alpha to 0 (transparent) in a region
            # image_transparent[image_transparent==100] = 0
            print(f"Transparented it!")
            print(f"Writing image to {filename}")
            cv2.imwrite(filename, image_transparent)

### HORIZONTAL MARKERS ###
def place_tote_tag(image, tote_idx, x_location, y_location, marker_size):
    dict = cv2.aruco.extendDictionary(250, 5)
    tote_tag = cv2.aruco.generateImageMarker(dict, tote_idx, marker_size)
    place_tag(image, tote_tag, x_location, y_location)

def place_facility_tag(image, facility_idx, x_location, y_location, marker_size):
    dict = cv2.aruco.extendDictionary(999, 6)
    facility_tag = cv2.aruco.generateImageMarker(dict, facility_idx, marker_size)
    place_tag(image, facility_tag, x_location, y_location)

def place_readable_tag(image, tote_idx, readable_x_location, marker_y_location):
    font_scale = 0.8
    color = (34, 139, 34)
    thickness = 2
    position = (readable_x_location + IMAGE_BORDER_PX, marker_y_location + IMAGE_BORDER_PX)

    cv2.putText(image, str(tote_idx), position, cv2.FONT_HERSHEY_SIMPLEX, font_scale, color, thickness, cv2.LINE_AA)



def generate_horizontal_stickers(tote_idx):
    print(f"Generating horizontal stickers for tote {tote_idx}...")
    # generate horizontal sticker
    sticker_width = mm_to_pixel(360.13)
    sticker_height = mm_to_pixel(28)
    marker_size = mm_to_pixel(21)

    image = np.ones((IMAGE_BORDER_PX*2 + sticker_height, IMAGE_BORDER_PX*2 + sticker_width, 3), np.uint8) * 255
    # add line for where sticker border is
    cv2.rectangle(image, (IMAGE_BORDER_PX, IMAGE_BORDER_PX), (sticker_width + IMAGE_BORDER_PX, sticker_height + IMAGE_BORDER_PX), (0,0,0), 2)

    # add tote sticker
    tote_id_x_location = get_pixel_x_location("tote_id")
    tote_id_y_location = get_pixel_y_location("tote_id")
    place_tote_tag(image, tote_idx, tote_id_x_location, tote_id_y_location, marker_size)

    # add human readable
    readable_x_location = get_pixel_x_location("human_readable")
    readable_y_location = get_pixel_y_location("human_readable")
    place_readable_tag(image, tote_idx, readable_x_location, readable_y_location)

    # add facility sticker
    facility_id_x_location = get_pixel_x_location("facility_id")
    facility_id_y_location = get_pixel_y_location("facility_id")
    place_facility_tag(image, FACILITY_ID, facility_id_x_location, facility_id_y_location, marker_size)

    # add cv markers
    cv_left_x_location = get_pixel_x_location("cv_left")
    cv_right_x_location = get_pixel_x_location("cv_right")
    cv_left_y_location = get_pixel_y_location("cv_left")
    cv_right_y_location = get_pixel_y_location("cv_right")
    generate_top_bottom_stickers(image, f"{tote_idx}_horizontal", cv_left_x_location, cv_right_x_location, cv_left_y_location, cv_right_y_location, marker_size)
    
### VERTICAL MARKERS ###
# generate vertical stickers
def generate_vertical_stickers():
    print(f"Generating vertical stickers now")
    sticker_width = mm_to_pixel(22)
    sticker_height = mm_to_pixel(135.35)
    marker_size = mm_to_pixel(19.5)

    image = np.ones((IMAGE_BORDER_PX*2 + sticker_height, IMAGE_BORDER_PX*2 + sticker_width, 3), np.uint8) * 255
    

    # add line for where sticker border is
    # cv2.rectangle(image, (IMAGE_BORDER_PX, IMAGE_BORDER_PX), (sticker_width+IMAGE_BORDER_PX, sticker_height+IMAGE_BORDER_PX), (255,255,255), 1 )

    # round the right hand corners
    radius = mm_to_pixel(5)
    white = (255,255,255)
    removable = (100,100,100)
    image[sticker_height-radius:sticker_height, sticker_width-radius:sticker_width] = 100
    image[0:radius, sticker_width-radius:sticker_width] = 100
    cv2.rectangle(image, (IMAGE_BORDER_PX, IMAGE_BORDER_PX), (sticker_width+IMAGE_BORDER_PX-radius, sticker_height+IMAGE_BORDER_PX), white, -1 )
    cv2.rectangle(image, (IMAGE_BORDER_PX + sticker_width - radius, sticker_height + radius), (IMAGE_BORDER_PX + sticker_width, IMAGE_BORDER_PX + sticker_height - radius), white, 1)

    # top right and bottom right
    cv2.ellipse(image, (IMAGE_BORDER_PX + sticker_width - radius, IMAGE_BORDER_PX + radius), (radius, radius), 270, 0, 90, white, -1)
    cv2.ellipse(image, (IMAGE_BORDER_PX + sticker_width - radius, IMAGE_BORDER_PX + sticker_height - radius), (radius, radius), 0, 0, 90, white, -1)

    # add cv markers
    cv_high_x_location = get_pixel_x_location("cv_high")
    cv_low_x_location = get_pixel_x_location("cv_low")
    cv_high_y_location = get_pixel_y_location("cv_high")
    cv_low_y_location = get_pixel_y_location("cv_low")
    generate_top_bottom_stickers(image, "vertical", cv_high_x_location, cv_low_x_location, cv_high_y_location, cv_low_y_location, marker_size)


# entrypoint to generate all stickers
if __name__ == "__main__":
    max_tote_id = 40
    print(f"Generating stickers now for {max_tote_id} totes at facility {FACILITY_ID}!")
    generate_vertical_stickers()
    for tote_idx in range(1, max_tote_id+1):
        generate_horizontal_stickers(tote_idx)
