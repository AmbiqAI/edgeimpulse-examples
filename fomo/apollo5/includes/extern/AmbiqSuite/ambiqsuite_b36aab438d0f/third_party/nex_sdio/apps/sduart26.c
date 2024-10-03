//____________________________________________________________________
// Copyright Info     :  All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
//Name                  :  sduart26.c
//
//Creation Date      :  12-April-2007
//
//Description          :  This file contains the code of UART serial driver for Arasan SDIO board
//
//Changed History  :
//<Date>                   <Author>        <Version>        < Description >
//12-April-2007            AG                   0.1                    base code
//____________________________________________________________________
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/wait.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <asm/segment.h>
#include <linux/version.h>
#include <linux/tty_driver.h>  // struct tty_driver
#include <linux/delay.h>       // udelay
#include <asm/uaccess.h>       // copy_to_user copy_from_user get_user put_user
#include <linux/semaphore.h>     // semaphore definitions
#include <linux/delay.h>
#include <linux/types.h>


#include "sduart.h"
#include "nex_api.h"


void EnableFunctionInterrupt(int sdhcNum,int slot);
int command_lock = 0;
int slot = 0,sdhc=0;
struct pid *uart_pid;
// int slot = 1;
//DECLARE_MUTEX(command_sema);

#define SEND_SIG ((struct siginfo *)1)

#if 0 // Complete Log
#define SD_TRACE_INIT(format,args...) printk(KERN_ALERT "=> %s [%d]\n" format,__FUNCTION__,__LINE__,##args)
#define SD_TRACE_EXIT(format,args...) printk(KERN_ALERT "<= %s [%d]\n" format,__FUNCTION__,__LINE__,##args)
#define SD_PRINT(format,args...)      printk(KERN_ALERT "   %s [%d] "  format,__FUNCTION__,__LINE__,##args)
#endif

#if 1 // Limited Log
#define SD_TRACE_INIT(format,args...)
#define SD_TRACE_EXIT(format,args...)
#define SD_PRINT(format,args...) printk(KERN_ALERT "[%s]:[%d]->"format,__FUNCTION__,__LINE__,##args)
#endif

//void debug_write_alpha();
void user_function(SD_CALL_BACK_INFO *a);
void sd_cleanup(void );
void sd_startup(PSD_INTERFACE  );

struct semaphore cleanup_sema;
struct semaphore work_to_do_sema;
int threadid = 0;


#define MINOR_START     128

struct _sdio_port {
    int                 x_char;
    struct async_icount icount;
    struct tty_struct   *tty;
    SD_INTERFACE        *pInterface;
    SD_UINT32           triggerLevel;
    SD_UCHAR            charTimeout;
    SD_UCHAR            phsMode;
    SD_UCHAR            MCR;
    SD_UCHAR            *Trans_buf;
    SD_INT              Trans_head;
    SD_INT              Trans_tail;
    SD_INT              Trans_cnt;
    SD_INT              flags;
    SD_INT              count;
    SD_INT              b_baud;
    SD_INT              baud_rate;
    SD_INT              cus_divisor;
    SD_INT              wk_char;
    SD_INT              block_open;
    wait_queue_head_t   open_wait;
    wait_queue_head_t   close_wait;
    struct termios      normal_termios;
    SD_SEMAPHORE        PortClose;
    SD_SEMAPHORE        PortWrite;
    SD_UINT32           modUse;
    SD_INT              card_present;
    SD_INT              interrupt_type;
    // WORK struct work_struct sdio_card_work;

}sdio_port;

SD_STATIC void sd_Disable_Tx_Ints (void *pContext);
SD_STATIC void sd_Enable_Tx_Ints (void *pContext);
SD_STATIC void sd_Disable_Rx_Ints (void *pContext);
SD_STATIC void sd_Enable_Rx_Ints (void *pContext);
SD_STATIC int  sd_Get_CD(void *pContext);
SD_STATIC int  sd_Set_Termios(void  *pContext);

SD_STATIC SD_INT32 sd_Serial_Open (struct tty_struct *tty, struct file *filp);
SD_STATIC SD_INT32 sd_Serial_Ioctl (struct tty_struct *tty, struct file *filp, unsigned int cmd, unsigned long arg);
SD_STATIC void sd_Serial_Throttle(struct tty_struct * tty);
SD_STATIC void sd_Serial_Unthrottle(struct tty_struct * tty);
SD_STATIC void sd_Send_Xchar (struct tty_struct *tty, char ch);
SD_STATIC void sd_Transmit_Char(struct _sdio_port *pPort);

SD_STATIC SD_INT sd_Serial_Set_BaudRate(SD_INTERFACE *pInterface, SD_ULONG baudRate);
SD_STATIC SD_INT sd_Serial_Set_BitSize(SD_INTERFACE *pInterface, SD_UCHAR bitSize);
SD_STATIC SD_INT32 sd_Serial_Set_Parity(PSD_INTERFACE pInterface, SD_UCHAR parity);
SD_STATIC SD_INT32 sd_Serial_Set_StopBits(PSD_INTERFACE pInterface, SD_UCHAR stopBit);
SD_STATIC SD_INT32 sd_Serial_Set_TriggerLevel (PSD_INTERFACE pInterface, SD_UINT32 triggerLevel);
SD_STATIC SD_INT32 sd_Serial_Set_PHSMode (PSD_INTERFACE pInterface, SD_UINT32   triggerLevel);

SD_STATIC struct tty_driver *sdio_tty_driver;
SD_STATIC SD_INT32 sd_Serial_Initialized = 0;
SD_INT32 sdio_refcount = 0;

void sd_Init_Semaphore (struct semaphore *);

void sd_Release_Semaphore(struct semaphore *);

int sd_Acquire_Semaphore(struct semaphore *);


SD_INT32  sd_Read_Register(PSD_INTERFACE, unsigned int, unsigned char *);
SD_INT32 sd_Write_Register(PSD_INTERFACE, unsigned int, unsigned char);
SD_INT32  sd_Read_Register_Multiple(PSD_INTERFACE,SD_UINT, SD_UCHAR, SD_UINT, SD_UCHAR *);

SD_UINT sd_Enable_Interface(PSD_INTERFACE);
SD_UINT sd_Set_BlockLength(PSD_INTERFACE,SD_UINT);

//void sd_Request_Interrupt(PSD_INTERFACE,void (*uart)(SD_INTERFACE *));
void sd_Request_Interrupt(PSD_INTERFACE,void (*uart)(void *));
void sd_Driver_SetContext(PSD_INTERFACE, struct _sdio_port *);
void sd_Driver_Register(void);
void sd_Driver_Unregister(void);
void debug_dump_registers(void);
void debug_write_alphabet(void);

PSD_INTERFACE sd_init_Interface(void);
struct _sdio_port *sd_Driver_GetContext(PSD_INTERFACE);

/* Place the character in the local Tx Buf */

SD_STATIC int sd_Serial_Put_Char(struct tty_struct  *tty,unsigned char c)

{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();
	
	if(!tty || !pPort->Trans_buf)
		goto RET;
	
	if (pPort->card_present == 0)
		goto RET;

	if(pPort->Trans_cnt >= PAGE_SIZE - 1) 
		goto RET;

	pPort->Trans_buf[pPort->Trans_head++] = c;
	pPort->Trans_head &= PAGE_SIZE - 1;
	pPort->Trans_cnt ++ ;
	SD_TRACE_EXIT();
	
		return 1;
	
RET:
	
		return 0;
	
}



SD_STATIC void sd_Serial_Flush_Chars(struct tty_struct *tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();
	
	/* Sanity check and start transmission */
	if (pPort->Trans_cnt <= 0 || tty->stopped || tty->hw_stopped || !pPort->Trans_buf)
		return;
	
	if (pPort->card_present == 0)
		return ;
	
	sd_Enable_Tx_Ints(pPort);
	SD_TRACE_EXIT();
}

