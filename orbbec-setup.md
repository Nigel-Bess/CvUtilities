# Orbbec Camera Setup

This document describes how to setup the Orbbec depth camera (335le).

## Initial configuration / connect laptop

Unfortunately, client machines such as your laptop can only conenct to Orbbecs when on the same subnet mask (first 3 of 4 IP bytes must match the camera's), so you'll need to artifically set your laptop's IP address to be something like `192.168.1.11` by setting the IP according to your operating system.  The [official Orbbec website](https://www.orbbec.com/docs/gemini-335le-quick-start/) describes how you can do this.  With your IP staticlly configured, you can now download the Orbbec SDK v2 Viewer application by downloading the release for your machine and running it.  If all goes well, the Viewer will display the new camera as selectable and you can test anything Orbbec-specific.

If plugging into a Unifi switch, ensure both the camera and laptop ports use the Default VLAN.

## Production Configuration

If this is a production Orbbec setup, you'll need to use the Viewer access from the previous step to set a new IP address for the camera.  Consult a networking admin to determine what VLAN or IP range to use.  Once you're sure of the camera's IP, go into Viewer while connected to the camera and click through the tab buttons on the left until you see an IP section.  Unfold the section and turn DHCP to off and set the IP address.  Set the subnet mask to 255.255.255.0 and the gateway address to whatever the facility gateway might be (it should always end with .1). Set the config and power cycle the camera.  NOTE! Often this IP setting does not stick and the Viewer Set process will need to be repeated until it does stick.

Finally, set your static laptop IP to the next IP after the camera's and restart Viewer to ensure you can now stream from the camera's new IP address.

### Troubleshooting

If the camera cannot be seen in the Viewer, it's possible the camera is not factory-fresh and has had it's IP set to something else, in this case you'll need help from a network admin to determine the camera's IP address and repeat the step of setting your static IP, except your new static IP will need to match the first 3 bytes of the camera's still.
