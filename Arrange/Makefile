# makefile of GigE SDK Sample code
#

#include ./ARCH

# Target CPU
#CPU	= x86
CPU	    = x64

# Target OS
OS	    = OSX
OS_DIR  = OSX

# OS-X version
OSXV	= -mmacosx-version-min=10.7
# architecture flags
#ARCH	= -m32

# compilation flags
OPT	= -O3 $(ARCH) $(OSXV) -stdlib=libc++

# compiler version
#CVER    = 4.2
# compiler
#CC      = g++-$(CVER)
#CC = g++
CC = clang++
# linker
LD      = ld
AR      = ar
SP      = strip

# some flags
DFLAGS	= -D_$(CPU) -D_$(OS) -DUSE_AVT_CAM
#FLAGS   = -fno-strict-aliasing -fexceptions -I/usr/include $(DFLAGS)
FLAGS   = -I/usr/include $(DFLAGS)

## TIFF library
#LTIFF   = -ltiff

# BOOST Setting
BST_DIR = /usr/local/lib/boost_1_55_0_with_clang
BST_INC_DIR = $(BST_DIR)/include
BST_SALIB = -BStatic $(BST_DIR)/lib/libboost_program_options.a

# OpenCV Setting
CV_LIB = -lopencv_core -lopencv_highgui -lopencv_imgproc -lavdevice

## few locations
OBJ_DIR	  = ./obj/$(CPU)
EXTRA_LIB = -lpthread -lz

## library
LJPEG   = -ljpeg

## libs (shared/static)
PVLIB   = -lPvAPI
SOLIB	= $(EXTRA_LIB) -Bdynamic -lm -lc

## final compilation flags
CFLAGS	= $(OPT) $(FLAGS) -Wall -I$(BST_INC_DIR) -D_REENTRANT $(EXTRA) $(CV_LIB)

## Executable
EXE	= Arrange

## Other Objects
OBJS = $(OBJ_DIR)/IO.o $(OBJ_DIR)/coefs.o
    
sample : $(EXE).cpp $(OBJS)
	$(CC) $(RPATH) $(TARGET) $(CFLAGS) $(EXE).cpp -o $(EXE) $(SOLIB) $(PVLIB) $(BST_SALIB) $(OBJS)

$(OBJ_DIR)/%.o : %.cpp
	$(CC) $(CFLAGS) $(VERSION) -c $< -o $@

clean: 
	rm $(OBJ_DIR)/*
	rm $(EXE)

install:
	cp -f $(EXE) $(BIN_DIR)
