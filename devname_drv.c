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

#include <linux/module.h>
#include <linux/fs.h>	
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/signal.h>

#include "devname_fnc.h"
#include "devname_defs.h"

MODULE_AUTHOR("Ludwig Petrosyan");
MODULE_DESCRIPTION("DESY AMC-PCIE board example driver");
MODULE_VERSION("1.1.0");
MODULE_LICENSE("Dual BSD/GPL");

pciedev_cdev     *devname_cdev_m = 0;
devname_dev     *devname_dev_p[PCIEDEV_NR_DEVS];
devname_dev     *devname_dev_pp;

static int        devname_open( struct inode *inode, struct file *filp );
static int        devname_release(struct inode *inode, struct file *filp);
static ssize_t devname_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t devname_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static long     devname_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int        devname_remap_mmap(struct file *filp, struct vm_area_struct *vma);

struct file_operations devname_fops = {
	.owner                  =  THIS_MODULE,
	.read                     =  devname_read,
	.write                    =  devname_write,
	.unlocked_ioctl    =  devname_ioctl,
	.open                    =  devname_open,
	.release                =  devname_release,
	.mmap                 = devname_remap_mmap,
};

static struct pci_device_id devname_ids[] = {
    { PCI_DEVICE(DEVNAME_VENDOR_ID, DEVNAME_DEVICE_ID)},
    { 0, }
};
MODULE_DEVICE_TABLE(pci, devname_ids);

/*
 * The top-half interrupt handler.
 */
