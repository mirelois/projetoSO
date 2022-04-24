CC = gcc
CFLAGS = -Wall -g

all: sdstore sdstored

sdstored: sdstored.c init.c hashtable.c

sdstore: sdstore.c init.c hashtable.c







