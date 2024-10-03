#ifndef _NEX_COMMON_TYPES_H
#define _NEX_COMMON_TYPES_H

#define FAILURE -1
#define SUCCESS	 0

typedef char	CHAR,	*PCHAR;
typedef unsigned char UCHAR, *PUCHAR;

typedef short INT16, *PINT16;
typedef unsigned short UINT16, *PUINT16;


typedef int INT32, *PINT32;
typedef unsigned int UINT32, *PUINT32;

typedef long LONG, *PLONG;
typedef unsigned long ULONG, *PULONG;

typedef long long LONGLONG, *PLONGLONG;
typedef unsigned long long ULONGLONG, *PULONGLONG;

typedef	void	VOID, *PVOID;

typedef ULONGLONG DMA_ADDR_T;

typedef struct nex_scatterlist {
	//ULONG  sg_magic;
	ULONG  page_link;
	UINT32 offset;
	UINT32 length;
	DMA_ADDR_T dma_address;
	UINT32 dma_length;
}SCATTERLIST;


typedef enum nex_irqreturn_t {
       NEX_IRQ_NONE                = (0 << 0),
       NEX_IRQ_HANDLED             = (1 << 0),
}IRQRETURN_T;

#define MAX_SLOTS 1
#define MAX_SDHC  1

#endif