SD_STATIC SD_INT sd_Serial_Write(struct tty_struct *tty, const unsigned char *buf,int count)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_INT cnt,writeLen = 0;
	SD_ULONG flags = 0 ;
	SD_TRACE_INIT();
	
	/* Sanity Check - We really dont want to send anything if the Port is not attached to any SDIO interface */
	if(!tty || !pPort->Trans_buf || !pPort->pInterface) 
		return 0;
	

	if (pPort->card_present == 0)
		return 0;
	

	while (1) 
		{
		local_irq_save(flags);
		cnt = count;

		if((SERIAL_XMIT_SIZE - (pPort->Trans_head)) < cnt ) {
			cnt = SERIAL_XMIT_SIZE - (pPort->Trans_head);
		}

		if((SERIAL_XMIT_SIZE - 1 - pPort->Trans_cnt) <  cnt) {
			cnt = SERIAL_XMIT_SIZE - 1 - (pPort->Trans_cnt);
		}

		if (cnt <= 0) {
			local_irq_restore(flags);
			break;
		}

		memcpy(pPort->Trans_buf + pPort->Trans_head, buf, cnt);

		pPort->Trans_head = (pPort->Trans_head + cnt) & (SERIAL_XMIT_SIZE-1);
		pPort->Trans_cnt += cnt;
		local_irq_restore(flags);

		buf += cnt;
		count -= cnt;
		writeLen += cnt;
	}

	if(!tty->hw_stopped && !tty->stopped && pPort->Trans_cnt) {
		sd_Enable_Tx_Ints(pPort);
	}
	SD_TRACE_EXIT();
	return writeLen;
}

SD_STATIC SD_INT sd_Serial_Write_Room(struct tty_struct *tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_INT space;
	SD_TRACE_INIT();

	if (pPort->card_present == 0)
	{
		return (0);
	}

	space = PAGE_SIZE - pPort->Trans_cnt -1;
	SD_TRACE_EXIT();
	return ((space < 0) ? 0 : space);
}

SD_STATIC SD_INT sd_Serial_Chars_In_Buff(struct tty_struct *tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();
	if (pPort->pInterface == NULL) {
		return (0);
	}

	if (pPort->card_present == 0)
	{
		return (0);
	}

	SD_TRACE_EXIT();
	return (pPort->Trans_cnt);
}

SD_STATIC void sd_Serial_Flush_Buffer(struct tty_struct *tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_ULONG flags;
	SD_TRACE_INIT();
	if(!tty) {
		return;
	}
	if (pPort->card_present == 0)
	{
		return;
	}

	local_irq_save(flags);
	pPort->Trans_head = 0;
	pPort->Trans_tail = 0;
	pPort->Trans_cnt  = 0;
	local_irq_restore(flags);
	wake_up_interruptible(&tty->write_wait);

	tty_wakeup(tty);
	SD_TRACE_EXIT();
}

SD_VOID sd_Serial_Stop(struct tty_struct *tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();

	RegisterISRCallBack(NULL,sdhc, slot);
	// WORK cancel_delayed_work(&pPort->sdio_card_work);

	if(!tty) {
		return ;
	}

	if(pPort->Trans_cnt && pPort->Trans_buf) {
		sd_Disable_Tx_Ints(pPort);
	}
	SD_TRACE_EXIT();
}

SD_VOID sd_Serial_Start(struct tty_struct *tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();

	printk("sd_Serial_Start has been called ....\n");
	if(!tty) {
		return ;
	}

	if(pPort->Trans_cnt && pPort->Trans_buf) {
		sd_Enable_Tx_Ints(pPort);
	}
	mdelay(1000);
	RegisterISRCallBack(user_function,sdhc, slot);
	EnableFunctionInterrupt(sdhc,slot);
	SD_TRACE_EXIT();
}

SD_VOID sd_Serial_Hangup(struct tty_struct *tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_ULONG flags;
	SD_TRACE_INIT();
	if (!tty || !(pPort->flags & ASYNC_INITIALIZED)){
		return;
	}

	if (pPort->card_present == 0)
	{
		return;
	}

	local_irq_save(flags);

	if (pPort->Trans_buf){
		free_page((unsigned long) pPort->Trans_buf);
		pPort->Trans_buf = NULL;
	}

	if(pPort->tty) {
		set_bit(TTY_IO_ERROR, &pPort->tty->flags);
	}

	pPort->flags &= ~ASYNC_INITIALIZED;
	local_irq_restore(flags);

	pPort->flags &= ~(ASYNC_NORMAL_ACTIVE);
	pPort->tty = NULL;
	pPort->count = 0;
	wake_up_interruptible(&pPort->open_wait);
	SD_TRACE_EXIT();
}

SD_STATIC SD_UINT    sd_baudrates[] = {
    0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800,
    9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600
};

