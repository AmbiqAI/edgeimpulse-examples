//____________________________________________________________________
// Copyright Info     :  All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
//Name                  :  sdmmc26.c
//
//Creation Date      :  12-April-2007
//
//Description          : This file contains the code of block driver for SD/MMC card for linux kernel 2.6.xx.
//
//Changed History  :
//<Date>                   <Author>        <Version>        < Description >
//12-April-2007                DS                  0.1                 base code
//____________________________________________________________________


/*   Steps to create nodes for this block driver
*     mknod /dev/BlockDev0 b 253 0  //first device with 16 possible partitions //first minor for first device
*     mknod /dev/BlockDev0p1 b 253 1 //first partition
*     mknod /dev/BlockDev0p2 b 253 2 //second partition

*     mknod /dev/BlockDev1 b 253 16 //second device //first minor for second device
*     mknod /dev/BlockDev1p1 b 253 17 //first partition
*     mknod /dev/BlockDev1p2 b 253 18 //second partition

*     fdisk -l /dev/BlockDev0  //quering first device
*     fdisk -l /dev/BlockDev1  //quering second
*/
/************************************************************************/

/*
 * Standard Kernel Header Files for Block Driver
 */
#include <linux/init.h>
#include <linux/module.h>       //module.c-> insmod/loading operations defined
#include <linux/moduleparam.h>  //module.c-> insmod/loading operations defined
#include <linux/fs.h>           //block device operations
#include <linux/blkdev.h>       //includes linux/genhd.h(gendisk structure)
#include <linux/hdreg.h>        //geometry
#include <linux/completion.h>   //for non blocking part
#include <linux/interrupt.h>    //tasklets
#include <linux/workqueue.h>    //workqueues
#include <linux/kthread.h>
#include <linux/fcntl.h>
#include <linux/scatterlist.h>

//  User defined header files
#include "nex_api.h"
#include "nex_common_types.h"
#define SET_BUS_INFO       1
//#define GET_CARD_INFO      2
#define CARD_TYPE          3
#define GET_CARD_CSD       4
#define SET_HS_CARD        5
#define SET_FS_CARD        6
#define SET_SDR12_CARD     7
#define SET_SDR25_CARD     8
#define SET_SDR50_CARD     9

#define SET_DDR50_CARD    10
#define GET_CARD_CAPACITY 11
#define READ_REG_ADDRESS  12
#define WRITE_REG_ADDRESS 13
#define WRITE_REG_VALUE   14
#define GET_CARD_INFO      15
#define SET_SDR104_CARD     16

#define ADMA2_ATB_TRAN (ADMA2_ATTRIB_TRAN|ADMA2_ATTRIB_VALID)
#define ADMA2_ATB_END (ADMA2_ATTRIB_TRAN|ADMA2_ATTRIB_END| ADMA2_ATTRIB_VALID)




//#Defined items
#define SUCCESS                          0
#define FAILURE                         -1
#define TOTAL_MINORS                     16
#define REQUEST_FUNCTION BlkRequestIssue
#define HARDWARE_SECTOR_SIZE             512
#define SECTORS_PER_TRACK                16
#define DISK_HEADS                       4


#define MAX_BLK_COUNT 			65536




// Definition for Card Types
#define INVALID_CARD                    -1
#define MMC_CARD                         0
#define SD_CARD                          1
#define SDIO_CARD                        2
#define SDHC_CARD                        3
#define MMCHC_CARD                       4
#define eMMC_CARD                        5
#define eMMCHC_CARD                      6
#define CEATA_CARD                       7
#define MEMSTICK_PRO_CARD                8
#define SDXC_CARD                        9


#define	m_SET_FIXED_CAPACITY(BlkDevice) set_capacity((BlkDevice->gen_disk),((1024*1024*100)/HARDWARE_SECTOR_SIZE));
#define DEBUG_LEVEL 0

//Debug Code
#if (DEBUG_LEVEL == 0)
#define INIT_PRINT(format, args...)
#define EXIT_PRINT(format, args...)
#define WRITE_PRINT(format, args...)
#elif (DEBUG_LEVEL == 1)
#define INIT_PRINT(format, args...)
#define EXIT_PRINT(format, args...)
#define WRITE_PRINT(format, args...) printk(KERN_ALERT "         %s %d]"   format, __FUNCTION__, __LINE__,##args)
#elif (DEBUG_LEVEL == 2)
#define INIT_PRINT(format, args...)  printk(KERN_ALERT "Entering %s %d]\n" format, __FUNCTION__, __LINE__,##args)
#define EXIT_PRINT(format, args...)  printk(KERN_ALERT "Exiting  %s %d]\n" format, __FUNCTION__, __LINE__,##args)
#define WRITE_PRINT(format, args...) printk(KERN_ALERT "         %s %d]"   format, __FUNCTION__, __LINE__,##args)
#endif


#define cur_sector(req) blk_rq_cur_sectors(req)
#define offset(req) blk_rq_pos(req)





//  Global structure for representing Device
struct BlockDevice {
int   SlotNum;            //slot number of the card
int            CardType;           //Type of Card MMC/SD
unsigned int   DevSize_in_Sectors; //actual size of device/disk in sectors dealing here
unsigned short DevUse;
unsigned short MediaChange;        //0 = no media changed, 1 = changed
unsigned short CardPresent;        //0 = card is present else not
spinlock_t     DevLock;            //locking
struct gendisk *gen_disk;          //gendisk for the device
struct request_queue *queue;       //request queue for this device : for reference after deleting gendisk
struct scatterlist *sg;
int sg_len;
struct request *req;
struct task_struct *thread;
struct completion cp;
int status;
struct semaphore	thread_sem;
char			*bounce_buf;
struct scatterlist	*bounce_sg;
unsigned int		bounce_sg_len;
int sdhcNum;

};



// Global declaration

int total_sdhc = MAX_SDHC;
int total_slot = MAX_SLOTS;

int slotPerSDHC[MAX_SDHC];

struct request_queue *gl_Queue; // data;
static struct BlockDevice *gl_BlkDevice[MAX_SDHC][MAX_SLOTS]; //global pointer to all the devices 
unsigned int BlkMajor = 0;
unsigned int dmaMode  = 1;
unsigned int buswidth=1;
struct completion comp;
unsigned int GenDiskStatus[MAX_SDHC][MAX_SLOTS];      // 0 == gendisk is ready, 1 == gendisk is not ready
                                            // used for tracking status for card removal
unsigned int GenDiskOpenStatus[MAX_SDHC][MAX_SLOTS];  // 0 == gendisk is ready after open, 1 == gendisk is not ready after open
unsigned int sectors_per_track=0;
unsigned int disk_heads=0;
                                            //used for tracking status of gendisk during initialization.
SD_CALL_BACK_INFO gl_CallBackInfo;          //for reference in work_handler
struct semaphore cleanup_sema;
struct semaphore work_to_do_sema;

unsigned short		max_hw_segs;	/* see blk_queue_max_hw_segments */
unsigned int bounce = 1;
unsigned int sg =1;


// Function Declaration
void UsrFunctionPtr(SD_CALL_BACK_INFO *);
static void BlkRequestIssue(struct request_queue *RequestQueue);
static int Initialize_Devices(struct BlockDevice *BlkDevice, int sdhcNum,int slotNum);
static int __init BlkInitialize(void);
void BlkSetupBus(int i,int j);
static int BlkRequestTransfer(struct gendisk *gd, struct request *pRequest,char r_w);
unsigned int BlkQueueMapSG(struct BlockDevice *BlkDevice);
void BlkQueueBouncePre(struct BlockDevice *BlkDevice);
void BlkQueueBouncePost(struct BlockDevice *BlkDevice);

int Blkioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long arg);




// Name        : BlkDevice_queue_thread
//
// Argument  : void pointer to Blockdevice
//
//Returns      : int : 0 == Success, Else Error
//
//Description : This function gets the request from the request queue.
//
static int BlkDevice_queue_thread(void *d)
{
	struct BlockDevice *BlkDevice = d;
	struct gendisk *gd;
	int chk_api_resp;

	struct request_queue *rq = BlkDevice->queue;
	char r_w;

	
	INIT_PRINT();
	current->flags |= PF_MEMALLOC;

	down(&BlkDevice->thread_sem);
	do {
		struct request *req = NULL;

		spin_lock_irq(rq->queue_lock);
		set_current_state(TASK_INTERRUPTIBLE);
		if (!blk_queue_plugged(rq))
			req = blk_fetch_request(rq);
		BlkDevice->req = req;
		gd = BlkDevice->gen_disk;
		spin_unlock_irq(rq->queue_lock);

		if (!req) {
			if (kthread_should_stop()) {
				set_current_state(TASK_RUNNING);
				break;

			}
			up(&BlkDevice->thread_sem);
			schedule();
			down(&BlkDevice->thread_sem);
			continue;
		}
		set_current_state(TASK_RUNNING);

		if(rq_data_dir(req) == 0)
			r_w = 'R';
		else
			r_w = 'W';
		

		chk_api_resp = BlkRequestTransfer(gd, req, r_w);

	} while (1);
	up(&BlkDevice->thread_sem);

	EXIT_PRINT();
	return SUCCESS;
}


