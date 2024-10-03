TARGET = encrypted_boot_generator
DEFINES = AES_CBC_MODE
CDEFINES = $(DEFINES:%=-D%)

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(SOURCES:%.c=$(BINDIR)/%.o)
BINDIR = bin

CC = gcc
CFLAGS = -MMD -MP $(CDEFINES)

all: $(TARGET) test/encrypted_gen5.bin 

test/encrypted_gen5.bin:encrypted_boot_generator generated_data.bin
	./encrypted_boot_generator generated_data.bin test/encrypted_gen5 encrypted_data 5 0x70000 |tee test/encrypt_trace_gen5_output
	cp ./test/encrypted_gen5.bin /tmp

TEST:
	./encrypted_boot_generator generated_data.bin test/encrypted_gen0 encrypted_data 0 0x70000
	./encrypted_boot_generator generated_data.bin test/encrypted_gen1 encrypted_data 1 0x70000
	./encrypted_boot_generator generated_data.bin test/encrypted_gen2 encrypted_data 2 0x70000
	./encrypted_boot_generator generated_data.bin test/encrypted_gen3 encrypted_data 3 0x70000
	./encrypted_boot_generator generated_data.bin test/encrypted_gen4 encrypted_data 4 0x70000
	./encrypted_boot_generator generated_data.bin test/encrypted_gen5 encrypted_data 5 0x70000
	./encrypted_boot_generator generated_data.bin test/encrypted_gen6 encrypted_data 6 0x70000
	./encrypted_boot_generator generated_data.bin test/encrypted_gen7 encrypted_data 7 0x70000
	./encrypted_boot_generator  ../libcn_sf_gest_secure_dll_xo test/cloudnav_dll CLOUDNAV_XO 4 0x70000


clean:
	rm -rf bin $(TARGET) current_key.bin test/*

realclean: clean

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(BINDIR):
	@mkdir -p bin

$(BINDIR)/%.o: %.c | $(BINDIR)
	$(CC) -c $(CFLAGS) $< -o $@