SD_VOID sd_Serial_Set_Termios(struct tty_struct *tty,
        struct ktermios * old_termios)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_INT b_rate,wakeChar,retVal;
	struct ktermios *tmpios;
	SD_TRACE_INIT();
	if(!tty) {
		SD_TRACE_EXIT();
		return;
	}

	if (pPort->card_present == 0)
	{
		return;
	}

	tmpios = tty->termios;

	b_rate = tmpios->c_cflag & CBAUD;

	if(b_rate & CBAUDEX){
		b_rate &= ~CBAUDEX;
		if((b_rate < 1)|| (b_rate > 4)) {
			tmpios->c_cflag &= ~CBAUDEX;
		}else{
			b_rate += 15;
		}
	}

	b_rate = sd_baudrates[b_rate];

	if ((tmpios->c_cflag & CBAUD) == B38400) {
		if ((pPort->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI) {
			b_rate = 57600;
		}
		else if ((pPort->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI){
			b_rate = 115200;
		}
		else if ((pPort->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI) {
			b_rate = 230400;
		}
		else if ((pPort->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP) {
			b_rate = 460800;
		}
		else if ((pPort->flags & ASYNC_SPD_MASK) == ASYNC_SPD_CUST) {
			b_rate = (pPort->b_baud / pPort->cus_divisor);
		}
	}

	pPort->baud_rate = b_rate;

	wakeChar = (b_rate / 10 / HZ) * 2;

	if (wakeChar < 0) {
		wakeChar = 0;
	}

	if (wakeChar >= SERIAL_XMIT_SIZE) {
		wakeChar  = SERIAL_XMIT_SIZE-1;
	}

	pPort->wk_char = wakeChar;

	retVal = sd_Set_Termios(pPort);

	if(retVal < 0) {
		SD_TRACE_EXIT();
		return;
	}

	if ((!old_termios || (old_termios->c_cflag & CRTSCTS)) && !(tmpios->c_cflag & CRTSCTS))
	{
		tty->stopped = 0;
		sd_Serial_Start(tty);
	}
	SD_TRACE_EXIT();
	return;
}

SD_INT sd_Block_Til_Ready(struct _sdio_port *pPort,struct file *filp)
{
	struct tty_struct *tty;
	SD_INT val;
	DECLARE_WAITQUEUE(wait, current);
	tty = pPort->tty;
	SD_TRACE_INIT();
	if(!tty){
	SD_TRACE_EXIT();
		return 0;
	}

	if (tty_hung_up_p(filp) || pPort->flags & ASYNC_CLOSING)
	{
		interruptible_sleep_on(&pPort->close_wait);
		if (pPort->flags & ASYNC_HUP_NOTIFY)
		{
	SD_TRACE_EXIT();
			return -EAGAIN;
		}
		else
		{
	SD_TRACE_EXIT();
			return -ERESTARTSYS;
		}
	}

	if(((filp->f_flags & O_NONBLOCK)) || (tty->flags & (1 << TTY_IO_ERROR)))
	{
		pPort->flags |= ASYNC_NORMAL_ACTIVE;
	SD_TRACE_EXIT();
		return 0;
	}

	add_wait_queue(&pPort->open_wait, &wait);
	if(!tty_hung_up_p(filp)) {
		pPort->count --;
	}
	pPort->block_open++;

	SD_PRINT("Entering For  loop for WAIT QUEUE \n");
	for(;;){
		val = sd_Get_CD(NULL);
		set_current_state(TASK_INTERRUPTIBLE);
		if((tty_hung_up_p(filp)) || !(pPort->flags & ASYNC_INITIALIZED)) {
			if(pPort->flags & ASYNC_HUP_NOTIFY){
				val = -EAGAIN;
			}else{
				val = -ERESTARTSYS;
			}
			break;
		}
		if(signal_pending(current)){
			val = -ERESTARTSYS;
			break;
		}
		schedule();
	}

	SD_PRINT("Woken up after WAIT QUEUE \n");
	set_current_state(TASK_RUNNING);
	remove_wait_queue(&pPort->open_wait,&wait);

	if(!tty_hung_up_p(filp)) {
		pPort->count++;
	}
	pPort->block_open--;

	if(val){
		return val;
	SD_TRACE_EXIT();
	}

	pPort->flags |= ASYNC_NORMAL_ACTIVE;
	SD_TRACE_EXIT();
	return 0;
}

SD_STATIC void sd_Hungup(void  *pContext)
{
	struct _sdio_port *pPort = (struct _sdio_port *)pContext;
	SD_TRACE_INIT();
	if (pPort->modUse) {
		pPort->modUse--;
		module_put(THIS_MODULE);
	}
	SD_TRACE_EXIT();
}

SD_VOID sd_Serial_Close(struct tty_struct *tty,struct file *filp)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();
	if(!tty) {
		return;
	}

	if (pPort->card_present == 0)
	{
		return;
	}

	SD_TRACE_INIT("sd_Serial_Close Called ...\n");
	RegisterISRCallBack(NULL, sdhc,slot);
	// WORK cancel_delayed_work(&pPort->sdio_card_work);

	if(!pPort->pInterface) {
		printk("ERROR: CLOSE NOT COMPLETED ....\n");
		return;
	}

	if(tty_hung_up_p(filp)) {
		sd_Hungup((void *)pPort);
		return;
	}

	if ((test_bit(1,(SD_ULONG *)&tty->count) == 1) && (pPort->count != 1)) {
		pPort->count = 1;
	}

	if(--pPort->count < 0) {
		pPort->count = 0;
	}

	if(pPort->count) {
		SD_PRINT("Going for Sem Release ...\n");
		sd_Release_Semaphore(&pPort->PortClose);
		return;
	}

	pPort->flags |= ASYNC_CLOSING;

	tty->closing = 1;
	sd_Disable_Rx_Ints(pPort);

//to check uncommented

/*
	if (tty->driver->flush_buffer)
		tty->driver->flush_buffer(tty);
*/
	tty_ldisc_flush(tty);
	tty->closing = 0;

	if (pPort->modUse) {
		pPort->modUse--;
		SD_PRINT("Module put %d \n", pPort->modUse);
		module_put(THIS_MODULE);
	}

	pPort->tty = 0;

	if(pPort->block_open)
	{
		wake_up_interruptible(&pPort->open_wait);
	}
	pPort->flags &= ~(ASYNC_NORMAL_ACTIVE|ASYNC_CLOSING | ASYNC_INITIALIZED);
	wake_up_interruptible(&pPort->close_wait);
	SD_TRACE_EXIT();
}

SD_INT sd_Get_Serial(struct _sdio_port *pPort,struct serial_struct *ss)
{
	struct serial_struct srStruct;
	SD_TRACE_INIT();
	memset(&srStruct, 0, sizeof(struct serial_struct));

	srStruct.flags = pPort->flags;
	srStruct.baud_base = pPort->b_baud;
	srStruct.custom_divisor = pPort->cus_divisor;

	srStruct.type = PORT_UNKNOWN;
	srStruct.xmit_fifo_size = -1;
	srStruct.line = -1;
	srStruct.port = -1;

	if(copy_to_user(ss,&srStruct,sizeof(struct serial_struct))) {
		return -EFAULT;
	}
	return 0;
}

SD_INT sd_Set_Serial(struct _sdio_port *pPort,struct serial_struct *ss)
{
	struct serial_struct srStruct;
	SD_TRACE_INIT();
	if (copy_from_user(&srStruct,ss,sizeof(struct serial_struct))){
		return(-EFAULT);
	}

	if (!capable(CAP_SYS_ADMIN))
	{
		if ((srStruct.baud_base != pPort->b_baud) || ((srStruct.flags & ~ASYNC_USR_MASK) != (pPort->flags & ~ASYNC_USR_MASK)))
		{
			return(-EPERM);
		}
	}

	pPort->flags        = (pPort->flags & ~ASYNC_USR_MASK) | (srStruct.flags & ASYNC_USR_MASK);
	pPort->b_baud       = srStruct.baud_base;
	pPort->cus_divisor  = srStruct.custom_divisor;

	sd_Serial_Set_Termios (pPort->tty, NULL);
	SD_TRACE_EXIT();
	return 0;
}

SD_STATIC void sd_Disable_Tx_Ints (void *pContext)
{
	struct _sdio_port *pPort = (struct _sdio_port *)pContext;
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_UCHAR intEnable = 0;
	SD_TRACE_INIT();
	if (pInterface == NULL) {
		return;
	}
	sd_Read_Register (pInterface, SDUART_INTR_ENABLE_REG, &intEnable);
	intEnable &= ~IER_ENABLE_TRANS_HOLD;
	sd_Write_Register(pInterface, SDUART_INTR_ENABLE_REG, intEnable);
	SD_TRACE_EXIT();
}

SD_STATIC void sd_Enable_Tx_Ints (void *pContext)
{
	struct _sdio_port *pPort = (struct _sdio_port *)pContext;
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_UCHAR intEnable = 0;
	SD_TRACE_INIT();
	if (pInterface == NULL) {
		return;
	}
	sd_Transmit_Char(pPort);
	sd_Read_Register (pInterface, SDUART_INTR_ENABLE_REG, &intEnable);
	intEnable |= IER_ENABLE_TRANS_HOLD;
	sd_Write_Register(pInterface, SDUART_INTR_ENABLE_REG, intEnable);
	SD_TRACE_EXIT();
}

SD_STATIC void sd_Disable_Rx_Ints (void *pContext)
{
	struct _sdio_port *pPort = (struct _sdio_port *)pContext;
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_UCHAR intEnable = 0;
	SD_TRACE_INIT();
	if (pInterface == NULL) {
		return;
	}
	sd_Read_Register (pInterface, SDUART_INTR_ENABLE_REG, &intEnable);
	intEnable &= ~(IER_ENABLE_RECEIVE_DATA | IER_ENABLE_MODEM_STATUS | IER_ENABLE_LINE_STATUS);
	sd_Write_Register(pInterface, SDUART_INTR_ENABLE_REG, intEnable);
	SD_TRACE_EXIT();
}

SD_STATIC void sd_Enable_Rx_Ints (void *pContext)
{
	struct _sdio_port *pPort = (struct _sdio_port *)pContext;
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_UCHAR intEnable = 0;
	SD_TRACE_INIT();
	if (pInterface == NULL) {
		return;
	}
	sd_Read_Register (pInterface, SDUART_INTR_ENABLE_REG, &intEnable);
	intEnable |= (IER_ENABLE_RECEIVE_DATA | IER_ENABLE_MODEM_STATUS | IER_ENABLE_LINE_STATUS);
	sd_Write_Register(pInterface, SDUART_INTR_ENABLE_REG, intEnable);
	SD_TRACE_EXIT();
}

SD_STATIC int  sd_Get_CD(void *pContext)
{
	SD_TRACE_INIT();
	return (1);
	SD_TRACE_EXIT();
}

SD_STATIC SD_INT32 sd_Set_Termios (void *pContext)
{
	struct _sdio_port *pPort = (struct _sdio_port *)pContext;
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_UINT32 baudRate;
	SD_UCHAR  bitSize = LCR_CHAR_LEN_8BITS;
	SD_UCHAR  Parity  = 0;
	SD_UINT32 cflag;
	SD_TRACE_INIT();
	if (pInterface == NULL) {
		return (0);
	}
	if (!pPort->tty || !pPort->tty->termios) {
		return (0);
	}

	baudRate = pPort->baud_rate;
	cflag = pPort->tty->termios->c_cflag;

	if (baudRate >= 1200 && baudRate <= 115200) {
		sd_Serial_Set_BaudRate (pInterface, baudRate);
	}

	switch (cflag & CSIZE) {
		case CS5:
			bitSize = LCR_CHAR_LEN_5BITS;
			break;
		case CS6:
			bitSize = LCR_CHAR_LEN_6BITS;
			break;
		case CS7:
			bitSize = LCR_CHAR_LEN_7BITS;
			break;
		case CS8:
		default:
			bitSize = LCR_CHAR_LEN_8BITS;
			break;
	}
	sd_Serial_Set_BitSize (pInterface, bitSize);

	if (cflag & CSTOPB) {
		sd_Serial_Set_StopBits(pInterface, LCR_SET_2_STOP_BIT);
	} else {
		sd_Serial_Set_StopBits(pInterface, LCR_SET_1_STOP_BIT);
	}
	if (cflag & PARENB) {
		Parity |= LCR_SET_PARITY_ENABLE;
	}
	if (!(cflag & PARODD)) {
		Parity |= LCR_SET_EVEN_PARITY;
	}
	sd_Serial_Set_Parity(pInterface, Parity);
	sd_Serial_Set_TriggerLevel(pInterface, sdio_port.triggerLevel);
	SD_TRACE_EXIT();
	return (0);
}

SD_STATIC SD_INT sd_Serial_Set_BaudRate (SD_INTERFACE *pInterface,
        SD_ULONG baudRate)
{
	SD_UINT16 divValue = 0;
	SD_UCHAR  lsb      = 0;
	SD_UCHAR  msb      = 0;
	SD_UCHAR  lcrValue = 0;

	SD_TRACE_INIT();
	if(BOARD_CLK_FREQ == 18432) {
		divValue = 115200 / baudRate;
	}
	if(BOARD_CLK_FREQ == 147456) {
		divValue = 921600 / baudRate;
	}
	lsb = divValue & 0x00FF;
	msb = (divValue & 0xFF00) >> 8;

	//debug_dump_registers();

	sd_Read_Register (pInterface, SDUART_LINE_CONTROL_REG, &lcrValue);
	lcrValue |= LCR_SET_DLAB_BIT;

	sd_Write_Register (pInterface, SDUART_LINE_CONTROL_REG,lcrValue);
	sd_Write_Register (pInterface, SDUART_DIVISOR_LATCH_LS_REG, lsb);
	sd_Write_Register (pInterface, SDUART_DIVISOR_LATCH_MS_REG, msb);

	/* disabling DLAB Bit */
	lcrValue &= LCR_RESET_DLAB_BIT;
	sd_Write_Register (pInterface, SDUART_LINE_CONTROL_REG, lcrValue);
	SD_TRACE_EXIT();
	return 0;
}

/*
 * To set bitSize use Proper Macros
 */
SD_STATIC SD_INT sd_Serial_Set_BitSize (SD_INTERFACE *pInterface,
        SD_UCHAR bitSize)
{
	SD_UCHAR lcrValue = 0;
	SD_TRACE_INIT();
	sd_Read_Register (pInterface, SDUART_LINE_CONTROL_REG, &lcrValue);
	lcrValue &= ~LCR_CHAR_LEN_MASK;

	if(bitSize == LCR_CHAR_LEN_5BITS ||
			bitSize == LCR_CHAR_LEN_6BITS ||
			bitSize == LCR_CHAR_LEN_7BITS ||
			bitSize == LCR_CHAR_LEN_8BITS)
	{
		lcrValue |= bitSize;
		sd_Write_Register (pInterface, SDUART_LINE_CONTROL_REG, lcrValue);
		sd_Read_Register (pInterface, SDUART_LINE_CONTROL_REG, &lcrValue);
	}else {
		sd_Error("BitSize Undefined !! \n");
		return -1;
	}
	SD_TRACE_EXIT();
	return 0;
}

/*
 * Transmit a character
 */
SD_STATIC void sd_Transmit_Char(struct _sdio_port *pPort)
{
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_UCHAR lineStatus;
	SD_INT32 Status = SD_SUCCESS;
	int i;
	SD_TRACE_INIT();
	if (!(pPort && pPort->pInterface)) {
		return;
	}

	if (pPort->card_present == 0)
	{
		return;
	}

	//mdelay(500);
	sd_Acquire_Semaphore(&pPort->PortWrite);
	/* TX while bytes available */
	//printk("sd_Transmit_Char...\n");
	for (i = 0; ((i < 100) && (Status == SD_SUCCESS)); i++)
	{
		if (pPort->pInterface == NULL) {
			sd_Release_Semaphore(&pPort->PortWrite);
			return;
		}
		sd_Read_Register (pInterface, SDUART_LINE_STATUS_REG, &lineStatus);
		if (!(lineStatus & LSR_GET_THR_EMPTY )) {
			break;
		}
		if (pPort->x_char) {
			SD_PRINT("VALUE OF X_CHAR IS 0x%x\n",pPort->x_char);
			Status = sd_Write_Register (pInterface, SDUART_TRANS_HOLD_REG,
					pPort->x_char);
			pPort->icount.tx++;
			pPort->x_char = 0;
		} else if (pPort->Trans_cnt <= 0 || pPort->tty->stopped ||
				pPort->tty->hw_stopped)
		{
			break;
		} else {
			//printk("Value of char Written/Transmitted %c \n", pPort->Trans_buf[pPort->Trans_tail]);
			Status = sd_Write_Register (pInterface, SDUART_TRANS_HOLD_REG,
					pPort->Trans_buf[pPort->Trans_tail++]);
			pPort->icount.tx++;
			pPort->Trans_tail &= SERIAL_XMIT_SIZE-1;
			if (--pPort->Trans_cnt <= 0) {
				break;
			}
		}
		udelay(10);
	}

	if (pPort->Trans_cnt <= 0 || pPort->tty->stopped ||
			pPort->tty->hw_stopped) {
		//printk("Disable Tx Ints \n");
		sd_Disable_Tx_Ints(pPort);
	}

	if (pPort->Trans_cnt <= pPort->wk_char) {
		/*  if ((pPort->tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
				pPort->tty->ldisc.write_wakeup) {
				(pPort->tty->ldisc.write_wakeup)(pPort->tty);
				}*/
		tty_wakeup(pPort->tty);
		wake_up_interruptible(&pPort->tty->write_wait);
	}
	sd_Release_Semaphore(&pPort->PortWrite);
	SD_TRACE_EXIT();
}

/*
 * Receive a character
 */
SD_STATIC void sd_Receive_Char (struct _sdio_port *pPort)
{
	struct tty_struct *tty = pPort->tty;
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_UCHAR rxCount = 1;
	SD_UCHAR rxBuf[256];
	SD_INT32 Status = SD_SUCCESS;
	SD_UINT32 i;
	SD_TRACE_INIT();
	if (pInterface == NULL) {
		return;
	}

	if (pPort->card_present == 0)
	{
		return;
	}

	if (pPort->charTimeout) {
		if (pPort->phsMode) {
			Status = sd_Read_Register (pInterface, SDUART_RECEIVE_COUNT_REG, &rxCount);
			if (Status != SD_SUCCESS) {
				rxCount = 1;
			}
			pPort->charTimeout = 0;
		} else {
			rxCount = 1;
		}
	} else {
		rxCount = pPort->triggerLevel;
	}

	if (rxCount == 1) {
		Status = sd_Read_Register (pInterface, SDUART_RECEIVE_REG, rxBuf);
		SD_PRINT("SINGLE :%c\n",*rxBuf);
		//printk("SINGLE :%c\n",*rxBuf);
	} else {
#if 1
		for (i = 0; ((i < rxCount) && (Status == SD_SUCCESS)); i++) {
			Status = sd_Read_Register (pInterface, SDUART_RECEIVE_REG, &rxBuf[i]);
			//printk(KERN_ALERT "Multiple %c\n",rxBuf[i]);
			SD_PRINT("Multiple %c\n",rxBuf[i]);
		}
#else
		Status = sd_Read_Register_Multiple (pInterface, SDUART_RECEIVE_REG,
				rxCount, 0, rxBuf);
#endif
}

/*********************
//intel : flip buffer code
if (Status == SD_SUCCESS)
{
#if 0
	for (i = 0; i < rxCount; i++)
	{
		if (tty->buf.tail->size >= TTY_FLIPBUF_SIZE)
		{
		// if (tty->flip.count >= TTY_FLIPBUF_SIZE)  //flip buffer changed in 2.6.x
		//	tty_flip_buffer_push (tty);
		}
#endif
		//SD_PRINT("Character Inserted %c \n", rxBuf[i]);
		//tty_insert_flip_char(tty, rxBuf[i], 0);
		SD_PRINT("Character Inserted %c \n", *rxBuf);
		tty_insert_flip_char(tty, *rxBuf, 0);
	}
	tty_flip_buffer_push(tty);
}
 ************************/

if (Status == SD_SUCCESS)
{
	for (i = 0; i < rxCount; i++)
	{
		//printk("Character Inserted %c \n", rxBuf[i]);
		SD_PRINT("Character Inserted %c \n", rxBuf[i]);
		tty_insert_flip_char(tty, rxBuf[i], 0);
	}
	tty_flip_buffer_push(tty);
}

SD_TRACE_EXIT();
}

// SDIO Interrupt handler - Run in thread context - Can sleep
// void UART_Interrupt_Handler(SD_INTERFACE *pInterface)
void UART_Interrupt_Handler(void *arg)
{
	SD_INTERFACE *pInterface = (SD_INTERFACE *)arg;
	SD_UCHAR rdIntr    = 0, whichIntr;
	SD_UCHAR lsr;
	SD_INT32 Status = SD_SUCCESS;
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();
	sd_Acquire_Semaphore(&pPort->PortClose);
	Status = sd_Read_Register (pInterface, SDUART_INTR_IDENT_REG, &rdIntr);
	//setDebugLevel(0);
	if (Status != SD_SUCCESS) {
		sd_Release_Semaphore(&pPort->PortClose);
		//printk("Enable the function INTR ..\n");
		EnableFunctionInterrupt(sdhc,slot);
		//setDebugLevel(0);
		SD_TRACE_EXIT();
		return;
	}

	if (!(pPort->flags & ASYNC_INITIALIZED)) {
		/* Port is not open - Clear all pending interrupts */

		//printk("Port is not open ...\n");
		sd_Read_Register (pInterface, SDUART_INTR_IDENT_REG, &rdIntr);
		sd_Read_Register (pInterface, SDUART_LINE_STATUS_REG, &rdIntr);
		sd_Read_Register (pInterface, SDUART_RECEIVE_REG, &rdIntr);
		sd_Read_Register (pInterface, SDUART_MODEM_STATUS_REG, &rdIntr);

		sd_Release_Semaphore(&pPort->PortClose);
		//setDebugLevel(0);
		EnableFunctionInterrupt(sdhc,slot);
		//printk("Enable the function INTR (2)..\n");
		SD_TRACE_EXIT();
		return;
	}

	//printk("SD: Check for rdIntr Flag 0x%x \n", rdIntr);
	if(!(rdIntr & IIR_INTR_PENDING)) { // If Interrupt Pending
		whichIntr = (rdIntr >> 1 & 0x07);

		// Re enable the Function Interrupts.

		//printk("SD: UART Interrupt Raised 0x%x \n", whichIntr);
		switch (whichIntr) {
			case 3: // Rx Line Status
				sd_Read_Register (pInterface, SDUART_LINE_STATUS_REG, &lsr);
				if (lsr & LSR_GET_OVERRUN_ERROR) {
					pPort->icount.overrun++;
				}
				if (lsr & LSR_GET_PARITY_ERROR) {
					pPort->icount.parity++;
				}
				if (lsr & LSR_GET_FRAMING_ERROR) {
					pPort->icount.frame++;
				}
				if (lsr & LSR_GET_BREAK_INTERRUPT) {
					pPort->icount.brk++;
				}
				sd_Read_Register (pInterface, SDUART_RECEIVE_REG, &lsr);
				break;
			case 6: // Character timeout
				pPort->charTimeout = 1;
			case 2: // Receive Data
				//printk("Going for Receive Char...\n");
				sd_Receive_Char (pPort);
				break;
			case 1: // Transmit Hold empty
				//printk("Going for Transmit Char...\n");
				sd_Transmit_Char (pPort);
				break;
			case 0: // Modem status
				//printk("Reading Modem Register in INT Handler ..\n");
				sd_Read_Register (pInterface, SDUART_MODEM_STATUS_REG, &lsr);
				break;
		}
	}
	EnableFunctionInterrupt(sdhc,slot);
	sd_Release_Semaphore(&pPort->PortClose);

	SD_TRACE_EXIT();
}

/*
 * sd_Serial_Startup
 *
 * Called during the serial device init - Set default values for the serial
 * device
 */
SD_STATIC SD_INT32 sd_Serial_Startup (struct _sdio_port *pPort)
{
	SD_UCHAR rdVal = 0;
	PSD_INTERFACE pInterface = pPort->pInterface;

	SD_TRACE_INIT();
	if (pInterface == NULL) {
		return (SD_ERROR_NODEV);
	}

	SD_PRINT("*GOT AT BAUD RATE* \n");

	/* Set Baud Rate */
	sd_Serial_Set_BaudRate(pInterface, 57600);

	/* Set Bit Size */
	sd_Serial_Set_BitSize(pInterface, LCR_CHAR_LEN_8BITS);
	sd_Serial_Set_TriggerLevel (pInterface, pPort->triggerLevel);
	mdelay(5);

	sd_Read_Register (pInterface, SDUART_LINE_STATUS_REG, &rdVal);
	sd_Read_Register (pInterface, SDUART_MODEM_STATUS_REG, &rdVal);
	sd_Read_Register (pInterface, SDUART_INTR_IDENT_REG, &rdVal);

	sd_Write_Register(pInterface, SDUART_INTR_ENABLE_REG, 0x0F);

	/* Modem Control and Loop Back */
	sd_Write_Register(pInterface, SDUART_MODEM_CONTROL_REG, 0x03);
	pPort->MCR = 0x03;

	sd_Serial_Set_PHSMode (pInterface, pPort->triggerLevel);
	mdelay(5);
	/* Request Card Interrupts */
	sd_Request_Interrupt(pInterface, &UART_Interrupt_Handler);
	EnableFunctionInterrupt(sdhc,slot);
	SD_TRACE_EXIT();
	return (SD_SUCCESS);
}

SD_STATIC SD_INT32 sd_Serial_Set_PHSMode (PSD_INTERFACE pInterface, SD_UINT32   triggerLevel)
{
	SD_UCHAR phsMode;
	SD_TRACE_INIT();
	if (triggerLevel >= 32) {
		phsMode = EXTENDED_FIFO_ENABLE;
	} else {
		phsMode = EXTENDED_FIFO_DISABLE;
	}
	SD_TRACE_EXIT();
	return (sd_Write_Register(pInterface, SDUART_EXT_FIFO_ENABLE_REG, phsMode));
}

SD_STATIC SD_INT32 sd_Serial_Set_TriggerLevel (PSD_INTERFACE pInterface, SD_UINT32 triggerLevel)
{
	SD_UCHAR FCR_mode = 0x0F;
	SD_UCHAR FCR_triggerlevel=0x80;
	SD_TRACE_INIT();
	switch(triggerLevel)
	{
		case 1:
		case 32:
			FCR_triggerlevel= 0x00;
			break;
		case 4:
		case 64:
			FCR_triggerlevel= 0x40;
			break;
		case 8:
		case 128:
			FCR_triggerlevel= 0x80;
			break;
		case 14:
		case 192:
			FCR_triggerlevel= 0xC0;
			break;
	}
	SD_TRACE_EXIT();
	return (sd_Write_Register (pInterface, SDUART_FIFO_CONTROL_REG, (FCR_mode |FCR_triggerlevel)));
}

SD_STATIC SD_INT32 sd_Serial_Set_StopBits(PSD_INTERFACE pInterface, SD_UCHAR stopBit)
{
	SD_UCHAR      lcrValue = 0;
	SD_TRACE_INIT();
	sd_Read_Register(pInterface,SDUART_LINE_CONTROL_REG, &lcrValue);
	lcrValue &= ~LCR_STOP_BIT_MASK;

	if(stopBit == LCR_SET_1_5_STOP_BIT) {
		lcrValue |= LCR_SET_1_5_STOP_BIT;
	} else if(stopBit == LCR_SET_2_STOP_BIT) {
		lcrValue |= LCR_SET_2_STOP_BIT;
	} else if(stopBit == LCR_SET_1_STOP_BIT) {
		lcrValue |= LCR_SET_1_STOP_BIT;
	} else {
		sd_Error("Wrong Stop Bit Definition !! \n");
		return -1;
	}
	sd_Write_Register(pInterface,SDUART_LINE_CONTROL_REG,lcrValue);
	SD_TRACE_EXIT();
	return 0;
}

SD_STATIC SD_INT32 sd_Serial_Set_Parity(PSD_INTERFACE pInterface, SD_UCHAR Parity)
{
	SD_UCHAR lcrValue = 0;
	SD_TRACE_INIT();
	sd_Read_Register (pInterface, SDUART_LINE_CONTROL_REG, &lcrValue);
	lcrValue &= ~LCR_PARITY_MASK;
	lcrValue |= Parity;
	sd_Write_Register(pInterface, SDUART_LINE_CONTROL_REG, lcrValue);
	SD_TRACE_EXIT();
	return 0;
}

/*
 * sd_Serial_Set_Modem
 *
 * Configure modem control signals
 */
SD_STATIC SD_INT sd_Serial_Set_Modem(struct tty_struct *tty,struct file *file,
                                          unsigned int set,unsigned int clear)
{
	struct _sdio_port *pPort = tty->driver_data;
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_TRACE_INIT();
	if(pInterface == NULL) {
		return -ENODEV;
	}

	if (pPort->card_present == 0)
	{
		return -ENODEV;
	}
	if(set & TIOCM_RTS)	{
		pPort->MCR |= MCR_SET_RTS_CONTROL;
	}
	if(set & TIOCM_DTR)	{
		pPort->MCR |= MCR_SET_DTR_CONTROL;
	}
	if(clear & TIOCM_RTS){
		pPort->MCR &= ~MCR_SET_RTS_CONTROL;
	}
	if(clear & TIOCM_DTR){
		pPort->MCR &= ~MCR_SET_DTR_CONTROL;
	}

	sd_Write_Register(pInterface, SDUART_MODEM_CONTROL_REG, pPort->MCR);
	SD_TRACE_EXIT();
	return 0;
}

SD_STATIC SD_INT sd_Serial_Get_Modem(struct tty_struct *tty,struct file *file)
{
	struct _sdio_port *pPort = tty->driver_data;
	PSD_INTERFACE pInterface = pPort->pInterface;
	unsigned int result;
	unsigned char status = 0, control = 0;
	SD_TRACE_INIT();
	if(pInterface == NULL) {
		return  -ENODEV;
	}

	if (pPort->card_present == 0)
	{
		return -ENODEV;
	}

	//printk(" Reading Modem Register \n");
	sd_Read_Register (pInterface, SDUART_MODEM_STATUS_REG, &status);
	control = pPort->MCR;

	result = ((control & MCR_SET_RTS_CONTROL) ? TIOCM_RTS : 0) |
		((control & MCR_SET_DTR_CONTROL) ? TIOCM_DTR : 0) |
		((status & MSR_GET_DCD) ? TIOCM_CAR : 0) |
		((status & MSR_GET_RI)  ? TIOCM_RNG : 0) |
		((status & MSR_GET_DSR) ? TIOCM_DSR : 0) |
		((status & MSR_GET_CTS) ? TIOCM_CTS : 0);
	SD_TRACE_EXIT();
	return result;

}

/*
 * sd_Serial_Ioctl
 *
 * Serial Device IOCTLs for getting/setting device information
 */
SD_STATIC SD_INT32 sd_Serial_Ioctl (struct tty_struct *tty, struct file *filp,
                                     unsigned int cmd, unsigned long arg)
{
	int ival, rc;
	struct _sdio_port *pPort = &sdio_port;
	PSD_INTERFACE pInterface = pPort->pInterface;
	SD_TRACE_INIT();
	if (pInterface == NULL) {
		return (-EIO);
	}

	if (pPort->card_present == 0)
	{
		return (-EIO);
	}


	rc = 0;
	switch (cmd) {
		case TIOCGSOFTCAR:
			rc = put_user((tty->termios->c_cflag & CLOCAL) ? 1 : 0,
					(unsigned int *) arg);
			break;
		case TIOCSSOFTCAR:
			if ((rc = access_ok(VERIFY_READ, (void *) arg, sizeof(int)))) {
				get_user(ival, (unsigned int *) arg);
				tty->termios->c_cflag =
					(tty->termios->c_cflag & ~CLOCAL) |
					(ival ? CLOCAL : 0);
			}
			break;
		case TIOCGSERIAL:
			if ((rc = access_ok(VERIFY_WRITE, (void *) arg, sizeof(struct serial_struct))))
				rc = sd_Get_Serial(&sdio_port, (struct serial_struct *) arg);
			break;
		case TIOCSSERIAL:
			if ((rc = access_ok(VERIFY_READ, (void *) arg, sizeof(struct serial_struct))))
				rc = sd_Set_Serial(&sdio_port, (struct serial_struct *) arg);

			break;
		default:
			rc = -ENOIOCTLCMD;
			break;
	}
	SD_TRACE_EXIT();
	return rc;
}

/*
 * sd_Serial_Open
 *
 * Called when the serial device is opened. Allocate required resources
 * and enable Serial interrupts
 */
SD_STATIC SD_INT32 sd_Serial_Open (struct tty_struct *tty, struct file *filp)
{
	SD_INT32 retval = 0;
	SD_INT line = 0;
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();

	printk("sd_Serial_Open Called \n");

	//setDebugLevel(0);
	if(!sd_Serial_Initialized) {
		sd_Error ("Uart Not initialized Yet\n");
		return -EIO;
	}

	if (pPort->card_present == 0)
	{
		printk("CARD IS NOT PRESENT : EXITING\n");
		return -EIO;
	}
#if 0
		/*
		 * Wating for card initialized
		 */
		while (1)
		{
			if ( SD_TRUE == IsCardInitialized(sdhc,slot) )
			{
				break;
			}
			else
			{
				return -EIO;
				//schedule();
			}
		}
#endif


	line = tty->index;

	if((line < 0)) {
		sd_Error ("Uart Port out of range\n");
		return -ENODEV;
	}

	tty->driver_data = pPort;
	pPort->tty = tty;
	pPort->count++;

	if(!pPort->Trans_buf) {
		pPort->Trans_buf = (unsigned char *) get_zeroed_page(GFP_KERNEL);
		if(!pPort->Trans_buf) {
			return -ENOMEM;
		}
	}

	pPort->Trans_cnt = pPort->Trans_head = pPort->Trans_tail = 0;

	sd_Serial_Set_Termios(pPort->tty,NULL);

	pPort->flags |= ASYNC_INITIALIZED;

	if(pPort->count == 1) {
		if (!try_module_get(THIS_MODULE)) {
			sd_Error("sdhcd: Unable to increment this module \n");
			return -1;
		}
		pPort->modUse++;      /* Added for lin 2.6.9 */
	}

	/*
	 * Start up serial port
	 */

	sd_Enable_Rx_Ints(pPort);
	sd_Enable_Tx_Ints(pPort);

	sd_Block_Til_Ready(pPort,filp);
	if(retval) {
		module_put(THIS_MODULE);
		pPort->modUse--; /* Added for lin 2.6.9 */
		pPort->count--;
		return retval;
	}

	if((pPort->count == 1) &&
			(pPort->flags & ASYNC_SPLIT_TERMIOS)) {
		sd_Set_Termios(pPort);
	}

	tty->low_latency = 1;
	//setDebugLevel(0);
	// sd_Request_Interrupt(0, &UART_Interrupt_Handler);
  //debug_write_alphabet() ;
	SD_TRACE_EXIT();
	return 0;
}

SD_STATIC void sd_Serial_Throttle(struct tty_struct * tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();

	if (pPort->card_present == 0)

	printk("sd_Serial_Open Called \n");
	{
		return;
	}

	if (I_IXOFF(tty)) {
		sd_Send_Xchar (tty, STOP_CHAR(tty));
	}
	SD_TRACE_EXIT();
}

SD_STATIC void sd_Serial_Unthrottle(struct tty_struct * tty)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();

	if (pPort->card_present == 0)
	{
		return;
	}

	if (I_IXOFF(tty)) {
		if (sdio_port.x_char) {
			sdio_port.x_char = 0;
		} else {
			sd_Send_Xchar (tty, START_CHAR(tty));
		}
	}
	SD_TRACE_EXIT();
}

SD_STATIC void sd_Send_Xchar (struct tty_struct *tty, char ch)
{
	struct _sdio_port *pPort = &sdio_port;
	if (pPort == NULL)
		return;
	sdio_port.x_char = ch;
	SD_TRACE_INIT();
	if (ch) {
		sd_Enable_Tx_Ints(tty);
	}
	SD_TRACE_EXIT();
}

/*
 * sd_Driver_Probe
 *
 * SDIO Client driver Initialization Entry point - Configure the device
 * with default settings
 */
SD_STATIC SD_INT32 sd_Driver_Probe(PSD_INTERFACE pInterface)
{
	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();
	memset (pPort, 0, sizeof(sdio_port));
	pPort->triggerLevel         = DEFAULT_TRIGGER_LEVEL;
	pPort->phsMode              = 1;
	pPort->pInterface           = pInterface;

	/* sd_Driver_SetContext is blank */
	sd_Driver_SetContext (pInterface, pPort);
	sd_Init_Semaphore (&pPort->PortClose);
	sd_Init_Semaphore (&pPort->PortWrite);
	init_waitqueue_head (&pPort->open_wait);
	init_waitqueue_head (&pPort->close_wait);

	sd_Serial_Startup (pPort);
  //sd_Serial_Initialized = 1;
	SD_PRINT("LOOKING FOR sd_Serial_Initialized VALUE IS : %d \n",sd_Serial_Initialized);

	SD_TRACE_EXIT();
	return (SD_SUCCESS);
}

PSD_INTERFACE sd_init_Interface()
{
	PSD_INTERFACE psd = 0;
	//psd = kmalloc(GFP_KERNEL, sizeof(struct sd_interface));
	psd = kmalloc(sizeof(struct sd_interface), GFP_KERNEL);

	if (psd)
	{
		memset(psd,0,sizeof(struct sd_interface));
		psd->SizeInSectors = 512;
		psd->BlockSize     = 512;

		// initialize the spin lock

		spin_lock_init(&psd->command);

		//printk("SPIN LOCK HAS Been initialized \n");
	}
	else
	{
		//printk("Error in allocating PSD_INTERFACE \n");
	}

	SD_PRINT("Value of SizeinSectors %d and BlockSize %d \n",
			psd->SizeInSectors, psd->BlockSize);
	return psd;
}



SD_STATIC struct tty_operations tty_ops = {
    .open           = sd_Serial_Open,
    .close          = sd_Serial_Close,
    .write          = sd_Serial_Write,
    .put_char       = sd_Serial_Put_Char,
    .flush_chars    = sd_Serial_Flush_Chars,
    .write_room     = sd_Serial_Write_Room,
    .chars_in_buffer = sd_Serial_Chars_In_Buff,
    .flush_buffer   = sd_Serial_Flush_Buffer,
    .ioctl          = sd_Serial_Ioctl,
    .throttle       = sd_Serial_Throttle,
    .unthrottle     = sd_Serial_Unthrottle,
    .set_termios    = sd_Serial_Set_Termios,
    .stop           = sd_Serial_Stop,
    .start          = sd_Serial_Start,
    .hangup         = sd_Serial_Hangup,
    .tiocmget       = sd_Serial_Get_Modem,
    .tiocmset       = sd_Serial_Set_Modem,
};

/*
 * sd_Uart_Load  - SDIO Serial Driver Module load time initialization code
 */
SD_INT __init sd_Uart_Load(void)
{
	//SD_INT retValue = 0;
	PSD_INTERFACE pInterface = 0;
	int count = 0;
	struct _sdio_port *pPort = &sdio_port;

    printk("\n============================================================================\n");
    printk("            Arasan SDIO (Secure Digital Input Output)- UART driver\n");
    printk("============================================================================\n\n");

	sdio_tty_driver = alloc_tty_driver(1);
	SD_TRACE_INIT();
	if(!sdio_tty_driver) {
		SD_PRINT("*ALLOC FAILURE*\n");
		return -ENOMEM;
	}

	SD_PRINT("*MEMORY ALLOCATION OF TTY IS SUCCESSSFUL->INITIALISING NOW*\n");
	//setDebugLevel(5);


	/*not avaliable - intel
	 sdio_tty_driver->devfs_name 	= "ttys/";
	 */
	sdio_tty_driver->owner                = THIS_MODULE;
	sdio_tty_driver->name                 = "ttyS128" ;
	sdio_tty_driver->major                = TTY_MAJOR;
	sdio_tty_driver->minor_start          = MINOR_START;
	sdio_tty_driver->type                 = TTY_DRIVER_TYPE_SERIAL;
	sdio_tty_driver->subtype              = SERIAL_TYPE_NORMAL;
	sdio_tty_driver->init_termios         = tty_std_termios;
	sdio_tty_driver->init_termios.c_cflag =  B57600 | CS8 | CREAD | HUPCL | CLOCAL;  //B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	sdio_tty_driver->flags                = TTY_DRIVER_REAL_RAW;

	tty_set_operations(sdio_tty_driver,&tty_ops); //TTY_OPERATIONS
	RegisterISRCallBack(user_function,sdhc, slot);
	if (tty_register_driver (sdio_tty_driver)) { //TTY REGISTERING FUNCTION (creates /dev,dvfs entries)
		sd_Error("TTY Registration Failed \n");
		return -1;
	}

	SD_PRINT("*DRIVER REGISTRATION SUCCESSFULL->GOING FOR DEVICE INITIALIZATION* \n");

	//intel : initalising interface implicitly because dont have initialising function
	SD_PRINT("*GOING FOR INIT INTERFACE*\n");

	msleep(1000);
	while(1)
	{
		if(IsCardInitialized(sdhc,slot) == SD_TRUE)
		{
			SD_PRINT("Card is initialized \n");
			count = 1;
			break;
		}
		else if (count > 20)
		{
			SD_PRINT("sduart: Card could not be initialized ...\n");
			count = 0;
			break;
		}
		else
		{
			schedule();
			count++;
		}
	}

	if (0 == count)
	{
		pPort->card_present = 0;
		sd_Serial_Initialized = 0;
		return 0;
	}

	pInterface = sd_init_Interface();
	if (0 == pInterface)
	{
		SD_PRINT("Error in initializing Interface \n");
	}
	else
	{
		SD_PRINT("Value of psd 0x%x \n", (unsigned int)pInterface);
	}

	SD_PRINT("*SD_INTERFACE DONE GOING FOR PROBE*\n");
	sd_Driver_Probe(pInterface);
	SD_PRINT("*PROBE DONE GOING TO EXIT LOAD FUNCTION NOW* \n");

	// WORK INIT_WORK(&pPort->sdio_card_work, UART_Interrupt_Handler, pInterface);
	sd_startup(pInterface);
	pPort->card_present = 1;
	sd_Serial_Initialized = 1;

	SD_TRACE_EXIT();
	return 0;
}

void __exit sd_Uart_Unload(void)
{
	SD_TRACE_INIT();
	sd_cleanup();
	// WORK flush_scheduled_work();
	sd_Driver_Unregister();
	//sd_Driver_Unregister (&sdio_Serial);
	tty_unregister_driver (sdio_tty_driver);
	SD_TRACE_EXIT();
}

module_init(sd_Uart_Load);
module_exit(sd_Uart_Unload);
MODULE_LICENSE("GPL");


void sd_Init_Semaphore (struct semaphore *sp)
{
	SD_TRACE_INIT();
	sema_init(sp,1);
	SD_TRACE_EXIT();
}

int sd_Acquire_Semaphore(struct semaphore *sp)
{
	SD_TRACE_INIT();
    return	down_interruptible(sp);
	SD_TRACE_EXIT();
}

void sd_Release_Semaphore(struct semaphore *sp)
{
	SD_TRACE_INIT();
	up(sp);
	SD_TRACE_EXIT();
}

void user_function(SD_CALL_BACK_INFO *a)
{
	struct _sdio_port *pPort = &sdio_port;
	PSD_INTERFACE pInterface = 0;
	SD_TRACE_INIT();


	if(a->intr_type == SD_CARD_INSERTED)
	{
		pPort->interrupt_type = a->intr_type;
		if(IsCardInitialized(sdhc,slot) == SD_TRUE)
			SD_PRINT("Card is initialized \n");
		else
		{
			pPort->card_present = 0;
			sd_Serial_Initialized = 0;
			return;
		}
		pInterface = sd_init_Interface();
		if (0 == pInterface)
		{
			SD_PRINT("Error in initializing Interface \n");
		}
		else
		{
			SD_PRINT("Value of psd 0x%x \n", (unsigned int)pInterface);
		}
		SD_PRINT("*SD_INTERFACE DONE GOING FOR PROBE*\n");
		sd_Driver_Probe(pInterface);
		SD_PRINT("*PROBE DONE GOING TO EXIT LOAD FUNCTION NOW* \n");
		sd_startup(pInterface);
		pPort->card_present = 1;
		sd_Serial_Initialized = 1;
		SD_TRACE_EXIT();
		return;
	}

	if (1 == sd_Serial_Initialized)
	{

		if (a->intr_type == SD_CARD_REMOVED)
		{
			SD_PRINT("Card Removed \n");
			pPort->interrupt_type = a->intr_type;
			pPort->card_present = 0;
			// RegisterISRCallBack(NULL,sdhc, slot);
			return;
		}
		else if (a->intr_type == SD_CARD_INSERTED)
		{
			SD_PRINT("Card Inserted \n");
			// pPort->card_present = 1;
			pPort->interrupt_type = a->intr_type;
			up(&work_to_do_sema);
			return;
		}
		//printk("schedule the work...\n");

		// wake up the thread..
		up(&work_to_do_sema);
	}
	else
	{
		SD_PRINT("Enabling Function Interrupt..\n");
		EnableFunctionInterrupt(sdhc,slot);
	}

	SD_TRACE_EXIT();
}

// yet to be defined
//interrupt processing
//void sd_Request_Interrupt(PSD_INTERFACE pInterface,void (*uart)(SD_INTERFACE *))
void sd_Request_Interrupt(PSD_INTERFACE pInterface,void (*uart)(void *))
{
	SD_TRACE_INIT();

	RegisterISRCallBack(user_function,sdhc, slot);

	SD_TRACE_EXIT();
}

void sd_Driver_SetContext (PSD_INTERFACE pInterface, struct _sdio_port *pPort)
{
	SD_TRACE_INIT();
	SD_TRACE_EXIT();
}

struct _sdio_port *sd_Driver_GetContext(PSD_INTERFACE pInterface)
{
	SD_TRACE_INIT();
	SD_TRACE_EXIT();
	return SD_SUCCESS;
}

SD_UINT sd_Set_BlockLength(PSD_INTERFACE pInterface, SD_UINT someint)
{
	SD_TRACE_INIT();
	SD_TRACE_EXIT();
	return SD_SUCCESS;
}

SD_UINT sd_Enable_Interface(PSD_INTERFACE pInteface)
{
	SD_TRACE_INIT();
	SD_TRACE_EXIT();
	return SD_SUCCESS;
}

void sd_Driver_Unregister(void)
{
	SD_TRACE_INIT();
	SD_TRACE_EXIT();
}

SD_INT32 sd_Read_Register(PSD_INTERFACE pInterface, unsigned int address, unsigned char *value)
{
	int status;
	SD_TRACE_INIT();

	command_lock++;
	status = ReadSDIORegister(1,address,value,sdhc,slot);
	command_lock--;

	SD_PRINT("SD_READ_REGISTER() - > VALUE RETURNED IS 0x%x\n",*value);
	SD_TRACE_EXIT();
	return SD_SUCCESS; // status;
}

SD_INT32 sd_Write_Register(PSD_INTERFACE pInterface, unsigned int address, unsigned char value)
{
	int status;
	SD_TRACE_INIT();
	SD_PRINT("SD_WRITE_REGISTER() - > VALUE WRITING IS 0x%x\n",value);

	command_lock++;
	status = WriteSDIORegister(1,address,value,sdhc,slot);
	command_lock--;

	SD_TRACE_EXIT();
	return SD_SUCCESS; // status;
}

SD_INT32  sd_Read_Register_Multiple (PSD_INTERFACE pInterface,SD_UINT someint,
                                           SD_UCHAR  rxCount,SD_UINT someint2,SD_UCHAR *rxBuf)
{
	SD_TRACE_INIT();
	SD_TRACE_EXIT();
	return SD_SUCCESS;
}

#if 0
void debug_dump_registers()
{
	unsigned char value;
	unsigned int status ;

	status = ReadSDIORegister(1,SDUART_RECEIVE_REG,&value,sdhc,slot);
	SD_PRINT("Value of Status 0x%x \n",status);
	SD_PRINT("VALUE OF SDUART_RECEIIVE_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_TRANS_HOLD_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_TRANS_HOLD_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_INTR_ENABLE_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_INTR_ENABLE_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_INTR_IDENT_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_INTR_IDENT_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_FIFO_CONTROL_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_FIFO_CONTROL_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_LINE_CONTROL_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_LINE_CONTROL_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_LINE_STATUS_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_LINE_STATUS_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_MODEM_CONTROL_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_MODEM_CONTROL_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_MODEM_STATUS_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_MODEM_STATUS_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_DIVISOR_LATCH_LS_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_DIVISOR_LATCH_LS_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_DIVISOR_LATCH_MS_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_DIVISOR_LATCH_MS_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_EXT_FIFO_ENABLE_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_EXT_FIFO_ENABLE_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_TRANS_COUNT_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_TRANS_COUNT_REG IS : 0x%x\n",value);

	ReadSDIORegister(1,SDUART_RECEIVE_COUNT_REG,&value,sdhc,slot);
	SD_PRINT("VALUE OF SDUART_RECEIVE_COUNT_REG IS : 0x%x\n",value);
}

void debug_write_alpha()
{
	char buffer[32] = "This is a 32 byte buffer ";

	SD_TRACE_INIT();

	SendData(buffer, SDUART_TRANS_HOLD_REG, 32, slot);

	SD_TRACE_EXIT();
}

void debug_write_alphabet()
{
	int num;
	for (num = 50; num < 75; num++)	{
		// Write a char...
		printk("VALUE OF NUMBER IS : %d\n",num);
		WriteSDIORegister(1,SDUART_TRANS_HOLD_REG,num,sdhc,slot);
		//	mdelay(1000);
//		schedule();
	}
}

#endif

int sd_Process_Intr(void *data)
{
	//printk("sd_Process_Intr is called ...\n");.

    int a =0;

	struct _sdio_port *pPort = &sdio_port;
	SD_TRACE_INIT();
	allow_signal(SIGKILL);

	while(1)
	{

		// Call the Interrupt Handler
		
		a = down_interruptible(&work_to_do_sema);
	
		//printk("Calling INT Handler from Thread \n");

		if (SD_CARD_REMOVED == pPort->interrupt_type)
		{
			printk("Card has been removed \n");
		}
		else if (SD_CARD_INSERTED == pPort->interrupt_type)
		{
			while (1)
			{
				pPort = &sdio_port;
				SD_PRINT("CHECKING FOR CARD INITALIZED\n");
				if (SD_TRUE == IsCardInitialized(sdhc,slot))
				{
					mdelay(500);
					RegisterISRCallBack(user_function,sdhc, slot);
					pPort->card_present = 1;
					break;
				}
				else
				{
					schedule();
				}
			}
		}
		else
		{
			UART_Interrupt_Handler(data);
		}

		if (signal_pending(current))
			break;

	}

	//printk("Breaking from kernel thread during cleanup...\n");

	up(&cleanup_sema);
	set_current_state(TASK_RUNNING);
	SD_TRACE_EXIT();
	return 0;
}

void sd_startup(PSD_INTERFACE pInterface)
{
	init_MUTEX_LOCKED(&cleanup_sema);
	init_MUTEX_LOCKED(&work_to_do_sema);
	SD_TRACE_INIT();
	threadid = kernel_thread(sd_Process_Intr, pInterface,CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGKILL);

	if (threadid < 0 )
		printk("thread Creation Failed ...\n");
	SD_TRACE_EXIT();
}

void sd_cleanup()
{

	int err = 0;

	int a = 0;

	if (0 == threadid )
	{
		printk("Thread was never created...\n");
		return;
	}

	kill_pid(find_vpid(threadid), SIGKILL, 1);
	msleep(10);
	a=down_interruptible(&cleanup_sema);
	err = kill_pid(find_vpid(2), SIGKILL, 1);
	
	if (err < 0)
		printk("Error in killing thread...\n");
}