// Name        : UsrFunctionPtr
//
// Argument  : pointer to SD_CALL_BACK_INFO
//
//Returns      : void
//
//Description : This function is called when the I/O job completed interrupt and interrupt is raised to inform the function completion

//                    or card removal/inserted processing.

//

void UsrFunctionPtr(SD_CALL_BACK_INFO  *pCallBackInfo)
{
	struct BlockDevice *BlkDevice = NULL;
	INIT_PRINT();

	BlkDevice = gl_BlkDevice[pCallBackInfo->sdhcNum][pCallBackInfo->slot];
	WRITE_PRINT("CALLBACKINFO->INTR_TYPE IS [0x%0X]\n",pCallBackInfo->intr_type);

	//case : when waiting for data_complete and comes card removed
	//need to do this imidiatly as data_complete may come before card_removed processing.
	//as we check cardpresent for ending request
	if(SD_CARD_REMOVED == pCallBackInfo->intr_type)
	{
		if(BlkDevice==NULL)
		{
			WRITE_PRINT("ERROR : BlkDevice is NULL\n");
			return;
		}
		BlkDevice->CardPresent = 1; // 1 == card is not present
	}

	//for I/O completion part
	if (SD_DATA_COMPLETE == pCallBackInfo->intr_type) //SD_DATA_COMPLETE = 5
	{
		WRITE_PRINT("\nSD_DATA_COMPLETE for slot %d%d\n", BlkDevice->sdhcNum,BlkDevice->SlotNum);
		if(pCallBackInfo->status != SD_SUCCESS)
		{
			WRITE_PRINT("Request failed with error %d\n", pCallBackInfo->status);
			BlkDevice->status = -EIO;
		}
		else
		{
			WRITE_PRINT("Request successfull. status = %d\n",pCallBackInfo->status);
			BlkDevice->status = SD_SUCCESS;
		}
		complete(&BlkDevice->cp);
		EXIT_PRINT();
		return;
	}
	else
	{
		WRITE_PRINT("\nSD_DATA_COMPLETE not received");
		EXIT_PRINT();
		return;
	}
}


// Name        : BlkRequestTansfer
//
// Argument  : struct gendisk *gd : pointer to gendisk associated
//                    struct request *pRequest : current request for transfer
//                    char r_w : read or write request
//
//Returns      : int : 0 == Success, Else Error
//
//Description : This function calls the api's that actually does the block transfer.
//
static int BlkRequestTransfer(struct gendisk *gd, struct request *pRequest,char r_w)
{
	unsigned int Offset,j;
	int ret_api = -1,ret,totalbytes=0; 
	struct scatterlist *s;
	
	
	struct BlockDevice *BlkDevice = gd->private_data;
	SD_NEX_ADMA_TABLE adma_table;
	unsigned int blocks=0;//no. of blocks or sectors of 512 bytes
	struct sd_scatterlist sglist;
	
	
	if (!blk_fs_request(pRequest))
	{
		ret= -EIO;
		goto END;
		
	}
                   
	if(sg)
		blocks = blk_rq_sectors(pRequest);
	else
		blocks = cur_sector(pRequest);
	 if (offset(pRequest) + blocks > get_capacity(pRequest->rq_disk))
	{
		ret= -EIO;
		goto END;
		
	}
	
	totalbytes = blocks*512;

	WRITE_PRINT("Read/Write is : %c \n", r_w);
	WRITE_PRINT("No. of Sec.          : %d \n", blocks);
	WRITE_PRINT("Length            : %u   \n",totalbytes);
	WRITE_PRINT("Offset (in Sectors)  : 0x%x \n",(int)offset(pRequest));
	
	do{
	
	
		if(sg)
		{
		
			if(blocks > MAX_BLK_COUNT)
				blocks = MAX_BLK_COUNT;
			
			
			sglist.sg = (SCATTERLIST *)BlkDevice->sg;
			sglist.sg_len = BlkQueueMapSG(BlkDevice);
			
		
				if(dmaMode==2)
				{
					s=(struct scatterlist *)sglist.sg; 
					
					for(j=0;j<sglist.sg_len;j++)
					{
						s[j].length = ((s[j].length<< 16) & 0xFFFF0000)| ADMA2_ATB_TRAN;
						WRITE_PRINT("length of sg[%d] = %x\n",j,s[j].length);
					}
					 s[j-1].length |= ADMA2_ATB_END;
					 WRITE_PRINT("length of  sg[%d] = %x\n",j-1,s[j-1].length);
				}
		}
			
	
		if ((SDHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)) || (SDXC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)) || (MMCHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)) ||
					(eMMCHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))||(MEMSTICK_PRO_CARD==CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)))
			Offset = offset(pRequest);
		else
			
			Offset = offset(pRequest)*512;
	
 	

	
		if(sg)
		{
			BlkQueueBouncePre(BlkDevice);
		}
	
		if ('R' == r_w)
		{
		
			if (0 == dmaMode)
			{
				WRITE_PRINT("GetData Transfer using PIO\n");
				if(sg)
					ret_api = GetData((void *)&sglist, totalbytes, Offset,BlkDevice->sdhcNum, BlkDevice->SlotNum);
				else
					ret_api = GetData(pRequest->buffer, totalbytes, Offset, BlkDevice->sdhcNum,BlkDevice->SlotNum);
				
				
			}
			else if (1 == dmaMode) //DMA transfer
			{
#if 0
				if (SDXC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
				{
					WRITE_PRINT("ADMA1 is not applicable for SDXC card\n");
					return FAILURE;
				}
#endif
				WRITE_PRINT("GetData Transfer with DMA ...\n");
				if(sg)
					ret_api = GetData((void *)&sglist, totalbytes, Offset, BlkDevice->sdhcNum,BlkDevice->SlotNum);
				else
					ret_api = GetData(pRequest->buffer, totalbytes, Offset,BlkDevice->sdhcNum, BlkDevice->SlotNum);
				
			}
			else if (2 == dmaMode)// ADMA Transfer
			{
				WRITE_PRINT("Going for GetData using ADMA2 for size 0x%x...\n",  totalbytes);
								
				if(sg)
					ret_api = GetData((void *)&sglist, totalbytes, Offset, BlkDevice->sdhcNum,BlkDevice->SlotNum);
				else
				{
					
					adma_table.desc = kzalloc(sizeof(SD_NEX_ADMA_TABLE)*1, GFP_ATOMIC);
					adma_table.num_tables = 1;
					adma_table.desc[0].adma_list = kzalloc(sizeof(SD_NEX_ADMA_LIST)*1, GFP_ATOMIC);
					adma_table.desc[0].num_entries=1;
					adma_table.desc[0].adma_list[0].buffer =  pRequest->buffer;
					adma_table.desc[0].adma_list[0].buffer_len = totalbytes;
					adma_table.desc[0].adma_list[0].link = 0;
					adma_table.desc[0].adma_list[0].attrib =((totalbytes<< 16) & 0xFFFF0000)|  ADMA2_ATB_END;
					ret_api = GetData((void *)&adma_table, totalbytes, Offset,BlkDevice->sdhcNum, BlkDevice->SlotNum);
				}
				
			}
		}
		else if ('W' == r_w)
		{
		
		 if (IsCardInitialized(BlkDevice->sdhcNum,BlkDevice->SlotNum) != MSPRO_READ_ONLY)
		  {
			if (0 == dmaMode)
			{
				WRITE_PRINT("SendData Transfer using PIO\n");
				if(sg)
					ret_api = SendData((void *)&sglist, totalbytes, Offset, BlkDevice->sdhcNum,BlkDevice->SlotNum);
				else
					ret_api = SendData(pRequest->buffer, totalbytes, Offset,BlkDevice->sdhcNum, BlkDevice->SlotNum);

			}
			else if (1 == dmaMode)
			{
#if 0
				if (SDXC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
				{
					WRITE_PRINT("ADMA1 is not applicable for SDXC card\n");
					return FAILURE;
				}
#endif

				WRITE_PRINT("SendData Transfer using DMA\n");
				if(sg)
					ret_api = SendData((void *)&sglist, totalbytes, Offset,BlkDevice->sdhcNum, BlkDevice->SlotNum);
				else
					ret_api = SendData(pRequest->buffer, totalbytes, Offset, BlkDevice->sdhcNum,BlkDevice->SlotNum);
			}
			else if (2 == dmaMode)// for AMDA2
			{
				WRITE_PRINT("Going for SendData using ADMA2 for size 0x%x...\n",  totalbytes);					

				
				if(sg)
					ret_api = SendData((void *)&sglist, totalbytes, Offset, BlkDevice->sdhcNum,BlkDevice->SlotNum);
				else
				{
					adma_table.desc = kzalloc(sizeof(SD_NEX_ADMA_TABLE)*1, GFP_ATOMIC);
					adma_table.num_tables = 1;
					adma_table.desc[0].adma_list = kzalloc(sizeof(SD_NEX_ADMA_LIST)*1, GFP_ATOMIC);
					adma_table.desc[0].num_entries=1;
					adma_table.desc[0].adma_list[0].buffer =  pRequest->buffer;
					adma_table.desc[0].adma_list[0].buffer_len = totalbytes;
					adma_table.desc[0].adma_list[0].link = 0;
					adma_table.desc[0].adma_list[0].attrib =((totalbytes<< 16) & 0xFFFF0000)|  ADMA2_ATB_END;
					ret_api = SendData((void *)&adma_table, totalbytes, Offset, BlkDevice->sdhcNum,BlkDevice->SlotNum);
				}
				
			}
		  }
		}
	
		wait_for_completion(&BlkDevice->cp);

		
		if(sg)
			BlkQueueBouncePost(BlkDevice);

		if(BlkDevice->status == SD_SUCCESS) // I/O success
		{
	  		ret = 0;
		
		}
		else //I/O Error
		{
			ret = -EIO;
		
		}

END: 
		spin_lock_irq(BlkDevice->queue->queue_lock);
		
		ret = __blk_end_request(pRequest, ret, totalbytes);
		
		spin_unlock_irq(BlkDevice->queue->queue_lock);

	if (2 == dmaMode)
	{
		if(!sg)
		{
			
			kfree(adma_table.desc[0].adma_list);
			kfree(adma_table.desc);

		}

	}

		
	}while(ret);
	return ret;	

}


