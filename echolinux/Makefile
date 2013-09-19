INCS = echolinux.h rtp.h echoGlobals.h

# Debugging options

DEBUG = -g -DHEXDUMP
  
# Installation 

#CCFLAGS =  -DAUDIO_BLOCKING -DLINUX -DHALF_CONTROL -DM_LITTLE_ENDIAN
CCFLAGS =
CC = gcc -O3 # for GNU's gcc compiler
LFLAGS = -lm

INTERNET_PORT = 2074

CARGS = -DInternet_Port=$(INTERNET_PORT)

#   Compiler flags

CFLAGS = $(DEBUG) -Igsm/inc $(CARGS) $(CCFLAGS)
#CFLAGS = -Igsm/inc $(CARGS) $(CCFLAGS)

PROGRAMS = echolinux echoaudio

all:	$(PROGRAMS)

echolinux: control.o rtpacket.o
	$(CC) control.o rtpacket.o $(CFLAGS) libgsm.a \
	$(LFLAGS) -o echolinux 

echoaudio: vox.o 
	$(CC) vox.o $(CFLAGS) libgsm.a $(LFLAGS) -o echoaudio 

control.o: control.c $(INCS)
	$(CC) -c $(CFLAGS) control.c 

vox.o: vox.c $(INCS)
	$(CC) -c $(CFLAGS) vox.c 

rtpacket.o: rtpacket.c $(INCS)
	$(CC) -c $(CFLAGS) rtpacket.c 

copy_defaults:
	if [ ! -d ~/.echoLinux ]; then mkdir ~/.echoLinux; fi; \
	cp -f *.txt ~/.echoLinux ; \
	cp -f *.wav ~/.echoLinux 

install: 
	cp echolinux /usr/local/bin/echolinux; \
	cp echoaudio /usr/local/bin/echoaudio

clean:
	rm -f *.o $(PROGRAMS)



