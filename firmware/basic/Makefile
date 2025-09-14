CC=clang
BIN := firmware.elf
BUILD_DIR := build
#Possible values Flash, StreamBuffer, RECM, QSET, CR, INT
#Possible Baremetal demos include LwIP, Animation, SD
GCC_INSTALL ?= /data/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/
GCC_INSTALL_INCLUDE = $(GCC_INSTALL)arm-none-eabi/include/
CHECKED_C_INSTALL_INCLUDE ?=/data/checkedc/include
GCC_INSTALL_LIB ?= $(GCC_INSTALL)arm-none-eabi/lib/thumb/v7-m/nofp/
BSP_INSTALL ?= ./32f429idiscovery-bsp/
HAL_INSTALL ?= ./stm32f4xx_hal_driver/




SOURCE_FILES += main.c 

#Finalize configuration variables based on project/demo
CFLAGS = ${CFLAGS_${PROJECT}_${DEMO}} -O0 -nostdlib -DSTM32F429xx 
#LDFLAGS = ${LDFLAGS_${PROJECT}_${DEMO}} //Pretty much every configuration uses the same ld flags, can extend in future.
LDFLAGS = -v -T ./scripts/stm32_flash.ld  -specs=nano.specs --specs=rdimon.specs #-lc -lrdimon
LDFLAGS += -Xlinker -Map=${BUILD_DIR}/output.map

LDFLAGS += -L$(GCC_INSTALL_LIB) -L./ -v
#LDFLAGS +=  -L./ -v
INCLUDE_DIRS = ${INCLUDE_DIRS_${PROJECT}_${DEMO}} -I $(PIECES)/partitioner/monitor/include/ -I $(GCC_INSTALL_INCLUDE) -I $(CHECKED_C_INSTALL_INCLUDE)
#Device Path
INCLUDE_DIRS += -I$(CMSIS_PATH)/Device/cmsis_device_f4/Include/
INCLUDE_DIRS += -I$(CMSIS_PATH)/CMSIS/Core/Include/
INCLUDE_DIRS += -I$(BSP_INSTALL)/
INCLUDE_DIRS += -I$(HAL_INSTALL)/Inc/






#Prepare variables for build rules
OBJ_FILES := $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)
CFLAGS += $(INCLUDE_DIRS)

CFLAGS += -flto=thin --target=arm-none-eabi -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -g 

CFLAGS += -DTRAIN



.PHONY: clean

$(BUILD_DIR)/$(BIN) : $(OBJ_FILES) 
	$(CC) -ffunction-sections -fdata-sections $(CFLAGS) $(LDFLAGS) $+ -o $(@)

%.d: %.c
	@set -e; rm -f $@; \
    $(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
    rm -f $@.$$$$

INCLUDES := $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.d)
-include $(INCLUDES)

${BUILD_DIR}/%.o : %.c Makefile
	-mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -c $< -o $@
	echo $(CFLAGS) > .cflags

clean:
	-rm -rf build

bc: $(OBJ_FILES) 
	$(warning Files are $(OBJ_FILES))
	llvm-link  -v $+ -o $(BUILD_DIR)/$(BIN).bc
	llvm-dis $(BUILD_DIR)/$(BIN).bc

exec: $(BUILD_DIR)/$(BIN).bc 
	llc -filetype=obj $(BUILD_DIR)/$(BIN).bc
	$(CC)  --target=arm-none-eabi -mthumb -mcpu=cortex-m4 $(LDFLAGS)  -Xlinker -Map=./build/rtmk.map $(BUILD_DIR)/$(BIN).bc  -o $(BUILD_DIR)/$(BIN)
