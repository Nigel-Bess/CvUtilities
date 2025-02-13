from vmbpy import *
import sys
import signal
import time

killed = False
# Sigterm handler that sets killed = True
def sigterm_handler(signal, frame):
    print('Killed')
    global killed
    killed = True

# Register sigterm handler
signal.signal(signal.SIGTERM, sigterm_handler)
signal.signal(signal.SIGINT, sigterm_handler)

def print_camera(cam: Camera):
    print('/// Camera Name   : {}'.format(cam.get_name()))
    print('/// Model Name    : {}'.format(cam.get_model()))
    print('/// Camera ID     : {}'.format(cam.get_id()))
    print('/// Serial Number : {}'.format(cam.get_serial()))
    print('/// Interface ID  : {}\n'.format(cam.get_interface_id()))

def setup_camera(cam):
    with cam:
        # Try to adjust GeV packet size. This Feature is only available for GigE - Cameras.
        try:
            stream = cam.get_streams()[0]
            stream.GVSPAdjustPacketSize.run()
            while not stream.GVSPAdjustPacketSize.is_done():
                pass
            print("Agreed on packet size of " + str(stream.get_feature_by_name("GVSPPacketSize")))
        except (AttributeError, VmbFeatureError) as e:
            print("Error finding GVSPAdjustPacketSize ")
            print(e)
            exit(1)

last_frame_time = int(time.time() * 1000)
frame_received = False
def frame_handler(cam, stream, frame):
    global last_frame_time
    global frame_received
    now = int(time.time() * 1000)
    print('{} acquired {} at {}'.format(cam, frame.get_status(), now))
    print('ms since last frame {}', now - last_frame_time)
    last_frame_time = int(time.time() * 1000)
    frame_received = True
    # cam.queue_frame(frame)

def main(cam_index):
    global frame_received
    print("Streaming cam at index " + str(cam_index))
    with VmbSystem.get_instance() as vmb:
        cams = vmb.get_all_cameras()
        print('Cameras found: {}'.format(len(cams)))
        cam = cams[cam_index]
        with cam:
            setup_camera(cam)
            while not killed:
                try:
                    # Start Streaming with a custom a buffer of 10 Frames (defaults to 5)
                    print(f"Stream starting at: {int(time.time() * 1000)}")
                    frame_received = False
                    cam.start_streaming(handler=frame_handler,
                                    buffer_count=50,
                                    allocation_mode=AllocationMode.AnnounceFrame)
                    while not frame_received and not killed:
                        time.sleep(0.01)
                except Exception as e:
                    print(e)
                finally:
                    cam.stop_streaming()

if __name__ == '__main__':
    # Read in first cli arg as the cam_index starting from zero
    cam_index = int(sys.argv[1])
    main(cam_index)
