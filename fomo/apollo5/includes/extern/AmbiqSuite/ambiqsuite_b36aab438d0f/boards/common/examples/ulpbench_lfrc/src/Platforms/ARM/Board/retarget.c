#include "board.h"

//
// If DEBUG mode is set in board.h
//
#if DEBUG>2

//
// If using IAR compiler include.
//
#if iar
//******************************************************************************
//
//! @brief Retarget printf to ITM for IAR compiler
//!
//! @param handle - STDIO file handle
//! @param *buf - Pointer to formatter string
//! @param size - Pointer to formatter string
//!
//! Intercepts STDOUT data and transmits out the ITM.
//!
//! @return size_t representing the number of characters printed.
//
//******************************************************************************
size_t __write(int handle, const unsigned char *buf, size_t size) 
{
	//
	// Print data to the ITM.
	//
    am_hal_itm_print((char *)buf);

    //
    // Return number of bytes transmitted.
    //
    return size;
}

//*****************************************************************************
//
// __lseek stub.
//
//*****************************************************************************
int __lseek(int fd, long offset, int origin)
{
	return NULL;
}

//*****************************************************************************
//
// __close() stub.
//
//*****************************************************************************
int __close(int fd)
{
	return NULL;
}

//*****************************************************************************
//
// remove() stub.
//
//*****************************************************************************
int remove(const char *path)
{
	return NULL;
}

#endif // iar

//
// IF using KEIL or gcc include.
//
#if keil|gcc

//
// FILE structure handle.
//
struct __FILE {
	int handle;
};

FILE __stdout;

//******************************************************************************
//
//! @brief Retarget printf to ITM for IAR compiler
//!
//! @param c - Character to transmit
//! @param *f - STDIO file handle
//!
//! Intercepts STDOUT data and transmits out the ITM.
//!
//! @return size_t representing the number of characters printed.
//
//******************************************************************************
int fputc(int c, FILE *f){
	int nbytes = 1;

	//
	// Print data to the ITM
	//
	am_hal_itm_print((char *)&c);	

	//
	// Return number of bytes transmitted
	// 
	return nbytes;
}

#endif // keil|gcc

#endif // DEBUG>2
