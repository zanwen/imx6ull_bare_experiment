CROSS_COMPILE 	?= arm-linux-gnueabihf-
NAME 			?= led_sdk

CC				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY			:= $(CROSS_COMPILE)objcopy
OBJDUMP			:= $(CROSS_COMPILE)objdump

OJBS 			:= start.o main.o

$(NAME).bin: $(OJBS)
	$(LD) -Timx6ull.lds $^ -o $(NAME).elf
	$(OBJCOPY) -O binary -S $(NAME).elf $@
	$(OBJDUMP) -D -m arm $(NAME).elf > $(NAME).dis

%.o: %.c 
	$(CC) -Wall -nostdlib -O2 -c $< -o $@

%.o: %.s
	$(CC) -Wall -nostdlib -O2 -c $< -o $@

clean:
	rm -rf *.o $(NAME).elf $(NAME).bin $(NAME).dis
