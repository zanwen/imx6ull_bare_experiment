CROSS_COMPILE 		?= arm-linux-gnueabihf-
TARGET 				?= epit

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
					   project 				\
					   bsp					\
					   bsp/led				\
					   bsp/beep				\
					   bsp/key				\
					   bsp/delay

# 所有源文件路径
SRC_DIRS			:= sdk					\
					   driver/clk			\
					   driver/gpio			\
					   driver/interrupt		\
					   driver/epit			\
					   driver/exti			\
					   project 				\
					   bsp/led				\
					   bsp/beep				\
					   bsp/key				\
					   bsp/delay

# 目标文件路径
OBJ_DIR			    := obj

# 头文件路径编译参数
INCLUDE_FLAGS		:= $(patsubst %, -I %, $(INCLUDE_DIRS))
C_FLAGS 			:= -Wall -nostdlib -O2 -std=c99 $(INCLUDE_FLAGS)

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

.PHONY : clean


# 链接
$(TARGET).bin : $(OBJS)
	$(LD) -T imx6ull.lds $^ -o $(TARGET).elf
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

# 编译
$(S_OBJS) : $(OBJ_DIR)/%.o : %.S
	$(CC) $(C_FLAGS) -c $< -o $@

$(C_OBJS) : $(OBJ_DIR)/%.o : %.c
	$(CC) $(C_FLAGS) -c $< -o $@


clean: 
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).dis $(C_OBJS) $(S_OBJS)


print:
	@echo $(INCLUDE_FLAGS)
	@echo $(S_FILES)
	@echo $(C_FILES)
	@echo $(S_FILES_NO_DIR)
	@echo $(C_FILES_NO_DIR)
	@echo $(S_OBJS)
	@echo $(C_OBJS)
	@echo $(OBJS)