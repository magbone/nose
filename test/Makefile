

APPS = \
	crypt_test\
	dfa_test\
	sha256_test\
	time_test\
	tun_test\
	bucket_test\
	master_peer_test\
	pmp_test\
	udp_test\
	nat_type_test\
	peer_test

# End of APPS

DARWIN_TUN_DEVICE_SOURCE = "../src/bsd/device.c"
LINUX_TUN_DEVICE_SOURCE  = "../src/linux/device.c"

TUN_DEVICE_SOURCE = ""

ifeq ($(shell uname), Darwin)
	TUN_DEVICE_SOURCE = $(DARWIN_TUN_DEVICE_SOURCE)
endif
ifeq ($(shell uname), Linux)
	TUN_DEVICE_SOURCE = $(LINUX_TUN_DEVICE_SOURCE)
endif

all:crypt_test dfa_test time_test tun_test sha256_test bucket_test master_peer_test pmp_test udp_test nat_type_test peer_test


crypt_test: 
	cc crypt_test.c ../src/crypt.c -lmbedtls -lmbedx509 -lmbedcrypto -o crypt_test

dfa_test:
	cc dfa_test.c ../src/dfa.c -o dfa_test

time_test:
	cc time_test.c ../src/timer.c -o time_test

tun_test:
	cc tun_test.c $(TUN_DEVICE_SOURCE) ../src/net_config.c -o tun_test

sha256_test:
	cc sha256_test.c ../src/utils.c -lssl -lcrypto -o sha256_test -w

bucket_test:
	cc bucket_test.c ../src/bucket.c ../src/utils.c -lssl -lcrypto -o bucket_test -w

master_peer_test:
	cc master_peer_test.c ../src/server.c ../src/master_peer.c ../src/bucket.c ../src/timer.c ../src/udp.c ../src/pmp.c -luv -lpthread -o master_peer_test -g

pmp_test:
	cc pmp_test.c ../src/pmp.c ../src/bucket.c -o pmp_test

udp_test:
	cc udp_test.c ../src/udp.c ../src/pmp.c ../src/bucket.c -o udp_test

nat_type_test:
	cc nat_type_test.c ../src/stun/nat_test.c ../src/utils.c -lssl -lcrypto -luv -w -o nat_type_test -D DEBUG

peer_test:
	cc peer_test.c ../src/stun/nat_test.c ../src/utils.c ../src/peer.c ../src/pmp.c ../src/bucket.c -lssl -lcrypto -luv -lpthread -w -o peer_test

clean: 
	rm -rf $(APPS)

