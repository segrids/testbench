objects := scb.o uart.o apdu.o interface.o wdt.o rtt.o pmc.o pio.o eefc.o tc.o ivt.o \
	systick.o rstc.o nvic.o twi.o spi.o hdq.o types.o

cond_objects := startup.o

libflash.a : libflash.a($(objects) $(cond_objects:.o=.flash.o))

libram.a : libram.a($(objects) $(cond_objects))

%.flash.o : %.c
	$(CC) $(CFLAGS) -DFLASH -c $^ -o $@

%.o : %.s
	arm-none-eabi-gcc $(CFLAGS) -c $^ -o $@

%.s : %.c
	$(CC) $(CFLAGS) -S $^ -o $@

.PHONY: clean
clean :
	-$(RM) $(objects) $(cond_objects) $(cond_objects:.o=.flash.o) libflash.a \
		libram.a
