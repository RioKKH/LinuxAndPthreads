# := 即時展開 変数の値は定義時に評価され固定される
# =  遅延展開 変数の値は使用時に評価される
TARGET := soloFlyQueue queueTest
CFLAGS := $(CFLAGS) -pthread
LDFLAGS := $(LDFLAGS) -pthread

.PHONY: all clean

all: $(TARGET)

soloFlyQueue: soloFlyQueue.o xyqueue.o
	$(CC) $(LDFLAGS) $^ -lm -o $@

queueTest: queueTest.o xyqueue.o
	$(CC) $(LDFLAGS) $^ -lm -o $@

# Implicit rules
# %はワイルドカード（パターン）を表し、任意の文字列にマッチする
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
