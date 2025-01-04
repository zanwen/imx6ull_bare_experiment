OBJS := start.o main.o 
TOOLCHAIN_PREFIX := arm-linux-gnueabihf-
GCC := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld 
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
OBJDUMP := $(TOOLCHAIN_PREFIX)objdump

led.bin : $(OBJS) # 从.o文件生成.bin文件
	$(LD) -T imx6ull.lds $^ -o led.elf 			
	$(OBJCOPY) -O binary -S -g led.elf $@ 		
	$(OBJDUMP) -D -m arm led.elf > led.dis		

%.o: %.c # 将.c编译成.o
	$(GCC) -Wall -nostdlib -O2 -c $< -o $@ 	

%.o: %.s # 将.s编译成.o
	$(GCC) -Wall -nostdlib -O2 -c $< -o $@ 	


clean:
	rm -rf *.o led.elf led.bin led.dis


