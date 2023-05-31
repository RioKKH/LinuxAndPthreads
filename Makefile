TARGET := firstThread firstThread_mainShorter firstThread2 firstThread2_exitProcess\
					firstThread3 firstThread3_join firstThread3_join_better\
					firstThread4 firstThread_equal firstThread_self threadAttr threadCancel\
					threadTimer

CFLAGS := $(CFLAGS) -pthread
LDFLAGS := $(LDFLAGS) -pthread

all: $(TARGET)

firstThread: firstThread.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread_mainShorter: firstThread_mainShorter.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread2: firstThread2.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread2_exitProcess: firstThread2_exitProcess.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread3: firstThread3.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread3_join: firstThread3_join.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread3_join_better: firstThread3_join_better.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread4: firstThread4.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread_equal: firstThread_equal.c
	$(CC) $(LDFLAGS) $^ -o $@

firstThread_self: firstThread_self.c
	$(CC) $(LDFLAGS) $^ -o $@

threadAttr: threadAttr.c
	$(CC) $(LDFLAGS) $^ -o $@

threadCancel: threadCancel.c
	$(CC) $(LDFLAGS) $^ -o $@

threadTimer: threadTimer.c
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) *.o
