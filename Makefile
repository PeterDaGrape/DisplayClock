CROSS_COMPILE := aarch64-rpi3-linux-gnu-
SYSROOT := /Volumes/crosstool-NG/Prefixes/sysroot
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
AS := $(CROSS_COMPILE)as

include .env

DIR_BIN 	 = ./bin
DIR_SRC      = ./src
OBJ_DIR 	 = ./obj

DEPLOY_HOST := petervine@piclock.local
DEPLOY_DIR := /home/petervine/PiClock

#Lib dirs
DIR_Config   = ./lib/Config
DIR_Driver	 = ./lib/Driver
DIR_GUI		 = ./lib/GUI
DIR_FONTS    = ./lib/Fonts
DIR_EPD		 = ./lib/EPD
DIR_DISPLAY_HANDLER		 = ./lib/DisplayHandler


DIR_RSC 	 = rsc
DIR_LIB      = ./lib

OBJ_C = $(wildcard ${DIR_SRC}/*.c ${DIR_Driver}/*.c ${DIR_GUI}/*.c ${DIR_EPD}/*.c ${DIR_Config}/*.c ${DIR_DISPLAY_HANDLER}/*.c ${DIR_FONTS}/*.c )
OBJ_O = $(patsubst %.c,${OBJ_DIR}/%.o,$(notdir ${OBJ_C}))




TARGET       = main

USELIB = USE_LGPIO_LIB
DEBUG = -D $(USELIB)

ifeq ($(USELIB), USE_BCM2835_LIB)
    LIB = -lbcm2835 -lm 
	OBJ_O := $(filter-out ${OBJ_DIR}/RPI_gpiod.o ${OBJ_DIR}/dev_hardware_SPI.o ${OBJ_DIR}/dev_hardware_i2c.o, ${OBJ_O})
else ifeq ($(USELIB), USE_WIRINGPI_LIB)
    LIB = -lwiringPi -lm 
	OBJ_O := $(filter-out ${OBJ_DIR}/RPI_gpiod.o ${OBJ_DIR}/dev_hardware_SPI.o ${OBJ_DIR}/dev_hardware_i2c.o, ${OBJ_O})
else ifeq ($(USELIB), USE_LGPIO_LIB)
	LIB += -llgpio -lm 
	OBJ_O := $(filter-out ${OBJ_DIR}/RPI_gpiod.o ${OBJ_DIR}/dev_hardware_SPI.o ${OBJ_DIR}/dev_hardware_i2c.o, ${OBJ_O})
else ifeq ($(USELIB), USE_GPIOD_LIB)
    LIB = -lgpiod -lm
endif
LIB += -lpthread
LIB += -lcurl
LIB += -ljson-c


# Compiler flags
MSG          = -g -O0 -Wall
CFLAGS      += $(MSG) $(DEBUG)
CFLAGS      += --sysroot=$(SYSROOT) -I$(SYSROOT)/usr/include -I$(SYSROOT)/usr/local/include
CFLAGS 		+= -I$(SYSROOT)/usr/include/aarch64-linux-gnu/curl
CFLAGS 		+= -I$(SYSROOT)/usr/include/json-c
CFLAGS 		+= -DRSC_PATH="\"$(DEPLOY_DIR)/$(DIR_RSC)\""
CFLAGS 		+= -DAPI_KEY=\"$(API_KEY)\"
LDFLAGS		+= --sysroot=$(SYSROOT) \
            -L$(SYSROOT)/usr/lib \
            -L$(SYSROOT)/usr/local/lib \
            -L$(SYSROOT)/usr/lib/aarch64-linux-gnu \
            -Wl,-rpath-link=$(SYSROOT)/usr/lib/aarch64-linux-gnu
# Create directories if they don't exist
$(shell mkdir -p $(OBJ_DIR) $(DIR_BIN))


# Build target
$(DIR_BIN)/$(TARGET): $(OBJ_O)
	$(CC) $(CFLAGS) $(OBJ_O) -o $@ $(LDFLAGS) $(LIB)

${OBJ_DIR}/%.o:$(DIR_SRC)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config) -I $(DIR_Driver) -I $(DIR_FONTS)  -I $(DIR_EPD) -I $(DIR_GUI) -I $(DIR_DISPLAY_HANDLER) 

    
${OBJ_DIR}/%.o:$(DIR_Driver)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config) $(DEBUG)

${OBJ_DIR}/%.o:$(DIR_EPD)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config) -I$(DIR_EPD) $(DEBUG)


${OBJ_DIR}/%.o:$(DIR_DISPLAY_HANDLER)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config) -I $(DIR_EPD) -I $(DIR_GUI) -I $(DIR_FONTS) -I $(DIR_Driver) $(DEBUG) 
    
${OBJ_DIR}/%.o:$(DIR_FONTS)/%.c 
	$(CC) $(CFLAGS) -c  $< -o $@ $(DEBUG)
	
${OBJ_DIR}/%.o:$(DIR_GUI)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ -I $(DIR_Config) $(DEBUG)
	
${OBJ_DIR}/%.o:$(DIR_Config)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB)



# Clean up
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)/* $(DIR_BIN)/$(TARGET)


deploy: $(DIR_BIN)/$(TARGET) kill
	scp $(DIR_BIN)/$(TARGET) $(DEPLOY_HOST):$(DEPLOY_DIR)/

	
	rsync -avz $(DIR_RSC) $(DEPLOY_HOST):$(DEPLOY_DIR)/ 
	rsync -avz $(DIR_SRC) $(DEPLOY_HOST):$(DEPLOY_DIR)/ 
	rsync -avz $(DIR_LIB) $(DEPLOY_HOST):$(DEPLOY_DIR)/ 



kill: 
	ssh $(DEPLOY_HOST) 'killall $(TARGET) || true'

run: deploy
	ssh $(DEPLOY_HOST) 'cd $(DEPLOY_DIR); nohup ./$(TARGET) > /dev/null 2>&1 &'

restart: kill deploy run

.PHONY: all clean
