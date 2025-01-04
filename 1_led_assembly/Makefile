led.bin : led.s
	arm-linux-gnueabihf-gcc -g -c led.s -o led.o
	arm-linux-gnueabihf-ld -Ttext 0x87800000 led.o -o led.elf
	arm-linux-gnueabihf-objcopy -O binary led.elf -S -g led.bin
	arm-linux-gnueabihf-objdump -D led.elf > led.dis

clean:
	rm led.o led.elf led.bin led.dis