// Name        : BlkRequestIssue
//
// Argument  : struct request_queue *RequestQueue: pointer to request Queue associated
//
//Returns      : void
//
//Description : This is the request issue function. it picks up a request does the checks on request and issues it for transfer.
//
static void BlkRequestIssue(struct request_queue *RequestQueue)
{

	struct BlockDevice *BlkDevice;
	struct request *req;
	INIT_PRINT();
	BlkDevice = RequestQueue->queuedata;


	if (!BlkDevice) 
	{
		WRITE_PRINT("killing requests for dead queue\n");
		while ((req = blk_fetch_request(RequestQueue)) != NULL)
		{
			
			req->cmd_flags |= REQ_QUIET;
			spin_lock_irq(BlkDevice->queue->queue_lock);
			__blk_end_request_all(req, -EIO);
			spin_unlock_irq(BlkDevice->queue->queue_lock);
		}

		return;
	}

	if (!BlkDevice->req)
		wake_up_process(BlkDevice->thread);


	EXIT_PRINT();
}



// Name        : BlkQueueMapSG
//
// Argument  : struct BlockDevice *BlkDevice: Block Device
//
//Returns      : length of scatterlist
//
//Description : Prepare the sg list to be handed of to the host driver
//

unsigned int BlkQueueMapSG(struct BlockDevice *BlkDevice)
{
	unsigned int sg_len;
	size_t buflen;
	struct scatterlist *sg;
	int i;

	if (!BlkDevice->bounce_buf)
		return blk_rq_map_sg(BlkDevice->queue, BlkDevice->req, BlkDevice->sg);

	sg_len = blk_rq_map_sg(BlkDevice->queue, BlkDevice->req, BlkDevice->bounce_sg);

	BlkDevice->bounce_sg_len = sg_len;

	buflen = 0;
	for_each_sg(BlkDevice->bounce_sg, sg, sg_len, i)
		buflen += sg->length;

	sg_init_one(BlkDevice->sg, BlkDevice->bounce_buf, buflen);

	return 1;
}

// Name        : BlkQueueBouncePre
//
// Argument  : struct BlockDevice *BlkDevice: Block Device
//
//Returns      : None
//
//Description :  If writing, this API bounce the data to the buffer before the request is sent to the host driver
//

void BlkQueueBouncePre(struct BlockDevice *BlkDevice)
{
	unsigned long flags;

	if (!BlkDevice->bounce_buf)
		return;

	if (rq_data_dir(BlkDevice->req) != WRITE)
		return;

	local_irq_save(flags);
	
	sg_copy_to_buffer(BlkDevice->bounce_sg, BlkDevice->bounce_sg_len,
		BlkDevice->bounce_buf, BlkDevice->sg[0].length);
	local_irq_restore(flags);
	
}
/* Name        : BlkQueueBouncePost
 *
 * Argument  : struct BlockDevice *BlkDevice: Block Device
 *Returns      : None
 *Description :  If reading, this API bounce the data from the buffer after the request has been handled by the host driver
 *
 */

void BlkQueueBouncePost(struct BlockDevice *BlkDevice)
{
	unsigned long flags;

	if (!BlkDevice->bounce_buf)
		return;

	if (rq_data_dir(BlkDevice->req) != READ)
		return;

	local_irq_save(flags);
	sg_copy_from_buffer(BlkDevice->bounce_sg, BlkDevice->bounce_sg_len,
		BlkDevice->bounce_buf, BlkDevice->sg[0].length);
	local_irq_restore(flags);
}



// Name        : BlkGetGeo
//
// Argument  : struct block_device *bdev : pointer to standard block_device
//                    struct hd_geometry *geo : pointer to geometry structure
//
//Returns      : int : 0 == Success, Else Failure
//
//Description : This function calculates the number of cylinders for disk.
//
static int BlkGetGeo(struct block_device *bdev, struct hd_geometry *geo)
{
	INIT_PRINT();

	geo->heads   = DISK_HEADS;
	geo->sectors = SECTORS_PER_TRACK;
//	geo->cylinders = get_capacity(bdev->bd_disk) / ((SECTORS_PER_TRACK) * (DISK_HEADS));
//	WRITE_PRINT("CYLINDERS ARE [%u]\n",geo->cylinders);

	EXIT_PRINT();
	return SUCCESS;
}