#if LINUX_VERSION_CODE < 0x20613 // irq_handler_t has changed in 2.6.19
static irqreturn_t devname_interrupt(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t devname_interrupt(int irq, void *dev_id)
#endif
{
	uint32_t intreg = 0;
	void*                   address;

	struct pciedev_dev *pdev   = (pciedev_dev*)dev_id;
	struct devname_dev *dev     = (devname_dev *)(pdev->dev_str);

	printk(KERN_ALERT "DEVNAME_GET INT IRQ_STATUS %X \n", intreg);

	return IRQ_HANDLED;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
    static int devname_probe(struct pci_dev *dev, const struct pci_device_id *id)
#else 
static int __devinit devname_probe(struct pci_dev *dev, const struct pci_device_id *id)
#endif
{  
	int result               = 0;
	int tmp_brd_num = -1;
	u32 tmp_info          = 0;
	pciedev_dev       *devname_pcie_dev;
	void*                   address;

	printk(KERN_ALERT "DEVNAME-PCIEDEV_PROBE CALLED \n");
	result = pciedev_probe_exp(dev, id, &devname_fops, devname_cdev_m, DEVNAME, &tmp_brd_num);
	if(!(devname_cdev_m->pciedev_dev_m[tmp_brd_num]->pciedev_all_mems)){
		printk(KERN_ALERT "DEVNAME-PCIEDEV_PROBE CALLED; NO BARs \n");
		result = pciedev_remove_exp(dev,  devname_cdev_m, DEVNAME, &tmp_brd_num);
		printk(KERN_ALERT "DEVNAME-PCIEDEV_REMOVE_EXP CALLED  FOR SLOT %i\n", tmp_brd_num);  
		return -ENOMEM;
	}
	/*if board has created we will create our structure and pass it to pcedev_dev*/
	if(!result){
		printk(KERN_ALERT "SIS8160-PCIEDEV_PROBE_EXP CREATING CURRENT STRUCTURE FOR BOARD %i\n", tmp_brd_num);
		devname_pcie_dev = devname_cdev_m->pciedev_dev_m[tmp_brd_num];
		devname_dev_pp = kzalloc(sizeof(devname_dev), GFP_KERNEL);
		if(!devname_dev_pp){
				return -ENOMEM;
		}
		printk(KERN_ALERT "DEVNAME-PCIEDEV_PROBE CALLED; CURRENT STRUCTURE CREATED \n");
		devname_dev_p[tmp_brd_num] = devname_dev_pp;
		devname_dev_pp->brd_num      = tmp_brd_num;
		devname_dev_pp->parent_dev  = devname_cdev_m->pciedev_dev_m[tmp_brd_num];
		pciedev_set_drvdata(devname_cdev_m->pciedev_dev_m[tmp_brd_num], devname_dev_p[tmp_brd_num]);
		pciedev_setup_interrupt(devname_interrupt, devname_cdev_m->pciedev_dev_m[tmp_brd_num], DEVNAME, 0); 
		
		/*****If needed : some read write to/from device*****/
		address = pciedev_get_baraddress(BAR0, devname_pcie_dev);
		iowrite32(0x1, (address + 0x10));
		smp_wmb();
		/*Collect INFO; usually in the address 0x0 the board ID and board version*/
		tmp_info = ioread32(address + 0x0);
		smp_rmb();
		devname_cdev_m->pciedev_dev_m[tmp_brd_num]->brd_info_list.PCIEDEV_BOARD_ID = tmp_info;
		devname_cdev_m->pciedev_dev_m[tmp_brd_num]->brd_info_list.PCIEDEV_BOARD_VERSION = tmp_info & 0xFF;
	}
    return result;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
static void devname_remove(struct pci_dev *dev)
#else
static void __devexit devname_remove(struct pci_dev *dev)
#endif
{
     int result               = 0;
     int tmp_slot_num = -1;
     int tmp_brd_num = -1;
     tmp_brd_num =pciedev_get_brdnum(dev);
     /* clean up any allocated resources and stuff here */
     kfree(devname_dev_p[tmp_brd_num]);
     /*now we can call pciedev_remove_exp to clean all standard allocated resources
      will clean all interrupts if it seted 
      */
     result = pciedev_remove_exp(dev,  devname_cdev_m, DEVNAME, &tmp_slot_num);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
	static struct pci_driver pci_devname_driver = {
		.name        = DEVNAME,
		.id_table    = devname_ids,
		.probe       = devname_probe,
		.remove    = devname_remove,
	};
#else
	static struct pci_driver pci_devname_driver = {
		.name        = DEVNAME,
		.id_table    = devname_ids,
		.probe       = devname_probe,
		.remove    = __devexit_p(devname_remove),
	};
#endif

static int devname_open( struct inode *inode, struct file *filp )
{
    int    result = 0;
    result = pciedev_open_exp( inode, filp );
    return result;
}

static int devname_release(struct inode *inode, struct file *filp)
{
    int result            = 0;
    result = pciedev_release_exp(inode, filp);
    return result;
} 

static ssize_t devname_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    ssize_t    retval         = 0;
    retval  = pciedev_read_exp(filp, buf, count, f_pos);
    return retval;
}

static ssize_t devname_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    ssize_t         retval = 0;
    retval = pciedev_write_exp(filp, buf, count, f_pos);
    return retval;
}

static int devname_remap_mmap(struct file *filp, struct vm_area_struct *vma)
{
	ssize_t         retval = 0;
	retval =pciedev_remap_mmap_exp(filp, vma);
	return 0;
}

static long  devname_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long result = 0;
    
    if (_IOC_TYPE(cmd) == PCIEDOOCS_IOC){
        if (_IOC_NR(cmd) <= PCIEDOOCS_IOC_MAXNR && _IOC_NR(cmd) >= PCIEDOOCS_IOC_MINNR) {
			if (_IOC_NR(cmd) <= PCIEDOOCS_IOC_DMA_MAXNR && _IOC_NR(cmd) >= PCIEDOOCS_IOC_DMA_MINNR) {
                result = devname_ioctl_dma(filp, &cmd, &arg);
            }else{
            result = pciedev_ioctl_exp(filp, &cmd, &arg, devname_cdev_m);
		  }
        }else{
                if (_IOC_NR(cmd) <= DEVNAME_IOC_MAXNR && _IOC_NR(cmd) >= DEVNAME_IOC_MINNR) {
                        result = devname_ioctl_dma(filp, &cmd, &arg);
                    }else{
                        return -ENOTTY;
                    }
            }
    }else{
         return -ENOTTY;
    }
	
    return result;
}

static void __exit devname_cleanup_module(void)
{
	printk(KERN_NOTICE "DEVNAME_CLEANUP_MODULE: PCI DRIVER UNREGISTERED\n");
	pci_unregister_driver(&pci_devname_driver);
	printk(KERN_NOTICE "DEVNAME_CLEANUP_MODULE CALLED\n");
	upciedev_cleanup_module_exp(&devname_cdev_m);
}

static int __init devname_init_module(void)
{
		int   result  = 0;
		printk(KERN_WARNING "DEVNAME_INIT_MODULE CALLED\n");
		result = upciedev_init_module_exp(&devname_cdev_m, &devname_fops, DEVNAME);
		result = pci_register_driver(&pci_devname_driver);
		printk(KERN_ALERT "DEVNAME_INIT_MODULE:REGISTERING PCI DRIVER RESUALT %d\n", result);
		return 0; /* succeed */
}

module_init(devname_init_module);
module_exit(devname_cleanup_module);


