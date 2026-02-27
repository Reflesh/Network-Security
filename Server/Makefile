CC       = gcc
CFLAGS   = -Wall -O2
LDFLAGS  = -lssl -lcrypto

SRCS_COMMON = src/tls_common.c src/tun.c
OBJS_COMMON = $(SRCS_COMMON:.c=.o)

all: server

server: $(OBJS_COMMON) src/server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 모든 src/*.c 를 src/*.o 로 컴파일
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o server
