# http://www.gnu.org/software/make/manual/make.html#Automatic-Variables

PREFIX ?= arm-none-eabi

export AR = $(PREFIX)-ar
export CC = $(PREFIX)-gcc
export OBJCOPY = $(PREFIX)-objcopy

override CFLAGS += -mcpu=cortex-m0plus
# override CFLAGS += -nostdlib ### nur für libtinycrypto
override CFLAGS += -mthumb
override CFLAGS += -Wall -Wno-main -O0 -gdwarf
# override CFLAGS += -fdata-sections -ffunction-sections

TINYCRYPTOLIBDIR := $(shell pwd)/libtinycrypt
override CFLAGS += -I$(TINYCRYPTOLIBDIR)/include/tinycrypt/
override LDFLAGS += -L$(TINYCRYPTOLIBDIR)

# override LDFLAGS += -nostdlib 
override LDFLAGS += --entry=ivt -gdwarf

export CFLAGS
export LDFLAGS

objects := ivt.o ckcu.o usart.o apdu.o main.o gpio.o crc.o pdma.o i2c.o interface.o crypthandler.o

%.o : %.S
	$(CC) $(CFLAGS) -c $^ -o $@

%.bin : %.elf
	$(OBJCOPY) -O binary $< $@

flash.elf : $(objects)
	$(CC) $(LDFLAGS) -Tlinker-script.ld $(filter %.o, $^) -ltinycrypt -o $@

#python3 flash.py --port=ttyUSB0 ht32/flash.bin
.PHONY: flash
flash : flash.bin
	python3 flash.py flash.bin --port=$(PORT)

.PHONY : modules-%
modules-% : override CFLAGS += -I$(shell pwd)
modules-% :
	$(MAKE) $(@:modules-%=modules/%.bin)

.PRECIOUS : modules/%.elf

modules/%.o : modules/%.s
	arm-none-eabi-gcc $(CFLAGS) -I. -c $^ -o $@

modules/module.elf : modules/interpreter.o modules/ivt.o 
	$(CC) $(LDFLAGS) -Tlinker-script-module.ld \
		-Wl,--just-symbols=flash.elf --entry=thread $^ -o $@

modules/%.s : modules/%.c
	$(CC) $(CFLAGS) -I. -S $^ -o $@

###############

.PHONY: clean
clean :
	-$(RM) $(objects) $(objects:%.o=%.s) $(objects:%.o=%.i) flash.elf ram.elf flash.bin ram.bin modules/module.bin modules/module.elf