// Name        : BlkRevalidateDisk
//
// Argument  : struct gendisk *gd : pointer to gendisk associated
//
//Returns      : int : 0 == Success, Else Failure
//
//Description : This function is called when the media is changed and to prepare new disk for usage.
//
MSPRO_INFO mspro_info;
int BlkRevalidateDisk(struct gendisk *gd)
{
	struct BlockDevice *BlkDevice = gd->private_data;
	unsigned long long MMCHC_Capacity_in_bytes=0;
	unsigned long int Capacity_in_bytes=0;
	unsigned int  Blocknr,Mult,Block_len;
	NEX_MMC_CSD   mmc_csd;
	NEX_SD_CSD_V1 sd_csd_info; //structure decoding will be based upon structure version
	NEX_SD_CSD_V2 sd_csd2_info;
	NEX_MMC_EXT_CSD mmc_ext_csd;

	INIT_PRINT();

	memset(&mmc_csd,     0, sizeof(NEX_MMC_CSD));
	memset(&sd_csd_info, 0, sizeof(NEX_SD_CSD_V1));
	memset(&sd_csd2_info,0, sizeof(NEX_SD_CSD_V2));
	memset(&mspro_info,0, sizeof(MSPRO_INFO));
	memset(&mmc_ext_csd,0, sizeof(NEX_MMC_EXT_CSD));
	memset(&mspro_info,0, sizeof(MSPRO_INFO));
	memset(&mspro_info.mspro_sys_info,0,sizeof(MSPRO_DEVINFO));

	//write code for media changed eg init new disk, kernel will read partition table after this function : if called by kernel
	if(BlkDevice->MediaChange)
	{
		// code for checking the type of card
		if (MMC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
		{
			BlkDevice->CardType = CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum);
			GetCardSpecificInfo_CSD(&mmc_csd,BlkDevice->sdhcNum,BlkDevice->SlotNum);
		if(mmc_csd.csd_struct < 2) 
       		{
			/*
			 *     capacity is = blocknr * block_len
			 *     blocknr     = (c_size(i.e. m3) + 1 ) * mult
			 *     mult         =  2 ^ (c_size_mult(i.e. e3) +2)
			 *     block_len  =  2 ^ read_bl_len(here read_blkbits)
			 */
			Mult              = 1 << (mmc_csd.e3 +2);
			Blocknr           = (mmc_csd.m3 + 1) * Mult;
			Block_len         = 1 << mmc_csd.read_blkbits;
			Capacity_in_bytes = Blocknr * Block_len;

      			if(BlkDevice->CardType == MMC_CARD)
			WRITE_PRINT("Card type in slot[%d%d] is MMC Card \n",BlkDevice->sdhcNum, BlkDevice->SlotNum);

			else
				WRITE_PRINT("Card type in slot[%d%d] is eMMC Card \n", BlkDevice->sdhcNum,BlkDevice->SlotNum);
			WRITE_PRINT("Capacity %lu Bytes, no.Blocks[%d], Blk_len[%d] \n", Capacity_in_bytes,Blocknr,  Block_len);
			BlkDevice->DevSize_in_Sectors = Capacity_in_bytes / HARDWARE_SECTOR_SIZE;
			WRITE_PRINT("Dev_Size in Sectors [%d]\n",BlkDevice->DevSize_in_Sectors);
		}
		else
		{
			GetCardSpecificInfo_ExtCSD(&mmc_ext_csd,BlkDevice->sdhcNum,BlkDevice->SlotNum);
			WRITE_PRINT("mmc_ext_csd.sec_count [%llx]\n",mmc_ext_csd.sec_count);
			MMCHC_Capacity_in_bytes = ((mmc_ext_csd.sec_count)  * 512 );
			BlkDevice->DevSize_in_Sectors = mmc_ext_csd.sec_count;
			WRITE_PRINT("CAPACITY FOR SLOT [%d%d] IS %llx Bytes. \n", BlkDevice->sdhcNum,BlkDevice->SlotNum, MMCHC_Capacity_in_bytes);
			WRITE_PRINT("SIZE IN SECTORS [%0X]\n",BlkDevice->DevSize_in_Sectors);
		}
		}
		else if (SD_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
		{

			BlkDevice->CardType = SD_CARD;
			GetCardSpecificInfo_CSD(&sd_csd_info,BlkDevice->sdhcNum,BlkDevice->SlotNum);
			WRITE_PRINT("SD_CSD Version is [%d]\n",sd_csd_info.csd_struct);

			if(0 == sd_csd_info.csd_struct) //code for version 1
			{
				/*Calculate capacity by csd 1.0 structure
				 *---------------------------------------
				 *     capacity is      = blocknr * block_len
				 *     blocknr          = (c_size + 1 ) * mult
				 *     mult              = 2 ^ (c_size_mult + 2)
				 *     block_len       = 2 ^ read_bl_len
				 */

				Mult              = 1 << (sd_csd_info.c_size_mult + 2);
				Blocknr           = (sd_csd_info.c_size + 1) * Mult;
				Block_len         = 1 << sd_csd_info.read_bl_len;
				Capacity_in_bytes = Blocknr * Block_len;
				WRITE_PRINT("Card type in slot[%d%d] is SD Card \n", BlkDevice->sdhcNum,BlkDevice->SlotNum );
				WRITE_PRINT("Its Capacity is %ld Bytes \n", Capacity_in_bytes);
				BlkDevice->DevSize_in_Sectors = (Capacity_in_bytes / HARDWARE_SECTOR_SIZE);
				WRITE_PRINT("Dev_Size in Sectors [%d]\n",BlkDevice->DevSize_in_Sectors);
			}
			else if (1 == sd_csd_info.csd_struct ) //code for version 2

			{
				GetCardSpecificInfo_CSD(&sd_csd2_info,BlkDevice->sdhcNum,BlkDevice->SlotNum);
				//calculate capacity by csd 2.0 structure
				Capacity_in_bytes = ((sd_csd2_info.c_size + 1) * 512 * 1024); //multiply by 1024 for : capacity in bytes
				WRITE_PRINT("Card type in slot[%d%d] is SD Card \n",BlkDevice->sdhcNum, BlkDevice->SlotNum );
				WRITE_PRINT("Its Capacity is %ld Bytes \n", Capacity_in_bytes);
				BlkDevice->DevSize_in_Sectors = (Capacity_in_bytes / HARDWARE_SECTOR_SIZE);
				WRITE_PRINT("Dev_Size in Sectors [%d]\n",BlkDevice->DevSize_in_Sectors);
			}
		}
		else if (SDHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
		{
			BlkDevice->CardType = SDHC_CARD;
			GetCardSpecificInfo_CSD(&sd_csd2_info,BlkDevice->sdhcNum,BlkDevice->SlotNum);
			//WRITE_PRINT("C_SIZE IS %d", sd_csd2_info.c_size);
			WRITE_PRINT("CARD TYPE IS SD HIGH_CAPACITY. \nSD_CSD Version is [%d]\n",sd_csd2_info.csd_struct);
			if (1 == sd_csd2_info.csd_struct ) //code for version 2
			{
				BlkDevice->DevSize_in_Sectors = (((sd_csd2_info.c_size) + 1) * 1024);
			}
			WRITE_PRINT("SIZE IN SECTORS [%d] \n",BlkDevice->DevSize_in_Sectors);
		}
		else if (SDXC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
		{
			BlkDevice->CardType = SDXC_CARD;
			GetCardSpecificInfo_CSD(&sd_csd2_info,BlkDevice->sdhcNum,BlkDevice->SlotNum);
			//WRITE_PRINT("C_SIZE IS %d", sd_csd2_info.c_size);
			WRITE_PRINT("CARD TYPE IS SD Extented Capacity. \nSD_CSD Version is [%d]\n",sd_csd2_info.csd_struct);
			if (1 == sd_csd2_info.csd_struct ) //code for version 2
			{
				BlkDevice->DevSize_in_Sectors = (((sd_csd2_info.c_size) + 1) * 1024);
			}
			WRITE_PRINT("SIZE IN SECTORS [%d] \n",BlkDevice->DevSize_in_Sectors);
		}
		else if (MMCHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)|| eMMCHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum) || eMMC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum) )
		{
			BlkDevice->CardType = CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum);
			GetCardSpecificInfo_ExtCSD(&mmc_ext_csd,BlkDevice->sdhcNum,BlkDevice->SlotNum);
			WRITE_PRINT("mmc_ext_csd.sec_count [%llx]\n",mmc_ext_csd.sec_count);
			MMCHC_Capacity_in_bytes = ((mmc_ext_csd.sec_count)  * 512 );
			BlkDevice->DevSize_in_Sectors = mmc_ext_csd.sec_count;
			WRITE_PRINT("CAPACITY FOR SLOT [%x] IS %llx Bytes. \n", BlkDevice->SlotNum, MMCHC_Capacity_in_bytes);
			WRITE_PRINT("SIZE IN SECTORS [%0X]\n",BlkDevice->DevSize_in_Sectors);
		}
		else if (SDIO_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
		{
			BlkDevice->CardType = SDIO_CARD;
			WRITE_PRINT("CARD TYPE IS SDIO\n CAPACITY IS NOT APPLICABLE\n");
			BlkDevice->DevSize_in_Sectors = 0;
		}
        #if defined(ENABLE_MSPRO)
		else if (MEMSTICK_PRO_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
		{
			BlkDevice->CardType = MEMSTICK_PRO_CARD;

			/* Calculate Capacity using the System Information from Attribute Information Area
					capacity = User Area Blocks * UnitSize * BlockSize
			*/

			GetCardAttribute(&mspro_info,BlkDevice->sdhcNum,BlkDevice->SlotNum);

			Capacity_in_bytes =(mspro_info.mspro_sys_info.totaluserarea * mspro_info.mspro_sys_info.unitsize
							* mspro_info.mspro_sys_info.blksize);

			WRITE_PRINT("Its Capacity is %d Bytes \n", Capacity_in_bytes);
			BlkDevice->DevSize_in_Sectors = (Capacity_in_bytes / HARDWARE_SECTOR_SIZE);
			WRITE_PRINT("Dev_Size in Sectors [%d]\n",BlkDevice->DevSize_in_Sectors);
		}
		#endif
		else if ((int)INVALID_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
		{
			BlkDevice->CardType = INVALID_CARD;
			WRITE_PRINT("CARD TYPE IS NOT DEFINED\n INVALID CARD CANT GET ITS CAPACITY \n");
			BlkDevice->DevSize_in_Sectors = 0;
			return FAILURE;
		}
		// code to get card's csd register and calculate the capacity of card
		BlkDevice->MediaChange = 0; //0 == no media changed : media changed is restored
		set_capacity(BlkDevice->gen_disk,BlkDevice->DevSize_in_Sectors);  //setting gd->capacity
	}

	EXIT_PRINT();
	return SUCCESS;
}

// Name        : BlkMediaChange
//
// Argument  : struct gendisk *gd : pointer to gendisk associated
//
//Returns      : int : 0 == Success, Else Failure
//
//Description : This function returns the value of media changed variable.
//
int BlkMediaChange(struct gendisk *gd)
{
	struct BlockDevice *BlkDevice = gd->private_data;
	INIT_PRINT();
	EXIT_PRINT();
	return BlkDevice->MediaChange;
}

// Name        : BlkRelease
//
// Argument  : struct inode *PInode : pointer to inode associated
//                    struct file *filp : pointer to file structure associated
//
//Returns      : int : 0 == Success, Else Failure
//
//Description : This function decrements the count of device usage when user has closed the device.
//
static int BlkRelease(struct gendisk *disk, fmode_t mode) 
{
	struct BlockDevice *BlkDevice = disk->private_data;

	INIT_PRINT();
	spin_lock(&BlkDevice->DevLock);
	if(BlkDevice->DevUse)
	{
		BlkDevice->DevUse--;
	}
	spin_unlock(&BlkDevice->DevLock);
	WRITE_PRINT("Device Closed Successfully ...\n");
	EXIT_PRINT();
	return SUCCESS;
}

// Name        : BlkOpen
//
// Argument  : struct inode pInode : pointer to inode associated
//                    struct file *filp : pointer to file structure associated
//
//Returns      : int : 0 == Success, Else Failure
//
//Description : This is the open function that is used to increment the usage count of device when called by kernel.
//                    it also checks out for the change in media
//

static int BlkOpen(struct block_device *bdev, fmode_t mode)
{
		struct BlockDevice *BlkDevice = bdev->bd_disk->private_data;
	
		INIT_PRINT();
		if (NULL == BlkDevice)
		{
			WRITE_PRINT("Cannot Open : Device Not Initialized\n");
			return -ENODEV;
		}

	// Testing : Card in slot is Initialized
	if(IsCardInitialized(BlkDevice->sdhcNum,BlkDevice->SlotNum) != SD_TRUE) //no need to put it in loop
	{
		WRITE_PRINT("Card Is Not Present : During open call : slot is [%d%d] \n",BlkDevice->sdhcNum,BlkDevice->SlotNum);
		GenDiskOpenStatus[BlkDevice->sdhcNum][BlkDevice->SlotNum] = 1; //fail this gendisk in initialization
		return -ENODEV;
	}

	// Code for checking if it is a MMC/SD card, otherwise just exit
	if (SDIO_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
	{
		WRITE_PRINT("SDIO_CARD IS PRESENT DURING OPEN: EXITING\n");
		BlkDevice->CardType = SDIO_CARD;
		GenDiskOpenStatus[BlkDevice->sdhcNum][BlkDevice->SlotNum] = 1; //fail this gendisk in initialization
		return -ENODEV ;
	}

	if (MEMSTICK_PRO_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
	{
		WRITE_PRINT("MEMSTICK_PRO_CARD IS PRESENT DURING OPEN: EXITING\n");
		BlkDevice->CardType = MEMSTICK_PRO_CARD;
	}

	if ((int)INVALID_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
	{
		WRITE_PRINT("INVALID_CARD IS PRESENT DURING OPEN : EXITING\n");
		GenDiskOpenStatus[BlkDevice->sdhcNum][BlkDevice->SlotNum] = 1; //fail this gendisk in initialization
		BlkDevice->CardType = INVALID_CARD;
		return -ENODEV;
	}

	if (! BlkDevice->DevUse) //check this out	//will check on 0 usage : first time
	{
		WRITE_PRINT("MediaChange is: %d of slot[%d%d]\n",BlkDevice->MediaChange,BlkDevice->sdhcNum,BlkDevice->SlotNum);
		//no need to call it : will call revalidate with in open (hense got called twice) as
		//i've given media_changed in registration. otherwise calls revalidate after exiting open always
		
		check_disk_change(bdev);

	}
	spin_lock(&BlkDevice->DevLock);
	BlkDevice->DevUse++;
	spin_unlock(&BlkDevice->DevLock);

	EXIT_PRINT();
	return SUCCESS;
}



// This function dumps the bus info
void dump_bus_info(struct sdio_bus_specific_info *busInfo)
{
	WRITE_PRINT("\n========Dumping Bus Info ================\n");
	WRITE_PRINT("clock is              [%d] Hz\n",busInfo->clock);
	WRITE_PRINT("voltage_range is      [%d] \n",busInfo->voltage_range);
	WRITE_PRINT("block_length is       [%d] \n",busInfo->block_length);
	WRITE_PRINT("bus_width is          [%d] \n",busInfo->bus_width);
	WRITE_PRINT("dma_enable_disable is [%d] \n",busInfo->dma_enable_disable);
	WRITE_PRINT("dma_type is           [%d] \n",busInfo->dma_type);
	WRITE_PRINT("========================================\n");
}

void BlkSetupBus(int i, int j)
{
	struct sdio_bus_specific_info bs_info;
	unsigned short val = 0;
	unsigned int ret_api;
	memset(&bs_info,0,sizeof(struct sdio_bus_specific_info));
	EnableDisableAutoCMD(SD_AUTOCMD12,i,j);
	GetBusSpecificInformation(&bs_info,i,j);
	#if defined(ENABLE_MSPRO)
	if(buswidth==1)
	{
		bs_info.clock = SERIAL_MAX_CLOCK;
		bs_info.bus_width =SERIAL_ACCESS;
	}
	else if(buswidth==4)
	{
		 bs_info.clock = PARALLEL4_MAX_CLOCK;
		 bs_info.bus_width =PARALLEL4_ACCESS;
	}
	else if(buswidth==8)
	{

		WRITE_PRINT("Setting buswidth=4 before setting buswidth=8 since the previous bus width was 1\n");
		bs_info.clock = PARALLEL4_MAX_CLOCK;
		bs_info.bus_width =PARALLEL4_ACCESS;
		SetBusSpecificInformation(&bs_info,i,j);

		WRITE_PRINT("Setting buswidth 8\n");
		bs_info.clock = PARALLEL4_MAX_CLOCK;
		bs_info.bus_width =PARALLEL8_ACCESS;

	}
	else
		WRITE_PRINT("Invalid buswidth\n");
	#endif
	
	//enable scattergather
	if(sg)
	{
		printk("Scattergather selected\n");
		bs_info.scattergather = 1;
		bounce = 1;
	}
	else
	{
		
		bs_info.scattergather = 0;
		bounce = 0;
	}

	
	//Setting the Transfer type PIO, DMA, ADMA
	WRITE_PRINT("Setting the Bus Information for slot [%d] with dmaMode [%d]\n",i,dmaMode);
	switch (dmaMode)
	{
		case 0 : //this is the PIO mode
				bs_info.dma_enable_disable = 0;
				bs_info.dma_type = 0;
				max_hw_segs = 128;
				SetBusSpecificInformation(&bs_info,i,j);
				break;
		case 1 : //this is the DMA Mode
				bs_info.dma_enable_disable = 1;
				bs_info.dma_type = 0;
				max_hw_segs = 1;
				SetBusSpecificInformation(&bs_info,i,j);
				break;
		case 2 : //this is the ADMA Mode
				bs_info.dma_enable_disable = 1;
				bs_info.dma_type = 1;
				max_hw_segs = 128;
				SetBusSpecificInformation(&bs_info,i,j);
				break;
		default :
				WRITE_PRINT("Transfer mode is invalid. Setting default to dma mode\n");
				dmaMode = 1;
				bs_info.dma_enable_disable = 1;
				bs_info.dma_type = 0;
				max_hw_segs = 1;
				SetBusSpecificInformation(&bs_info,i,j);
				break;
	}





	memset(&bs_info,0,sizeof(struct sdio_bus_specific_info));
	GetBusSpecificInformation(&bs_info,i,j);
	//dump_bus_info(&bs_info);
	if (SD_TRUE == bs_info.ifcard_present)
	{
		WRITE_PRINT("\nSET_BUS_SPECIFIC_INFORMATION IS SUCCESSFUL\n");
		WRITE_PRINT("SD Bus Width 0x%x \n", bs_info.bus_width);
		WRITE_PRINT("after : dma enable/disable is  0x%x \n", bs_info.dma_enable_disable);
		WRITE_PRINT("after : dma type 0x%x \n", bs_info.dma_type);
		ReadHWRegister(0x28, &val, i,j); // -------> have to fix this in future
		WRITE_PRINT("after : Value of Host Control 0x%x \n", val);
	}
	else
	{
		WRITE_PRINT("Failed to set the Bus Information, will use previous settings\n");
		return;
	}
	//Issuing CMD8 for eMMC cards for user area access
	if (eMMC_CARD == CardType(i,j) || eMMCHC_CARD == CardType(i,j))
	{
		WRITE_PRINT("Going to set Boot Partition for user access\n");
		ret_api = eMMCSetBootPartition(eMMC_NO_BOOT_ACK,eMMC_NO_BOOT_EN ,eMMC_NO_BOOT_ACCESS,i,j);
		if(!ret_api)
		    WRITE_PRINT("Setting Boot Partition for user access\n");
	}
}




int Blkioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long arg)
{
	
  int slotNum = 0,sdhcNum=0, *value;
  int status = -1;
  u32 val;
  static int offset; 
  struct sdio_bus_specific_info *businfo;
  NEX_SD_CSD_V2   *sd_csd;
  businfo = (struct sdio_bus_specific_info*)arg;
  sd_csd = (NEX_SD_CSD_V2 *)arg;

  WRITE_PRINT("IOCTL CMD [%d]..\n",cmd);
  if(cmd == SET_BUS_INFO)
  {
printk("---------------------------------------\n");
printk("Blg sdmmc Checking \n");
printk("SlotNum is            - %d\n", slotNum);
printk("Clock is              - %d Hz\n", businfo->clock);
printk("card version is       - %d Hz\n", businfo->card_ver);
printk("Block_length is       - %d\n", businfo->block_length);
printk("Bus_width is          - %d\n", businfo->bus_width);
printk("DMA_enable_disable is - %d\n", businfo->dma_enable_disable);
printk("DMA_type is           - %d\n", businfo->dma_type);
printk("Card Presence is      - %d\n", businfo->ifcard_present);
//printk("Dual Clock is         - %d\n", businfo->dual_clock);
printk("---------------------------------------\n"); 
     SetBusSpecificInformation(businfo,sdhcNum,slotNum); 
      return SUCCESS;
  }
  else if(cmd == GET_CARD_INFO)
  {
printk("GET CARD INFO IOCTL.. \n");
      memset(businfo,0,(sizeof(struct sdio_bus_specific_info)));
      GetBusSpecificInformation(businfo,sdhcNum,slotNum);  
/*printk("---------------------------------------\n");
printk("Blg sdmmc Checking \n");
printk("SlotNum is            - %d\n", slotNum);
printk("Clock is              - %d Hz\n", businfo->clock);
printk("card version is       - %d Hz\n", businfo->card_ver);
printk("Block_length is       - %d\n", businfo->block_length);
printk("Bus_width is          - %d\n", businfo->bus_width);
printk("DMA_enable_disable is - %d\n", businfo->dma_enable_disable);
printk("DMA_type is           - %d\n", businfo->dma_type);
printk("Card Presence is      - %d\n", businfo->ifcard_present);
//printk("Dual Clock is         - %d\n", businfo->dual_clock);
printk("---------------------------------------\n"); */
      return SUCCESS;
  }
  else if(cmd == CARD_TYPE)
  {
printk("CARD_TYPE IOCTL.. \n");
        value = (int *)arg;
	*value = CardType(sdhcNum,slotNum);
        return SUCCESS;
  }
  else if(cmd == GET_CARD_CSD)
  {
printk("GET_CARD_CSD IOCTL.. \n");
      memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
      GetCardSpecificInfo_UpdatedCSD(sd_csd,sdhcNum,slotNum); 
      printk("BLKIIOCTL: Tran Speed [%x] \n",sd_csd->tran_speed);    
      return SUCCESS;
  }
  else if(cmd == GET_CARD_CAPACITY)
  {
      printk("BLKIIOCTL: CARD CAPACITY IN SECTORS [%x] \n",gl_BlkDevice[sdhcNum][slotNum]->DevSize_in_Sectors); 
      value = (int *)arg; 
      *value  = gl_BlkDevice[sdhcNum][slotNum]->DevSize_in_Sectors;
      return SUCCESS;
  }
  else if(cmd == SET_HS_CARD)
  {
        printk("SET_HS_CARD IOCTL.. \n");
       memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
       status = SetMMCSDHighSpeed(sdhcNum,slotNum);
       if (SUCCESS != status )
       {
           printk("Error : 0x%X\n", status);
           return status;
       }
      // GetCardSpecificInfo_UpdatedCSD(sd_csd,sdhcNum,slotNum);
       //printk(" Tran Speed:%02X  \n",sd_csd->tran_speed);
       return status;
  }
  else if(cmd == SET_FS_CARD)
  {
        
	memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
        status = SetMMCSDFullSpeed(sdhcNum,slotNum);

	memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
        if ( SUCCESS != status )
  	memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
        {
            printk("Error : 0x%X\n", status);
            return status;
        }
	//GetCardSpecificInfo_UpdatedCSD(sd_csd,sdhcNum,slotNum);
	//printk(" Tran Speed:%02X  \n",sd_csd->tran_speed);
        return status;
  }
  else if(cmd == SET_SDR12_CARD)
  {
        
	memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
        status = SDXC_UHS_Setting(sdhcNum,slotNum, // int slotNum, // Ranges in 0 to 1,
                                0x0, // int Current_limit, // Ranges in CL_200MA(0) to CL_800MA(3)
                                0xF, // int drive_strength, // Ranges in TYPE_B(0) to TYPE_D(3)
                                0xF, // int cmd_sys, // Ranges in Default(0) to Adv_Sec_SD(3)
                                0x0);// int access_mode)// Ranges in DDR12(0) to DDR50(4)
        if ( SD_SUCCESS != status )
        {
            printk("Error : 0x%X\n", status);
            return status;
        }
	//GetCardSpecificInfo_UpdatedCSD(sd_csd,sdhcNum,slotNum);
	//printk(" Tran Speed:%02X  \n",sd_csd->tran_speed);
        return status;
  }   
  else if(cmd == SET_SDR25_CARD)
  {
        
	memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
        status = SDXC_UHS_Setting(sdhcNum,slotNum,    // int slotNum,        // Ranges in 0 to 1,
						    0x0,  // int Current_limit,  // Ranges in CL_200MA(0) to CL_800MA(3)
						    0xF,  // int drive_strength, // Ranges in TYPE_B(0)   to TYPE_D(3)
						    0xF,  // int cmd_sys,        // Ranges in Default(0)  to Adv_Sec_SD(3)
						    0x1); // int access_mode)    // Ranges in DDR12(0)    to DDR50(4)
        if ( SD_SUCCESS != status )
        {
            printk("Error : 0x%X\n", status);
            return status;
        }
	//GetCardSpecificInfo_UpdatedCSD(sd_csd,sdhcNum,slotNum);
	//printk(" Tran Speed:%02X  \n",sd_csd->tran_speed);
        return status;
  } 
  else if(cmd == SET_SDR50_CARD)
  {
        
	memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
        status = SDXC_UHS_Setting(sdhcNum,slotNum,    // int slotNum,        // Ranges in 0 to 1,
						    0x0,  // int Current_limit,  // Ranges in CL_200MA(0) to CL_800MA(3)
						    0xF,  // int drive_strength, // Ranges in TYPE_B(0)   to TYPE_D(3)
						    0xF,  // int cmd_sys,        // Ranges in Default(0)  to Adv_Sec_SD(3)
						    0x2); // int access_mode)    // Ranges in DDR12(0)    to DDR50(4)
						   
        if ( SD_SUCCESS != status )
        {
            printk("Error : 0x%X\n", status);
            return status;
        }
	//GetCardSpecificInfo_UpdatedCSD(sd_csd,sdhcNum,slotNum);
	//printk(" Tran Speed:%02X  \n",sd_csd->tran_speed);
        return status;
  } 
 /* else if(cmd == SET_SDR104_CARD)
  {
        
	memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
        status = SDXC_UHS_Setting(sdhcNum,slotNum,    // int slotNum,        // Ranges in 0 to 1,
						    0x0,  // int Current_limit,  // Ranges in CL_200MA(0) to CL_800MA(3)
						    0xF,  // int drive_strength, // Ranges in TYPE_B(0)   to TYPE_D(3)
						    0xF,  // int cmd_sys,        // Ranges in Default(0)  to Adv_Sec_SD(3)
						    0x3); // int access_mode)    // Ranges in DDR12(0)    to DDR50(4)
        if ( SD_SUCCESS != status )
        {
            printk("Error : 0x%X\n", status);
            return status;
        }
	//GetCardSpecificInfo_UpdatedCSD(sd_csd,slotNum);
	//printk(" Tran Speed:%02X  \n",sd_csd->tran_speed);
        return status;
  } */ 
 else if(cmd == SET_DDR50_CARD)
  {
        
	memset(sd_csd,0,(sizeof(NEX_SD_CSD_V2)));
        status = SDXC_UHS_Setting(sdhcNum,slotNum,    // int slotNum,        // Ranges in 0 to 1,
						    0x0,  // int Current_limit,  // Ranges in CL_200MA(0) to CL_800MA(3)
						    0xF,  // int drive_strength, // Ranges in TYPE_B(0)   to TYPE_D(3)
						    0xF,  // int cmd_sys,        // Ranges in Default(0)  to Adv_Sec_SD(3)
						    0x4); // int access_mode)    // Ranges in DDR12(0)    to DDR50(4)
        if ( SD_SUCCESS != status )
        {
            printk("Error : 0x%X\n", status);
            return status;
        }
	//GetCardSpecificInfo_UpdatedCSD(sd_csd,sdhcNum,slotNum);
	//printk(" Tran Speed:%02X  \n",sd_csd->tran_speed);
        return status;
  }  
  else if(cmd == WRITE_REG_ADDRESS)
  {
      offset = arg;
      return SUCCESS;
  }
  else if(cmd == WRITE_REG_VALUE)
  {
      val = arg;
      printk("offset = %x\n",offset);
      printk("value = %x\n",val);
      WriteHWRegister_32(offset,val,sdhcNum,slotNum);
      return SUCCESS;
  }
  else if(cmd == READ_REG_ADDRESS)
  {
      offset = *((int *)arg);
      ReadHWRegister_32(offset,&val,sdhcNum,slotNum);
      *((int *)arg) = val;
      return SUCCESS;
  }
  else
  	{
  		;
  	}
     // printk("Invalid ioctl called\n");

return -ENOTTY;
}
// Name        : Block_Ops
//
// Argument  : None
//
//Returns      : None
//
//Description : This is the standard block device operations sructure
//
static struct block_device_operations Block_Ops = {
.owner           = THIS_MODULE,
.open            = BlkOpen,
.release         = BlkRelease,
.media_changed   = BlkMediaChange,
.revalidate_disk = BlkRevalidateDisk,
.getgeo          = BlkGetGeo,
.ioctl           = Blkioctl
};

// Name        : Initialize_Devices
//
// Argument  : struct BlockDevice *BlkDevice : pointer to current Device that is being initialized
//		     int sdhcNum : Number of the sd host controller being used
//                   int slotNum : Number of the slot being used
//
//Returns      : int : 0 == Success, Else Failure
//
//Description : This is the initialization function that is meant for initialize the device and associate a gendisk with it.
//
static int Initialize_Devices(struct BlockDevice *BlkDevice, int sdhcNum, int slotNum)
{
	struct sdio_bus_specific_info bs_info;
	#if defined(ENABLE_MSPRO)
	unsigned long int Capacity_in_bytes=0;
	#endif
	MSPRO_INFO ms_info;
	unsigned int bouncesz = 65536;
	unsigned int max_sectors;
        static int fminor = TOTAL_MINORS;
	INIT_PRINT();
	memset(&bs_info,0,sizeof(struct sdio_bus_specific_info));
	memset(&ms_info,0, sizeof(MSPRO_INFO));
	memset(&ms_info.mspro_devinfo,0,sizeof(MSPRO_DEVINFO));
	memset(&ms_info.mspro_sys_info,0,sizeof(MSPRO_SYS_INFO));
	WRITE_PRINT("Test Getting Bus Info Before Setting....\n");
	// Initializeing BlockDevice Parameters
	BlkDevice->SlotNum            = slotNum;
	BlkDevice->sdhcNum	      = sdhcNum;
	BlkDevice->DevUse             = 0;
	BlkDevice->MediaChange        = 1;     //0 == media is not changed, yet we need to check media at first go
	#if defined(ENABLE_MSPRO)
 	if (MEMSTICK_PRO_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))
 	{
	 	GetCardAttribute(&ms_info,0);
		sectors_per_track=ms_info.mspro_devinfo.sectors_per_track;
		disk_heads= ms_info.mspro_devinfo.heads;
		WRITE_PRINT("sectors_per_track = %x\n",sectors_per_track);
		WRITE_PRINT("disk_heads = %x\n",disk_heads);
		Capacity_in_bytes =(ms_info.mspro_sys_info.totaluserarea * ms_info.mspro_sys_info.unitsize
								* ms_info.mspro_sys_info.blksize); // capacity in bytes
		WRITE_PRINT("total user area = %x\n",ms_info.mspro_sys_info.totaluserarea);
		WRITE_PRINT("unitsize = %x\n",ms_info.mspro_sys_info.unitsize);
		WRITE_PRINT("blksize = %x\n",ms_info.mspro_sys_info.blksize);

		WRITE_PRINT("Its Capacity is %d GB \n", Capacity_in_bytes/(1024*1024*1024));
		BlkDevice->DevSize_in_Sectors = Capacity_in_bytes/HARDWARE_SECTOR_SIZE;

		WRITE_PRINT("Dev_Size in Sectors [%d]\n",BlkDevice->DevSize_in_Sectors);
	}
	#else
		BlkDevice->DevSize_in_Sectors = 32768; //initial 16 MB capacity to disk
	#endif
	BlkDevice->CardPresent        = 0;     //say card is present
	init_completion(&BlkDevice->cp);
	// Allocating and filling the gendisk structure for this disk
	BlkDevice->gen_disk = alloc_disk(TOTAL_MINORS); //allocating this disk for (here 16) minor partitions
	if (NULL == BlkDevice->gen_disk)
	{
		WRITE_PRINT("Cannot Allocate gendisk\n");
		return -EINVAL;
	}
	//WRITE_PRINT("GENDISK CREATED IS [0x%0x]\n",BlkDevice->gen_disk);
	BlkDevice->gen_disk->major       = BlkMajor;
	BlkDevice->gen_disk->first_minor = fminor; 
        fminor +=TOTAL_MINORS;
	if(total_sdhc == 1)
		snprintf(BlkDevice->gen_disk->disk_name, 32, "BlockDev%d", slotNum); //devices are BlkDev0 BlkDev1 ....
	else
		snprintf(BlkDevice->gen_disk->disk_name, 32, "BlockDev%d%d", sdhcNum,slotNum); //devices are BlkDev00 BlkDev10 ....
	BlkDevice->gen_disk->fops        = &Block_Ops;
	BlkDevice->req = NULL;
	spin_lock_init(&BlkDevice->DevLock); //need a  spinlock before queue creation
	
	BlkDevice->queue = blk_init_queue(REQUEST_FUNCTION, &BlkDevice->DevLock);
	if (NULL == BlkDevice->queue)
	{
		WRITE_PRINT("Queue Allocation Failed \n");
		return -ENODEV;
	}
	
	
	BlkDevice->queue->queuedata = BlkDevice;
	BlkDevice->gen_disk->queue = BlkDevice->queue;
	BlkDevice->gen_disk->flags   |= GENHD_FL_REMOVABLE; //removable media
	
	
	if(sg)
	{
	    if(bounce)
		{
		 	if (max_hw_segs == 1) 
		 	{
				if ((SDHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)) || (SDXC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)) || (MMCHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)) ||
					(eMMCHC_CARD == CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum))||(MEMSTICK_PRO_CARD==CardType(BlkDevice->sdhcNum,BlkDevice->SlotNum)))
			{
				//printk("max sectors = 256\n");
			
				
				max_sectors = 256;
			}
			else
				max_sectors = 128;
	        	WRITE_PRINT("bounce buffer allocation\n");
	               
	            if (bouncesz > 512) 
				{
		        
				
				BlkDevice->bounce_buf = kzalloc(max_sectors * 512, GFP_ATOMIC); 
		            if (!BlkDevice->bounce_buf) 
					{
		                WRITE_PRINT("unable to allocate bounce buffer\n");
						return -ENOMEM;
		            }
	           	}
	 
	            if (BlkDevice->bounce_buf) 
				{
	            	blk_queue_bounce_limit(BlkDevice->queue, BLK_BOUNCE_ANY);
	                blk_queue_max_sectors(BlkDevice->queue, max_sectors);//bouncesz / 512);
	                blk_queue_max_hw_segments(BlkDevice->queue, bouncesz / 512);
	                blk_queue_max_segment_size(BlkDevice->queue, bouncesz);
	 
	                BlkDevice->sg = kzalloc(sizeof(struct scatterlist), GFP_ATOMIC);
	                if (!BlkDevice->sg) 
					{
	                 	BlkDevice->sg = NULL;
						blk_cleanup_queue(BlkDevice->queue);
						return -ENOMEM;
	                }
	                sg_init_table(BlkDevice->sg, 1);
	 
	                BlkDevice->bounce_sg = kzalloc(sizeof(struct scatterlist) * bouncesz / 512, GFP_ATOMIC);
	                if (!BlkDevice->bounce_sg) 
					{
	                	WRITE_PRINT("bounce sg could not be allocated\n"); 
						return -ENOMEM;
	                }
	                sg_init_table(BlkDevice->bounce_sg, bouncesz / 512);
	           	}
		    }

		}

		if (!BlkDevice->bounce_buf)
		{
			bouncesz= 1024;//min(65535,524288 / 512) = min(65535, 1024)
			blk_queue_bounce_limit(BlkDevice->queue, BLK_BOUNCE_HIGH);
			blk_queue_max_sectors(BlkDevice->queue,bouncesz);  
			blk_queue_max_hw_segments(BlkDevice->queue, max_hw_segs);  // 128
			blk_queue_max_segment_size(BlkDevice->queue, 65536);
			
	 		

			BlkDevice->sg = kzalloc(sizeof(struct scatterlist) * max_hw_segs, GFP_ATOMIC);
			if (!BlkDevice->sg) 
			{
				BlkDevice->sg = NULL;
				blk_cleanup_queue(BlkDevice->queue);
				return -ENOMEM;
			}
			
			sg_init_table(BlkDevice->sg, max_hw_segs);
			
		}
		
	}

	WRITE_PRINT("BlkDevice->queue->limits->max_hw_sectors = %d\n",BlkDevice->queue->limits.max_hw_sectors);
	WRITE_PRINT("BlkDevice->queue->limits->max_sectors = %d\n",BlkDevice->queue->limits.max_sectors);
	WRITE_PRINT("BlkDevice->queue->limits->max_segment_size = %d\n",BlkDevice->queue->limits.max_segment_size);
	WRITE_PRINT("BlkDevice->queue->limits->max_segments = %d\n",BlkDevice->queue->limits.max_hw_segments);
	

	set_capacity(BlkDevice->gen_disk,BlkDevice->DevSize_in_Sectors); //setting gd->capacity

	BlkDevice->gen_disk->private_data = BlkDevice;


	sema_init(&BlkDevice->thread_sem, 1);
	BlkDevice->thread = kthread_run(BlkDevice_queue_thread, BlkDevice, "Blkthrd");

	if (IS_ERR(BlkDevice->thread)) {

		WRITE_PRINT("\nThread creation error");
		blk_cleanup_queue(BlkDevice->queue);
		unregister_blkdev(BlkMajor,"BlockDev");
		kfree(gl_BlkDevice);
		return PTR_ERR(BlkDevice->thread);

	}
	// Finally adding this device to system / check init errors before this
	WRITE_PRINT("CALLING **ADD_DISK** FOR SLOT [%d%d]\n",BlkDevice->sdhcNum,BlkDevice->SlotNum);
	add_disk(BlkDevice->gen_disk);

	if(GenDiskOpenStatus[BlkDevice->sdhcNum][BlkDevice->SlotNum])
	{
		//fail this gendisk
		if(BlkDevice->gen_disk)
		{
			del_gendisk(BlkDevice->gen_disk);
			put_disk(BlkDevice->gen_disk);  //check this out //reduces the count
			BlkDevice->gen_disk = NULL;     //just trying
		}
		if(BlkDevice->queue)
		{
			blk_cleanup_queue(BlkDevice->queue);
			BlkDevice->queue = NULL; //just trying
		}

		if(BlkDevice->DevUse)
		{
			BlkDevice->DevUse = 0;
			module_put(THIS_MODULE); //trying to reduce usage count with this
		}
		GenDiskStatus[BlkDevice->sdhcNum][BlkDevice->SlotNum] = 1; //no gendisk now;
	}
	else
	{
		WRITE_PRINT("INITIALIZE : CARD WAS PRESENT DURING INIT SLOT [%d%d]\n",BlkDevice->sdhcNum,BlkDevice->SlotNum);
	}

	EXIT_PRINT();
	return SUCCESS;
}

