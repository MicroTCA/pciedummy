/**
*Copyright 2016-  DESY (Deutsches Elektronen-Synchrotron, www.desy.de)
*
*This file is part of SIS8160 driver.
*
*SIS8160 is free software: you can redistribute it and/or modify
*it under the terms of the GNU General Public License as published by
*the Free Software Foundation, either version 3 of the License, or
*(at your option) any later version.
*
*SIS8160 is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*GNU General Public License for more details.
*
*You should have received a copy of the GNU General Public License
*along with SIS8160.  If not, see <http://www.gnu.org/licenses/>.
**/

/*
*	Author: Ludwig Petrosyan (Email: ludwig.petrosyan@desy.de)
*/

#ifndef _DEVNAME_FNC_H_
#define _DEVNAME_FNC_H_

#include "pciedev_io.h"
#include "pciedev_ufn.h"

#define DEVNAME "devname"	                                        /* name of device */

//#define DEVNAME_VENDOR_ID 0x10EE	                    /* XILINX vendor ID  change to board vendor ID*/
//#define DEVNAME_DEVICE_ID 0x0088	                    /* change to  board device ID  */

//use this to test driver on sis8160 board
#define DEVNAME_VENDOR_ID 0x1796	                    /* XSIS vendor ID*/
#define DEVNAME_DEVICE_ID 0x0028	                    /* sis8160 device ID  */

#define DEVNAME_SUBVENDOR_ID PCI_ANY_ID	
#define DEVNAME_SUBDEVICE_ID PCI_ANY_ID	

struct devname_dev {
    int                              brd_num;
    struct pciedev_dev *parent_dev;
};
typedef struct devname_dev devname_dev;

long devname_ioctl_dma(struct file *, unsigned int* , unsigned long* );

#endif /* _DEVNAME_FNC_H_ */
