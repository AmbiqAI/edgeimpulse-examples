#makefile written for AES and Ambriq 
HDRS =  aes_cipher.h aes_encrypt_tables.h aes_decrypt_tables.h 
SRCS =  aes_encrypt.c aes_decrypt.c expand_key.c


#AR=arm-none-eabi-ar
#CC = arm-none-eabi-gcc
#CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
#CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
#CFLAGS += -ffreestanding -nostdlib -DARM


CFLAGS = -O3 
CC = gcc

all: aes aes_cbc expand_key

aes: $(SRCS) $(HDRS) aes_driver.c
	$(CC)  $(CFLAGS) aes_driver.c $(SRCS) -o aes


aes_cbc: $(SRCS) $(HDRS) aes_driver_cbc.c
	$(CC)  $(CFLAGS) -DAES_CBC_MODE aes_driver_cbc.c $(SRCS) -o aes_cbc

#compile the driver that test 128/192/256 keys against reference standard expansions 
expand_key: expand_key.c 
	$(CC)  $(CFLAGS) -DTEST_KEY  expand_key.c -o expand_key

run:
	./aes_cbc raw_input.bin 

test:
	./aes

test_key:
	./expand_key

tar:
	tar -cf aes_code.tar Makefile $(SRCS) $(HDRS) aes_driver.c aes_driver_cbc.c
clean:
	rm -f aes expand_key aes_cbc

