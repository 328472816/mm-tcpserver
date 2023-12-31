ARCH ?= x86

ifeq ($(ARCH),x86)
		CC=gcc
else
		CC=arm-linux-gnueabihf-gcc
endif

TARGET=mm-tcpserver
#OBJS=main.o mp3.o
BUILD_DIR= build
SRC_DIR= src
INC_DIR= include
CFLAGS=$(patsubst %,-I%,$(INC_DIR))
INCLUDES=$(foreach dir,$(INC_DIR),$(wildcard $(patsubst)/*.h))

SOURCES=$(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c))
OBJS=$(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))
VPATH=$(SRC_DIR)

$(BUILD_DIR)/$(TARGET):$(OBJS)
		$(CC) $^ -o $@ $(CFLAGS) -lpthread -lm

#main.o:
#	gcc -c main.c -o main.o

#mp3.o:
#	gcc -c mp3.c -o mp3.o

$(BUILD_DIR)/%.o:%.c $(INCLUDES) | create_build
		$(CC) -c $< -o $@ $(CFLAGS) -lpthread -lm

.PHONY:clean create_build

clean:
	rm -r $(BUILD_DIR)

create_build:
	mkdir -p $(BUILD_DIR)
