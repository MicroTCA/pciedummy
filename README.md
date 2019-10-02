# pciedummy
pcie device driver based on upciedev, used as template to crate new drivers

To crate the new driver just change all file names from devname* to your "device name"*, and make changes in Makefile.
For example, if You want to crate the diver for sis81600 board change names from devname* to sis8160*

In all files make Replace:
DEVNAME to SIS8160 
devname to sis8160
change in devname_fhc.h:
#define DEVNAME_VENDOR_ID 0x10EE	                /* XILINX vendor ID  change to board vendor ID*/
#define DEVNAME_DEVICE_ID 0x0088	                    /* change to  board device ID  */
to your board PCIe Vendor and Device IDs


to install the driver run "./sc" as root and run "depmod -a" as root
run modprobe sis8160
the driver will crate device file /dev/sis8160sX X-is a slot number where You have your device


compile, install and enjoy

in case of quetions maile to ludwig.petrosyan@desy.de
