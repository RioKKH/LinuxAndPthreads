TARGET := firstThread

CFLAGS := $(CFLAGS) -pthread
LDFLAGS := $(LDFLAGS) -pthread

all: $(TARGET)

firstThread: firstThread.c
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) *.o