// Name        : BlkInitialize
//
// Argument  : void
//
//Returns      : int : 0 == Success, Else Failure
//
//Description : This is the initialization function called on module init its job is to register and initialize the device associated
 //                  with this block driver.
//
static int __init BlkInitialize()
{
	int i = 0,j;
	INIT_PRINT();
	// Registering device for major number
	BlkMajor = 	register_blkdev(BlkMajor,"BlockDev");
	if (BlkMajor <= 0)
	{
		WRITE_PRINT("Cannot Register the Block Driver\n");
		unregister_blkdev(BlkMajor,"BlockDev");
		return -EBUSY;
	}

	printk("**         BlockDriver loading Begins           **\n");
	printk("**************************************************\n");

	total_sdhc = Get_NumOfSDHostControllers();
	printk("Number of SD Host Controllers = %d\n",total_sdhc);
	for(i=0;i<total_sdhc;i++)
	{
		
		slotPerSDHC[i] = Get_NumOfSlots(i);
		printk("Number of slots for SDHC %d = %d\n",i,slotPerSDHC[i]);
	}


	/* Allocating memory to total number of devices Allocating all device structures (2 devices here)
	     indivisually assigning parameters to each device structure e.g. size for disk1 and size for disk 2
	 */
	

	// Initialising the devices and allocating gendisk structure to each device
	for(i=0; i<total_sdhc; i++)
	{
		for(j=0;j<slotPerSDHC[i];j++)
		{

			int chk_init= 0;
			int ret_isrcall;

			gl_BlkDevice[i][j] = kzalloc(sizeof(struct BlockDevice), GFP_ATOMIC);
			

			GenDiskOpenStatus[i][j] = 0;
			GenDiskStatus[i][j] = 0;
			// Registering interrupt taskhandler
			ret_isrcall = RegisterISRCallBack(UsrFunctionPtr, i,j);
			if (SD_SUCCESS != ret_isrcall)
			{
				WRITE_PRINT("RegisterISRCallback Failed\n");
				return -1;
			}

		        // Testing : Card in slot is Initialized
		        if(IsCardInitialized(i,j) != SD_TRUE) //no need to put it in loop
		        {
		                WRITE_PRINT("Card Is Not Present in slot [%d] during initialization\n",i);
		                continue;
		        }

			//setting Transfer type ( PIO, DMA, ADMA)  with SetBusSpeceficInfo
			BlkSetupBus(i,j);


			// Initializing all the devices
			chk_init = Initialize_Devices(gl_BlkDevice[i][j], i,j); //passed  address of device and number for node
			
			if (0 != chk_init)
			{
				WRITE_PRINT("Device Initialization Failed\n");
				unregister_blkdev(BlkMajor,"BlockDev");
				kfree(gl_BlkDevice);
				return -1; //don't do any more initialization
			}
		}
	}

	EXIT_PRINT("\n");
	return 0; //to kernel : success full registration and initialisation
}


