

APPS = \
	crypt_test\
	dfa_test\
	pcap_test\
	sha256_test\
	time_test\
	tun_test

# End of APPS


all:crypt_test dfa_test pcap_test time_test tun_test sha256_test


crypt_test: 
	cc crypt_test.c ../src/crypt.c -lmbedtls -lmbedx509 -lmbedcrypto -o crypt_test

dfa_test:
	cc dfa_test.c ../src/dfa.c -o dfa_test

pcap_test:
	cc pcap_test.c -lpcap -o pcap_test

time_test:
	cc time_test.c ../src/timer.c -o time_test

tun_test:
	cc tun_test.c ../src/bsd/device.c ../src/net_config.c -o tun_test

sha256_test:
	cc sha256_test.c ../src/utils.c -lssl -lcrypto -o sha256_test -w

clean: 
	rm -rf $(APPS)

