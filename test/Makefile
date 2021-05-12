

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
	peer_test\
	conf_test\
	key_neg_test

# End of APPS

C_FLAGS ?= -std=gnu99 -w

DARWIN_TUN_DEVICE_SOURCE = "../src/bsd/device.c"
LINUX_TUN_DEVICE_SOURCE  = "../src/linux/device.c"

TUN_DEVICE_SOURCE = ""

ifeq ($(shell uname), Darwin)
	TUN_DEVICE_SOURCE = $(DARWIN_TUN_DEVICE_SOURCE)
endif
ifeq ($(shell uname), Linux)
	TUN_DEVICE_SOURCE = $(LINUX_TUN_DEVICE_SOURCE)
endif

all:crypt_test dfa_test sha256_test bucket_test master_peer_test pmp_test udp_test tun_test nat_type_test peer_test conf_test key_neg_test


crypt_test: 
	cc crypt_test.c ../src/crypt.c -lmbedtls -lmbedx509 -lmbedcrypto -o crypt_test $(C_FLAGS)

dfa_test:
	cc dfa_test.c ../src/dfa.c -o dfa_test $(C_FLAGS)

tun_test:
	cc tun_test.c ../src/net_config.c -levent -o tun_test $(C_FLAGS) -g

sha256_test:
	cc sha256_test.c ../src/utils.c -lmbedtls -lmbedx509 -lmbedcrypto -o sha256_test $(C_FLAGS)

bucket_test:
	cc bucket_test.c ../src/bucket.c ../src/utils.c -lmbedtls -lmbedx509 -lmbedcrypto -o bucket_test $(C_FLAGS)

master_peer_test:
	cc master_peer_test.c ../src/master_peer.c ../src/bucket.c ../src/udp.c ../src/pmp.c ../src/conf/conf-reader.c -levent -lpthread -o master_peer_test -g $(C_FLAGS) -D DEBUG

pmp_test:
	cc pmp_test.c ../src/pmp.c ../src/bucket.c -o pmp_test $(C_FLAGS)

udp_test:
	cc udp_test.c ../src/udp.c ../src/pmp.c ../src/bucket.c -o udp_test $(C_FLAGS)

nat_type_test:
	cc nat_type_test.c ../src/stun/nat_test.c ../src/utils.c -lmbedtls -lmbedx509 -lmbedcrypto -o nat_type_test -D DEBUG $(C_FLAGS)

peer_test:
	cc peer_test.c ../src/stun/nat_test.c ../src/utils.c ../src/peer.c ../src/pmp.c ../src/bucket.c ../src/udp.c ../src/conf/conf-reader.c ../src/pcp.c ../src/net_config.c $(TUN_DEVICE_SOURCE) ../src/crypt.c -lmbedtls -lmbedx509 -lmbedcrypto -lssl -lcrypto -luv -lpthread -lz -levent -o peer_test $(C_FLAGS) -g 

conf_test:
	cc conf_test.c ../src/conf/conf-reader.c -o conf_test $(C_FLAGS)

key_neg_test:
	cc key_neg_test.c ../src/key_neg.c -L ~/Downloads/mbedtls/library -lmbedtls -lmbedx509 -lmbedcrypto -o key_neg_test

clean: 
	rm -rf $(APPS)

