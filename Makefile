TARGET := firstThread firstThread_mainShorter firstThread2 firstThread2_exitProcess\
					firstThread3 firstThread3_join firstThread3_join_better\
					firstThread4 firstThread_equal firstThread_self threadAttr threadCancel\
					threadTimer fastPrimeNumber_NG fastPrimeNumber_OK fly2 fly3_deadlock\
					fly3_nodeadlock fly2rwlock soloFly soloFly2 condTimedWait soloFly3 fly5 fly6\
					timer_finalVersion timer_finalVersion2 timer_finalVersion3 timer


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

fastPrimeNumber_NG: fastPrimeNumber_NG.c
	$(CC) $(LDFLAGS) $^ -o $@ -g

fastPrimeNumber_OK: fastPrimeNumber_OK.c
	$(CC) $(LDFLAGS) $^ -o $@ -g

fly2: fly2.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

fly3_deadlock: fly3_deadlock.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

fly3_nodeadlock: fly3_nodeadlock.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

fly2rwlock: fly2rwlock.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

soloFly: soloFly.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

soloFly2: soloFly2.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

condTimedWait: condTimedWait.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

soloFly3: soloFly3.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

fly5: fly5.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

fly6: fly6.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

timer_finalVersion: timer_finalVersion.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

timer_finalVersion2: timer_finalVersion2.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

timer_finalVersion3: timer_finalVersion3.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm

timer: timer.c
	$(CC) $(LDFLAGS) $^ -o $@ -g -lm


clean:
	rm -f $(TARGET) *.o
