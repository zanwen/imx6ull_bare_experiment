objs := start.o main.o 
prefix := arm-linux-gnueabihf-
gcc := $(prefix)gcc
ld := $(prefix)ld
objcopy := $(prefix)objcopy
objdump := $(prefix)objdump

led.bin : $(objs)
	$(ld) -T imx6ull.lds -o led.elf $^
	$(objcopy) -O binary -S -g led.elf $@
	$(objdump) -D -m arm led.elf > led.dis

%.o : %.c
	$(gcc) -Wall -nostdlib -c -o $@ $<

%.o : %.s
	$(gcc) -Wall -nostdlib -c -o $@ $<

clean:
	rm -rf *.o led.elf led.bin led.dis 