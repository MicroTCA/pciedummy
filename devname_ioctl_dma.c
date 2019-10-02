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

/*
*	Author: Ludwig Petrosyan (Email: ludwig.petrosyan@desy.de)
*/


#include <linux/types.h>
#include <linux/timer.h>
//#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/delay.h>

#include "devname_fnc.h"
#include "devname_defs.h"

long     devname_ioctl_dma(struct file *filp, unsigned int *cmd_p, unsigned long *arg_p)
{
	unsigned int     cmd;
	unsigned long  arg;
	 pid_t                 cur_proc = 0;
	int                      minor    = 0;
	int                      d_num    = 0;
	int                      retval   = 0;
	int                      i = 0;
	long                   timeDMAwait;
	ulong                 value;
	u_int	           tmp_dma_size;
	u_int	           tmp_dma_trns_size;
	u_int	           tmp_dma_offset;
	void*                  pWriteBuf          = 0;
	void*                  address;
	int                      tmp_order          = 0;
	unsigned long  length             = 0;
	dma_addr_t      pTmpDmaHandle;
	u32                    dma_sys_addr ;
	int                      tmp_source_address = 0;
	u_int                  tmp_offset;
	u_int                  tmp_data;
	u_int                  tmp_data1;
	u_int                  tmp_cmd;
	u_int                  tmp_reserved;
	u_int                  tmp_num;
	u_int                  tmp_step;
	u32                    tmp_data_32;

	struct pci_dev*          pdev;
	struct pciedev_dev*  dev ;
	struct devname_dev*  devnamedev ;
    
	devname_reg              reg_data;
	device_ioctrl_dma   dma_data;
	device_ioctrl_data   data;
	device_ioctrl_time   time_data;
	device_ioctrl_data_buf data_buf;
	int                              reg_size;
	int                              io_size;
	int                              io_buf_size;
	int                              time_size;
	int                              io_dma_size;
	int                             dma_done_count;
	
	
	//PEER2PEER STAFF
	u_int    p2p_slot;
	u_int    p2p_bar;
	u_int    p2p_offset;
	u_int    p2p_phs_address;
	u_int    p2p_phs_end;
	u_int    p2p_phs_flag;
	device_phys_address  p2p_phaddress;

	cmd                            = *cmd_p;
	arg                              = *arg_p;
	reg_size                      = sizeof(devname_reg);
	io_size                         = sizeof(device_ioctrl_data);
	io_buf_size                  = sizeof(device_ioctrl_data_buf);
	time_size	= sizeof(device_ioctrl_time);
	io_dma_size = sizeof(device_ioctrl_dma);
    
	dev                 = filp->private_data;
	devnamedev    = (devname_dev   *)dev->dev_str;
	pdev               = (dev->pciedev_pci_dev);
	minor             = dev->dev_minor;
	d_num           = dev->dev_num;	
	cur_proc       = current->group_leader->pid;

	if(!dev->dev_sts){
		printk(KERN_ALERT  "DEVNAME_IOCTL_DMA: NO DEVICE %d\n", dev->dev_num);
		retval = -EFAULT;
		return retval;
	}

	address = pciedev_get_baraddress(BAR0, dev);
	if(!address){
		printk(KERN_ALERT  "DEVNAME_IOCTL_DMA: NO MEMORY\n");
		retval = -EFAULT;
		return retval;
	}
    

    
	if (mutex_lock_interruptible(&dev->dev_mut)){
		printk(KERN_ALERT  "DEVNAME_IOCTL_DMA: NO MUTEX\n");
		return -ERESTARTSYS;
	}
    
	switch (cmd) {
		case DEVNAME_PHYSICAL_SLOT:
			retval = 0;
			if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			tmp_offset   = data.offset;
			tmp_data     = data.data;
			tmp_cmd      = data.cmd;
			tmp_reserved = data.reserved;
			data.data    = dev->slot_num;
			data.cmd     = DEVNAME_PHYSICAL_SLOT;
			if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			break;
		case DEVNAME_DRIVER_VERSION:
			data.data   =   dev->parent_dev->PCIEDEV_DRV_VER_MAJ;
			data.offset =  dev->parent_dev->PCIEDEV_DRV_VER_MIN;
			if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			break;
		case DEVNAME_FIRMWARE_VERSION:
			tmp_data_32       = ioread32(address + 0);
			smp_rmb();
			data.data = tmp_data_32;
			if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			break;
		case DEVNAME_REG_READ:
			retval = 0;
			if (copy_from_user(&reg_data, (devname_reg*)arg, (size_t)reg_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			tmp_offset     = reg_data.offset*4;
			tmp_data       = reg_data.data;
			if (tmp_offset  > dev->rw_off[0]) {
				printk (KERN_ALERT "DEVNAME_REG_READ: OUT OF MEM\n");
				mutex_unlock(&dev->dev_mut);
				return EFAULT;
			}
			tmp_data_32       = ioread32(address + tmp_offset);
			smp_rmb();
			udelay(2);
			reg_data.data = tmp_data_32;
			if (copy_to_user((devname_reg*)arg, &reg_data, (size_t)reg_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			break;
		case DEVNAME_REG_WRITE:
			 retval = 0;
			if (copy_from_user(&reg_data, (devname_reg*)arg, (size_t)reg_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			tmp_offset     = reg_data.offset*4;
			tmp_data       = reg_data.data;
			if (tmp_offset >dev->rw_off[0]) {
				printk (KERN_ALERT "DEVNAME_REG_WRITE: OUT OF MEM\n");
				mutex_unlock(&dev->dev_mut);
				return EFAULT;
			}
			tmp_data_32 = reg_data.data &  0xFFFFFFFF;
			iowrite32(tmp_data_32, ((void*)(address + tmp_offset)));
			smp_wmb();
			udelay(2);
			if (copy_to_user((devname_reg*)arg, &reg_data, (size_t)reg_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			break;
		case PCIEDEV_GET_DMA_TIME:
		case DEVNAME_GET_DMA_TIME:
			retval = 0;
			if (copy_from_user(&time_data, (device_ioctrl_time*)arg, (size_t)time_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			// do sime job
			if (copy_to_user((device_ioctrl_time*)arg, &time_data, (size_t)time_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				return retval;
			}
			break;
		 case PCIEDEV_READ_DMA:
		 case DEVNAME_READ_DMA:
			retval = 0;
			if (copy_from_user(&dma_data, (device_ioctrl_dma*)arg, (size_t)io_dma_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				printk (KERN_ALERT "DEVNAME_READ_DMA: COULD NOT COPY FROM USER\n");
				return retval;
			}
			tmp_dma_size          = dma_data.dma_size;
			tmp_dma_offset        = dma_data.dma_offset;
			 if(tmp_dma_size <= 0){
				 printk (KERN_ALERT "DEVNAME_READ_DMA: SIZE 0 tmp_dma_size %d\n", tmp_dma_size);
				 mutex_unlock(&dev->dev_mut);
				 return EFAULT;
			}

			//do some job
			
			udelay(5);
			break;
		case PCIEDEV_WRITE_DMA:
		case DEVNAME_WRITE_DMA:
			//printk (KERN_ALERT "DEVNAME_READ_DMA: START DMA write\n");
			retval = 0;
			if (copy_from_user(&dma_data, (device_ioctrl_dma*)arg, (size_t)io_dma_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				printk (KERN_ALERT "DEVNAME_WRITE_DMA: COULD NOT COPY FROM USER\n");
				return retval;
			}
			tmp_dma_size           = dma_data.dma_size;
			tmp_dma_offset        = dma_data.dma_offset;
			if(tmp_dma_size <= 0){
				 printk (KERN_ALERT "DEVNAME_WRITE_DMA: tmp_dma_size %d\n", tmp_dma_size);
				 mutex_unlock(&dev->dev_mut);
				 return EFAULT;
			}
			
			//do some jobs
			
			udelay(2);
			break;
			
		case PCIEDEV_WRITE_DMA_P2P:
		case DEVNAME_WRITE_DMA_2PEER:
			retval = 0;
			if (copy_from_user(&dma_data, (device_ioctrl_dma*)arg, (size_t)io_dma_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				printk (KERN_ALERT "DEVNAME_WRITE_DMA_2PEER: COULD NOT COPY FROM USER\n");
				return retval;
			}
			tmp_dma_size           = dma_data.dma_size;
			tmp_dma_offset        = dma_data.dma_offset;
			tmp_cmd                    = dma_data.dma_cmd;
			p2p_slot                      = (dma_data.dma_reserved1 >> 16) & 0xFFFF; 
			p2p_bar                       = dma_data.dma_reserved1 & 0xFFFF;
			p2p_offset                   = dma_data.dma_reserved2;
           
			 if(tmp_dma_size <= 0){
				 printk (KERN_ALERT "DEVNAME_WRITE_DMA_2PEER: SIZE 0 tmp_dma_size %d\n", tmp_dma_size);
				 mutex_unlock(&dev->dev_mut);
				 return EFAULT;
			}
            
			// do some jobs
			
			udelay(5);
			break;
		default:
			mutex_unlock(&dev->dev_mut);
		return -ENOTTY;
		break;
	}
	mutex_unlock(&dev->dev_mut);

	return retval;
}
