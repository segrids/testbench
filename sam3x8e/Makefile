export PORT ?= ttyACM1
PREFIX ?= arm-none-eabi

export AR = $(PREFIX)-ar
export CC = $(PREFIX)-gcc
export OBJCOPY = $(PREFIX)-objcopy

LIBDIR := $(shell pwd)/lib
override CFLAGS += -I$(LIBDIR)

override CFLAGS += -mcpu=cortex-m3

override CFLAGS += -nostdlib -mthumb
override CFLAGS += -Wall -Wno-main -O0 -gdwarf
#override CFLAGS += -fdata-sections -ffunction-sections

override LDFLAGS += -nostdlib --entry=ivt -L$(LIBDIR) -gdwarf -Wl,--gc-sections

export CFLAGS
export LDFLAGS

objects := swd.o blake2s.o utils.o loader.o 
cond_objects := main.o cinterrupts.o asminterrupts.o 
optional_objects := debugger.o adapter.o tester.o

%.o : %.S
	$(CC) $(CFLAGS) -c $^ -o $@

%.bootloader.o : %.c
	$(CC) $(CFLAGS) -Os -DBOOTLOADER -c -o $@ $^

%.bootloader.o : %.S
	$(CC) $(CFLAGS) -Os -DBOOTLOADER -c -o $@ $^

%.bin : %.elf
	$(OBJCOPY) -O binary $< $@

.PHONY: libflash
libflash :
	cd ./lib && $(MAKE) libflash.a

.PHONY: lib-%
lib-% :
	$(MAKE) -C lib $(@:lib-%=%)


.PHONY: libram
libram :
	cd ./lib && $(MAKE) libram.a

flash.elf : $(objects) $(cond_objects) $(optional_objects) libflash
	$(CC) $(LDFLAGS) -T./ld/linker-script-flash.ld $(filter %.o, $^) -lflash -o $@


bootloader.elf : $(objects) $(cond_objects:%.o=%.bootloader.o) libram
	$(CC) $(LDFLAGS) -Os -T./ld/linker-script-ram.ld $(filter %.o, $^) -lram -o $@

.PHONY : modules-%
modules-% : override CFLAGS += -I$(shell pwd)
modules-% :
	$(MAKE) $(@:modules-%=modules/%.bin)

.PRECIOUS : modules/%.elf

modules/%.o : modules/%.s
	arm-none-eabi-gcc $(CFLAGS) -I. -c $^ -o $@

modules/module.elf : modules/interpreter.o modules/ivt.o
	$(CC) $(LDFLAGS) -Tld/linker-script-module.ld \
		-Wl,--just-symbols=flash.elf --entry=thread $^ -o $@

modules/%.s : modules/%.c
	$(CC) $(CFLAGS) -I. -S $^ -o $@

.PHONY: erase
erase :
	stty 1200 -F /dev/$(PORT)

# python3 flash.py --target=SAM3x8e --port=ttyACM2 sam3x8e/flash.bin
.PHONY: flash
flash : flash.bin erase
	../flash.py --erase --port=$(PORT) --verify --reset --bootloader=bootloader.bin $<

.PHONY: clean
clean :
	-$(RM) $(modules:%=modules/%.o) $(modules:%=modules/%.elf) \
		$(modules:%=modules/%.bin) $(objects) $(cond_objects) $(optional_objects)\
		$(cond_objects:%.o=%.bootloader.o) flash.elf flash.bin bootloader.elf bootloader.bin
	$(MAKE) -C lib clean
