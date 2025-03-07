# http://www.gnu.org/software/make/manual/make.html#Automatic-Variables

PREFIX ?= arm-none-eabi

export AR = $(PREFIX)-ar
export CC = $(PREFIX)-gcc
export LD = $(PREFIX)-ld
export OBJCOPY = $(PREFIX)-objcopy

LIBDIR := $(shell pwd)/lib
override CFLAGS += -I$(LIBDIR)

override CFLAGS += -mcpu=cortex-m0plus
override CFLAGS += -nostdlib
override CFLAGS += -mthumb
override CFLAGS += -Wall -Wno-main -O0 -gdwarf -Wall -Wextra -pedantic-errors
# override CFLAGS += -fdata-sections -ffunction-sections

override LDFLAGS += -L$(LIBDIR) -gdwarf -Wl,--gc-sections 
#-Wall -Wextra -pedantic-errors

#TINYCRYPTOLIBDIR := $(shell pwd)/libtinycrypt
#override CFLAGS += -I$(TINYCRYPTOLIBDIR)/include/tinycrypt/
#override LDFLAGS += -L$(TINYCRYPTOLIBDIR)

override LDFLAGS += -nostdlib 
override LDFLAGS += -nostartfiles

export CFLAGS
export LDFLAGS

#objects := ivt.o ckcu.o usart.o apdu.o main.o gpio.o crc.o pdma.o i2c.o interface.o
objects := ivt.o main.o

%.o : %.S
	$(CC) $(CFLAGS) -c $^ -o $@

%.bin : %.elf
	$(OBJCOPY) -O binary $< $@


.PHONY: libflash
libflash :
	cd ./lib && $(MAKE) libflash.a

.PHONY: lib-%
lib-% :
	$(MAKE) -C lib $(@:lib-%=%)


flash.elf : $(objects) libflash
	$(LD) $(LDFLAGS) -T./linker-script-flash.ld $(filter %.o, $^) -lflash -o $@

#python3 flash.py --port=ttyUSB0 ht32/flash.bin
.PHONY: flash
flash : flash.bin
	python3 flash.py flash.bin --adapter=$(ADAPTER)


#######

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
