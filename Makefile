

C_FLAGS ?= -std=gnu99 -w -O2
LIBS ?= -lmbedtls -lmbedx509 -lmbedcrypto -lssl -lcrypto -luv -lpthread -lz -levent
DARWIN_TUN_DEVICE_SOURCE = "./src/bsd/device.c"
LINUX_TUN_DEVICE_SOURCE  = "./src/linux/device.c"

TUN_DEVICE_SOURCE = ""

ifeq ($(shell uname), Darwin)
	TUN_DEVICE_SOURCE = $(DARWIN_TUN_DEVICE_SOURCE)
endif
ifeq ($(shell uname), Linux)
	TUN_DEVICE_SOURCE = $(LINUX_TUN_DEVICE_SOURCE)
endif

all: nose

nose:
	cc ./src/bucket.c ./src/crypt.c ./src/master_peer.c ./src/net_config.c \
	./src/pcp.c ./src/peer.c ./src/pmp.c ./src/sem.c ./src/timer.c ./src/utils.c \
	./src/udp.c ./src/conf/conf-reader.c ./src/stun/nat_test.c ./src/nose.c $(TUN_DEVICE_SOURCE) $(LIBS)  -o nose $(C_FLAGS)
clean:
	rm -rf ./nose