TARGET = encrypt_binary
DEFINES = AES_CBC_MODE
CDEFINES = $(DEFINES:%=-D%)

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(SOURCES:%.c=$(BINDIR)/%.o)
BINDIR = bin

CC = gcc
CFLAGS = -MMD -MP $(CDEFINES)

all: $(TARGET)

clean:
	rm -rf bin $(TARGET)

realclean: clean

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(BINDIR):
	@mkdir -p bin

$(BINDIR)/%.o: %.c | $(BINDIR)
	$(CC) -c $(CFLAGS) $< -o $@
