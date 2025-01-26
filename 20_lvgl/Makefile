CROSS_COMPILE 		?=  /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
TARGET 				?= lvgl

CC					:= $(CROSS_COMPILE)gcc
LD					:= $(CROSS_COMPILE)ld
OBJCOPY				:= $(CROSS_COMPILE)objcopy
OBJDUMP				:= $(CROSS_COMPILE)objdump

# 所有头文件路径
INCLUDE_DIRS 		:= sdk					\
					   driver/clk			\
					   driver/gpio			\
					   driver/interrupt		\
					   driver/exti			\
					   driver/epit			\
					   driver/delay			\
					   driver/uart			\
					   driver/i2c			\
					   project 				\
					   bsp					\
					   bsp/led				\
					   bsp/beep				\
					   bsp/key				\
					   bsp/lcd				\
					   bsp/rtc				\
					   bsp/logger			\
					   bsp/key_filter		\
					   bsp/gt911			\
					   bsp/touch			\
					   bsp/ov5640			\
					   lvgl/examples/porting			\
					   lvgl/src							\
					   lvgl								\
					   lvgl/demos								\
					   lvgl/demos/stress								\
					   lvgl/demos/music								\
					   stdio/include

# 所有源文件路径
SRC_DIRS			:= sdk					\
					   driver/clk			\
					   driver/gpio			\
					   driver/interrupt		\
					   driver/epit			\
					   driver/exti			\
					   driver/delay			\
					   driver/uart			\
					   driver/i2c			\
					   project 				\
					   bsp/led				\
					   bsp/beep				\
					   bsp/key				\
					   bsp/lcd				\
					   bsp/rtc				\
					   bsp/logger			\
					   bsp/key_filter		\
					   bsp/gt911			\
					   bsp/touch			\
					   bsp/ov5640			\
					   lvgl/examples/porting						\
					   lvgl/src/core								\
					   lvgl/src/draw								\
					   lvgl/src/draw/sw								\
					   lvgl/src/extra								\
					   lvgl/src/extra/layouts/flex					\
					   lvgl/src/extra/layouts/grid					\
					   lvgl/src/extra/libs/bmp						\
					   lvgl/src/extra/libs/ffmpeg						\
					   lvgl/src/extra/libs/freetype						\
					   lvgl/src/extra/libs/fsdrv						\
					   lvgl/src/extra/libs/gif						\
					   lvgl/src/extra/libs/png						\
					   lvgl/src/extra/libs/rlottie						\
					   lvgl/src/extra/libs/sjpg						\
					   lvgl/src/extra/others/gridnav							\
					   lvgl/src/extra/others/monkey							\
					   lvgl/src/extra/others/snapshot							\
					   lvgl/src/extra/themes/basic							\
					   lvgl/src/extra/themes/default							\
					   lvgl/src/extra/themes/mono							\
					   lvgl/src/extra/widgets/animimg							\
					   lvgl/src/extra/widgets/calendar				\
                       lvgl/src/extra/widgets/chart				\
                       lvgl/src/extra/widgets/colorwheel				\
                       lvgl/src/extra/widgets/imgbtn				\
                       lvgl/src/extra/widgets/keyboard				\
                       lvgl/src/extra/widgets/led				\
                       lvgl/src/extra/widgets/list				\
                       lvgl/src/extra/widgets/menu				\
                       lvgl/src/extra/widgets/meter				\
                       lvgl/src/extra/widgets/msgbox				\
                       lvgl/src/extra/widgets/span				\
                       lvgl/src/extra/widgets/spinbox				\
                       lvgl/src/extra/widgets/spinner				\
                       lvgl/src/extra/widgets/tabview				\
                       lvgl/src/extra/widgets/tileview				\
                       lvgl/src/extra/widgets/win				\
					   lvgl/src/font								\
					   lvgl/src/draw/sdl							\
					   lvgl/src/draw/stm32_dma2d 					\
					   lvgl/src/hal				 					\
					   lvgl/src/misc				 				\
					   lvgl/src/widgets				 				\
					   lvgl/demos/stress								\
					   lvgl/demos/music								\
					   lvgl/demos/music/assets								\
					   stdio/lib

# 目标文件路径
OBJ_DIR			    := obj

# 头文件路径编译参数
INCLUDE_FLAGS		:= $(patsubst %, -I %, $(INCLUDE_DIRS))
LIBPATH			    := -lgcc -L /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4

# 遍历源文件路径，找出所有.s和.o文件
S_FILES			    := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.S))
C_FILES			    := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))

# 将所有.s和.o文件名称替换为.o文件
# 1. 去掉目录前缀
S_FILES_NO_DIR 		:= $(notdir $(S_FILES))
C_FILES_NO_DIR 		:= $(notdir $(C_FILES))
# 2. 替换扩展名为.o，并增加obj存放路径前缀
S_OBJS				:= $(patsubst %, $(OBJ_DIR)/%, $(S_FILES_NO_DIR:.S=.o))
C_OBJS				:= $(patsubst %, $(OBJ_DIR)/%, $(C_FILES_NO_DIR:.c=.o))
OBJS			    := $(S_OBJS) $(C_OBJS)

VPATH				:= $(SRC_DIRS)

.PHONY : all clean d

# 链接
all : $(OBJS)
	$(LD) -T imx6ull.lds $^ -o $(TARGET).elf $(LIBPATH)
	$(OBJCOPY) -O binary -S $(TARGET).elf $(TARGET).bin
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

# 编译
$(S_OBJS) : $(OBJ_DIR)/%.o : %.S
	$(CC) -Wall -DNDEBUG -nostdlib -fno-builtin -c -O2 $(INCLUDE_FLAGS) -c $< -o $@

$(C_OBJS) : $(OBJ_DIR)/%.o : %.c
	$(CC) -Wall -Wa,-mimplicit-it=thumb -DNDEBUG -nostdlib -fno-builtin -O2 -std=gnu99 $(INCLUDE_FLAGS) -c $< -o $@

d:
	imxdownload $(TARGET).bin /dev/sdd

clean: 
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).dis $(C_OBJS) $(S_OBJS) load.imx


print:
	@echo $(INCLUDE_FLAGS)
	@echo $(S_FILES)
	@echo $(C_FILES)
	@echo $(S_FILES_NO_DIR)
	@echo $(C_FILES_NO_DIR)
	@echo $(S_OBJS)
	@echo $(C_OBJS)
	@echo $(OBJS)