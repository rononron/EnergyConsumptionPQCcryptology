PARAMS = sphincs-sha2-192s
THASH = simple

CC=/usr/bin/gcc
CFLAGS=-Wall -Wextra -Wpedantic -O3 -std=c99 -Wconversion -Wmissing-prototypes -DPARAMS=$(PARAMS) $(EXTRA_CFLAGS) -I/opt/homebrew/Cellar/openssl@3/3.2.1/include
LDFLAGS =-L/opt/homebrew/Cellar/openssl@3/3.2.1/lib
LDLIBS=-L/opt/homebrew/Cellar/openssl@3/3.2.1/lib -lcrypto

SOURCES =          address.c randombytes.c merkle.c wots.c wotsx1.c utils.c utilsx1.c fors.c sign.c ../timeMeasure.c
HEADERS = params.h address.h randombytes.h merkle.h wots.h wotsx1.h utils.h utilsx1.h fors.h api.h  hash.h thash.h ../timeMeasure.h 

ifneq (,$(findstring shake,$(PARAMS)))
	SOURCES += fips202.c hash_shake.c thash_shake_$(THASH).c
	HEADERS += fips202.h
endif
ifneq (,$(findstring haraka,$(PARAMS)))
	SOURCES += haraka.c hash_haraka.c thash_haraka_$(THASH).c
	HEADERS += haraka.h
endif
ifneq (,$(findstring sha2,$(PARAMS)))
	SOURCES += sha2.c hash_sha2.c thash_sha2_$(THASH).c
	HEADERS += sha2.h
endif

DET_SOURCES = $(SOURCES:randombytes.%=rng.%)
DET_HEADERS = $(HEADERS:randombytes.%=rng.%)

.PHONY: clean 

default: GEN SIG VER


GEN: GEN.c $(DET_SOURCES) $(DET_HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(DET_SOURCES) $< -lcrypto

SIG: SIG.c $(DET_SOURCES) $(DET_HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(DET_SOURCES) $< -lcrypto

VER: VER.c $(DET_SOURCES) $(DET_HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(DET_SOURCES) $< -lcrypto


clean:
	rm -f GEN
	rm -f SIG
	rm -f VER