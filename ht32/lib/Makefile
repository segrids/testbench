objects := apdu.o  ckcu.o  crc.o  gpio.o  i2c.o  interface.o  ivt.o  pdma.o usart.o startup.o

libflash.a : libflash.a( $(objects) )

# object files to be build with FLASH precompiler switch
%.flash.o : %.c
	$(CC) $(CFLAGS) -DFLASH -c $^ -o $@

%.o : %.s
	$(CC) $(CFLAGS) -c $^ -o $@

%.s : %.c
	$(CC) $(CFLAGS) -S $^ -o $@

.PHONY: clean
clean :
	-$(RM) $(objects) libflash.a
