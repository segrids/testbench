objects := scb.o uart.o apdu.o wdt.o rtt.o pmc.o pio.o eefc.o tc.o ivt.o \
	systick.o rstc.o nvic.o twi.o spi.o hdq.o li.o types.o master.o

# object files depending on compiler switch
cond_objects := startup.o slave.o

libram.a : libram.a($(objects) $(cond_objects))

# for libflash use startup.flash.o instead of startup.o:
libflash.a : libflash.a($(objects) $(cond_objects:.o=.flash.o))

# object files to be build with FLASH precompiler switch
%.flash.o : %.c
	$(CC) $(CFLAGS) -DFLASH -c $^ -o $@

%.o : %.s
	$(CC) $(CFLAGS) -c $^ -o $@

%.s : %.c
	$(CC) $(CFLAGS) -S $^ -o $@

.PHONY: clean
clean :
	-$(RM) $(objects) $(cond_objects) $(cond_objects:.o=.flash.o) libram.a libflash.a