/*******************************************************************************
Name        	: BlkExit

Argument		: void

Returns      	: void

Description 	: This si the exit function called in module exit this does all the cleanup job for block driver.
********************************************************************************/
static void __exit BlkExit(void)
{
	struct BlockDevice *BlkDevice = NULL;
	int i,j;
	int ret_isrcall;
	

	struct sdio_bus_specific_info bs_info;
	INIT_PRINT();
	memset(&bs_info,0,sizeof(struct sdio_bus_specific_info));

	for(i=0; i<total_sdhc; i++)
	{
		for(j=0;j<slotPerSDHC[i];j++)
		{
#if defined(ENABLE_MSPRO)
		GetBusSpecificInformation(&bs_info,i,j);
		WRITE_PRINT("buswidth=%x\n",bs_info.bus_width);
		WRITE_PRINT("clock=%x\n",bs_info.clock);
		bs_info.clock = SERIAL_MAX_CLOCK;
		bs_info.bus_width =SERIAL_ACCESS;
		bs_info.scattergather = 0;
		SetBusSpecificInformation(&bs_info,i,j);
#else
		GetBusSpecificInformation(&bs_info,i,j);
		bs_info.scattergather = 0;
		SetBusSpecificInformation(&bs_info,i,j);
#endif
		BlkDevice = gl_BlkDevice[i][j];
		
		/*  need to check gendisk is deleted first or the queue is deleted first think : gendisk is deleted first otherwise
		      deleting queue will make null pointer in gendisk and hence crash. queue can be deleted later
		 */
		if(BlkDevice->gen_disk)
		{
			del_gendisk(BlkDevice->gen_disk);
			put_disk(BlkDevice->gen_disk); //check this out //reduces the count
		}
		else
		{
			WRITE_PRINT("THERE IS NO GENDISK FOR SLOT [%d%d]\n",BlkDevice->sdhcNum,BlkDevice->SlotNum);
		}

		// UnRegistering interrupt taskhandler
		if(BlkDevice->thread != NULL)
		{
			kthread_stop(BlkDevice->thread);
			WRITE_PRINT("\nkthread_stop ended");
		}
		
		
		
		if(BlkDevice->queue)
		{
			blk_cleanup_queue(BlkDevice->queue);
			
		BlkDevice->queue = NULL;
		
			
		}
		else
		{
			WRITE_PRINT("THERE IS NO QUEUE FOR SLOT [%d%d]\n",BlkDevice->sdhcNum,BlkDevice->SlotNum);
		}
		

		
		ret_isrcall = RegisterISRCallBack(NULL, i,j);
		if (SD_SUCCESS != ret_isrcall)
		{
			WRITE_PRINT("RegisterISRCallback Failed : Cleanup is not Done \n");
			return;
		}
		 if (BlkDevice->bounce_sg)
                	kfree(BlkDevice->bounce_sg);
        	BlkDevice->bounce_sg = NULL;

		 if (BlkDevice->sg)
               		kfree(BlkDevice->sg);
         	BlkDevice->sg = NULL;

		if (BlkDevice->bounce_buf)
                   kfree(BlkDevice->bounce_buf);
         	BlkDevice->bounce_buf = NULL;
		kfree(gl_BlkDevice[i][j]);
		}
		
	}
	
	BlkDevice = NULL;
	unregister_blkdev(BlkMajor,"BlockDev");
	
	printk("***            BlockDriver Unloaded                   ***\n");
	printk("*********************************************************\n");
	EXIT_PRINT("\n");
}

module_param(dmaMode, uint, 0444);
MODULE_PARM_DESC(dmaMode,	"Definition of DMA Transfer type : PIO, DMA, ADMA");
module_param(sg, uint, 0444);
MODULE_PARM_DESC(sg,	"Scattergather select : 1 - scattergather is used, 0 - not used");
#if defined(ENABLE_MSPRO)
module_param(buswidth, uint, 0444);
MODULE_PARM_DESC(buswidth,	"Buswidth for Mspro card : 1, 4, 8");
#endif

module_init(BlkInitialize);
module_exit(BlkExit);

MODULE_LICENSE("GPL");
