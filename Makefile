CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
LIBS = -lrt -lpthread

SRCS = main.c scheduler.c logger.c ipc_shm.c bankers.c worker.c
OBJS = $(SRCS:.c=.o)
TARGET = campus_compute

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) campus_compute.log
