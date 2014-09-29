# makefile of GigE SDK Sample code
#

#include ./ARCH

# Target CPU
#CPU	= x86
CPU	    = x64

# Target OS
OS	    = OSX
OS_DIR  = OSX

# Install target
BIN_DIR = /usr/local/bin

# architecture flags
#ARCH	= -m32

# compilation flags
OPT	= -O3 $(ARCH)  

# compiler version
#CVER    = 4.2
# compiler
#CC      = g++-$(CVER)
#CC = g++
CC = g++
# linker
LD      = ld
AR      = ar
SP      = strip

# some flags
#DFLAGS	= -D_$(CPU) -D_$(OS) -DUSE_AVT_CAM
DFLAGS	= -D_$(CPU) -D_$(OS) 
#FLAGS   = -fno-strict-aliasing -fexceptions -I/usr/include $(DFLAGS)
FLAGS   = -I/usr/include $(DFLAGS)

## TIFF library
#LTIFF   = -ltiff

# BOOST Setting
BST_SALIB = -BStatic /usr/lib/x86_64-linux-gnu/libboost_program_options.a

# OpenCV Setting
CV_LIB = -lopencv_core -lopencv_highgui -lopencv_imgproc 

## few locations
OBJ_DIR	  = ./obj/$(CPU)
EXTRA_LIB = -lpthread -lz

## library
LJPEG   = -ljpeg

## libs (shared/static)
#PVLIB   = -lPvAPI
SOLIB	= $(EXTRA_LIB) -Bdynamic -lm -lc

## final compilation flags
CFLAGS	= $(OPT) $(FLAGS) -Wall -D_REENTRANT $(EXTRA)

## Executable
EXE	= Arrange

## Other Objects
OBJS = $(OBJ_DIR)/IO.o $(OBJ_DIR)/coefs.o $(OBJ_DIR)/AVTCamera.o $(OBJ_DIR)/Lock.o
    
sample : $(EXE).cpp $(OBJS)
	$(CC) $(RPATH) $(TARGET) $(CFLAGS) $(EXE).cpp -o $(EXE) $(SOLIB) $(PVLIB) $(OBJS) $(CV_LIB) $(BST_SALIB)

$(OBJ_DIR)/%.o : %.cpp
	$(CC) $(CFLAGS) $(VERSION) -c $< -o $@

clean: 
	rm $(OBJ_DIR)/*
	rm $(EXE)

install:
	cp -f $(EXE) $(BIN_DIR)
