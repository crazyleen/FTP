
TOPDIR = $(shell pwd)
BUILDDIR ?= $(TOPDIR)/obj
PREFIX ?= $(TOPDIR)/bin

#CROSS_COMPILE ?= arm-none-linux-gnueabi-
#CROSS_COMPILE ?= arm-arago-linux-gnueabi-
#CROSS_COMPILE ?= arm-linux-
CC=$(CROSS_COMPILE)gcc
CFLAGS = -g -Wall
LDFLAGS := 
LIBS :=
INCLUDES := -I$(TOPDIR)

VPATH = $(BUILDDIR)

#export 
export CC
export CFLAGS
export INCLUDES
export TOPDIR


APP=client server
all: $(APP) 

OBJECTSCLIENT = ftp_packet.o client_ftp_functions.o file_transfer_functions.o client_ftp.o
client:	${OBJECTSCLIENT}
	@@cd $(BUILDDIR) && cd $(BUILDDIR) && $(CC) $(INCLUDES)  ${CFLAGS}  $^ -o $@ ${LDFLAGS}

OBJECTSSERVER = ftp_packet.o server_ftp_functions.o file_transfer_functions.o server_ftp.o
server:	${OBJECTSSERVER}
	@@cd $(BUILDDIR) && cd $(BUILDDIR) && $(CC) $(INCLUDES)  ${CFLAGS}  $^ -o $@ ${LDFLAGS}
	
.c.o:
	@$(CC) -c $(CFLAGS) $(INCLUDES)   $< -o $(BUILDDIR)/$@

install:
	@cd $(BUILDDIR) && cp  $(APP) $(PREFIX)
	

clean:
	@cd $(BUILDDIR) && rm -f *.o $(APP)
	@cd $(PREFIX) && rm -f $(APP)

.PHONY: clean install