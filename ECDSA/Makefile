export CFLAGS="-I/opt/homebrew/Cellar/openssl@3/3.2.1/include"
export NISTFLAGS="-I/opt/homebrew/Cellar/openssl@3/3.2.1/include"
export LDFLAGS="-L/opt/homebrew/Cellar/openssl@3/3.2.1/lib"

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wno-deprecated-declarations
OPENSSL_INCLUDE_PATH = -I/opt/homebrew/Cellar/openssl@3/3.2.1/include
OPENSSL_LIB_PATH = -L/opt/homebrew/Cellar/openssl@3/3.2.1/lib
OPENSSL_LIBS = -lssl -lcrypto

SOURCES = GEN.c SIG.c VER.c ../timeMeasure.c
HEADERS = ../timeMeasure.h

.PHONY: all clean

all: GEN SIG VER

GEN: GEN.c ../timeMeasure.c
	$(CC) $(CFLAGS) -o $@ $^ $(OPENSSL_INCLUDE_PATH) $(OPENSSL_LIB_PATH) $(OPENSSL_LIBS)

SIG: SIG.c ../timeMeasure.c
	$(CC) $(CFLAGS) -o $@ $^ $(OPENSSL_INCLUDE_PATH) $(OPENSSL_LIB_PATH) $(OPENSSL_LIBS)

VER: VER.c ../timeMeasure.c
	$(CC) $(CFLAGS) -o $@ $^ $(OPENSSL_INCLUDE_PATH) $(OPENSSL_LIB_PATH) $(OPENSSL_LIBS)

clean:
	rm -f GEN SIG VER
