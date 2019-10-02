#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include "pciedev_io.h"

/* useconds from struct timeval */
#define	MILLS(tv) (((double)(tv).tv_usec ) + ((double)(tv).tv_sec * 1000000.0)) 	


int	         fd;
struct timeval   start_time;
struct timeval   end_time;

int main(int argc, char* argv[])
{
    int	 ch_in        = 0;
    char nod_name[15] = "";
    device_rw	          l_Read;
    device_ioctrl_data    io_RW;
    device_ioctrl_time    DMA_TIME;;
    int			  tmp_mode;
    u_int	          tmp_offset;
    int      	          tmp_data;
    int      	          tmp_barx;
    float                 tmp_fdata;
    int                   len = 0;
    int                   k = 0;
    int                   itemsize = 0;
    
    itemsize = sizeof(device_rw);
    printf("ITEMSIZE %i \n",itemsize);
	
    if(argc ==1){
        printf("Input \"prog /dev/damc0\" \n");
        return 0;
    }

    strncpy(nod_name,argv[1],sizeof(nod_name));
    fd = open (nod_name, O_RDWR);
    if (fd < 0) {
        printf ("#CAN'T OPEN FILE \n");
        exit (1);
    }

    while (ch_in != 11){
        printf("\n READ (1) or WRITE (0) or END (11) ?-");
        printf("\n GET DRIVER VERSION (2) or GET FIRMWARE VERSION (3)?-");
        printf("\n GET SLOT NUM (4) or GET_DMA_TIME (5) or GET_INFO (6) ?-");
        scanf("%d",&ch_in);
        fflush(stdin);
        l_Read.offset_rw   = 0;
        l_Read.data_rw     = 0;
        l_Read.mode_rw     = 0;
        l_Read.barx_rw     = 0;
        l_Read.size_rw     = 0;
        l_Read.rsrvd_rw    = 0;
	switch (ch_in){
            case 0 :
                printf ("\n INPUT  BARx (0,1,2,3...)  -");
                scanf ("%x",&tmp_barx);
                fflush(stdin);

                printf ("\n INPUT  MODE  (0-D8,1-D16,2-D32)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);

                printf ("\n INPUT  ADDRESS (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);

                printf ("\n INPUT DATA (IN HEX)  -");
                scanf ("%x",&tmp_data);
                fflush(stdin);

                l_Read.data_rw   = tmp_data;
                l_Read.offset_rw = tmp_offset;
                l_Read.mode_rw   = tmp_mode;
                l_Read.barx_rw   = tmp_barx;
                l_Read.size_rw   = 0;
                l_Read.rsrvd_rw  = 0;

                printf ("MODE - %X , OFFSET - %X, DATA - %X\n",  
                     l_Read.mode_rw, l_Read.offset_rw, l_Read.data_rw);

                len = write (fd, &l_Read, sizeof(device_rw));
                if (len != itemsize ){
                        printf ("#CAN'T READ FILE return %i\n", len);
                }

                break;
	    case 1 :
                printf ("\n INPUT  BARx (0,1,2,3)  -");
                scanf ("%x",&tmp_barx);
                fflush(stdin);
                printf ("\n INPUT  MODE  (0-D8,1-D16,2-D32)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                printf ("\n INPUT OFFSET (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);		
                l_Read.data_rw   = 0;
                l_Read.offset_rw = tmp_offset;
                l_Read.mode_rw   = tmp_mode;
                l_Read.barx_rw   = tmp_barx;
                l_Read.size_rw   = 0;
                l_Read.rsrvd_rw  = 0;
                printf ("MODE - %X , OFFSET - %X, DATA - %X\n", 
                        l_Read.mode_rw, l_Read.offset_rw, l_Read.data_rw);                
                len = read (fd, &l_Read, sizeof(device_rw));
                if (len != itemsize ){
		   printf ("#CAN'T READ FILE return %i\n", len);
		}
                
                printf ("READED : MODE - %X , OFFSET - %X, DATA - %X\n", 
			l_Read.mode_rw, l_Read.offset_rw, l_Read.data_rw);
		break;
           case 2 :
                ioctl(fd, PCIEDEV_DRIVER_VERSION, &io_RW);
                tmp_fdata = (float)((float)io_RW.offset/10.0);
                tmp_fdata += (float)io_RW.data;
                printf ("DRIVER VERSION IS %f\n", tmp_fdata);
                break;
	    case 3 :
                ioctl(fd, PCIEDEV_FIRMWARE_VERSION, &io_RW);
                printf ("FIRMWARE VERSION IS - %X\n", io_RW.data);
		break;
            case 4 :
                ioctl(fd, PCIEDEV_PHYSICAL_SLOT, &io_RW);
                printf ("SLOT NUM IS - %X\n", io_RW.data);
		break;
            case 5:
                len = ioctl (fd, PCIEDEV_GET_DMA_TIME, &DMA_TIME);
                if (len) {
                    printf ("######ERROR GET TIME %d\n", len);
                }
                printf ("===========DRIVER TIME \n");
                printf("STOP DRIVER TIME START %li:%li STOP %li:%li\n",
                                                            DMA_TIME.start_time.tv_sec, DMA_TIME.start_time.tv_usec, 
                                                            DMA_TIME.stop_time.tv_sec, DMA_TIME.stop_time.tv_usec);
                break;
            case 6 :
                l_Read.offset_rw   = 0;
                l_Read.data_rw     = 0;
                l_Read.mode_rw     = 4;
                l_Read.barx_rw     = 0;
                l_Read.size_rw     = 0;
                l_Read.rsrvd_rw    = 0;

                len = read (fd, &l_Read, sizeof(device_rw));
                if (len != itemsize ){
		   printf ("#CAN'T READ FILE return %i\n", len);
		}

                printf ("READED : DRV_VERSION - %i.%i \n",  l_Read.data_rw, l_Read.offset_rw);
                printf ("READED : FRM_VERSION - %i\n", l_Read.mode_rw);
                printf ("READED : SLOT_NUM    - %i \n", l_Read.size_rw);
                printf ("READED : DRV_MEMS    - %X \n", l_Read.barx_rw);
                for(k = 0; k < 5; k++){
                    printf("BAR Nm %i - %i\n", k, ((l_Read.barx_rw > k)&0x1));
                }
		break;
	   default:
		break;
	}
    }

    close(fd);
    return 0;
}

