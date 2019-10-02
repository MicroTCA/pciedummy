/* 
 * File:   devname_defs.h
 * Author: petros
 *
 * Created on January 12, 2011, 5:20 PM
 */

/**
*Copyright 2016-  DESY (Deutsches Elektronen-Synchrotron, www.desy.de)
*
*This file is part of DEVNAME driver.
*
*DEVNAME is free software: you can redistribute it and/or modify
*it under the terms of the GNU General Public License as published by
*the Free Software Foundation, either version 3 of the License, or
*(at your option) any later version.
*
*DEVNAME is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*GNU General Public License for more details.
*
*You should have received a copy of the GNU General Public License
*along with DEVNAME.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef	DEVNAME_DEFS_H
#define	DEVNAME_DEFS_H

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include "pciedev_io.h"

struct device_ioctrl_data_buf  {
        u_int    offset;
        u_int    cmd;
        u_int    num;
        u_int    step;
        u_int    data[64];
};
typedef struct device_ioctrl_data_buf device_ioctrl_data_buf;

typedef struct t_devnamereg{
	u_int offset; /* offset from bar0 */
	u_int data;   /* data which will be read / written */
}devname_reg;

/* Use 'o' as magic number */

#define DEVNAME_IOC           			'0'
#define DEVNAME_PHYSICAL_SLOT       _IOWR(DEVNAME_IOC, 20, int)
#define DEVNAME_REG_READ            _IOWR(DEVNAME_IOC, 21, int)
#define DEVNAME_REG_WRITE           _IOWR(DEVNAME_IOC, 22, int)
#define DEVNAME_GET_DMA_TIME 	    _IOWR(DEVNAME_IOC, 23, int)
#define DEVNAME_DRIVER_VERSION      _IOWR(DEVNAME_IOC, 24, int)
#define DEVNAME_FIRMWARE_VERSION    _IOWR(DEVNAME_IOC, 25, int)
#define DEVNAME_READ_DMA            _IOWR(DEVNAME_IOC, 26, int)
#define DEVNAME_WRITE_DMA           _IOWR(DEVNAME_IOC, 27, int)
#define DEVNAME_WRITE_DMA_2PEER         _IOWR(DEVNAME_IOC, 28, int)

#define DEVNAME_IOC_MAXNR           28
#define DEVNAME_IOC_MINNR           20

#endif	/* DEVNAME_DEFS_H */

