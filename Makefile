export PORT_DEBUG ?= ttyACM0
PREFIX ?= arm-none-eabi

export AR = $(PREFIX)-ar
export CC = $(PREFIX)-gcc
export OBJCOPY = $(PREFIX)-objcopy

LIBDIR := $(shell pwd)/lib

##override CFLAGS += -nostdlib -mthumb -ffunction-sections
override CFLAGS += -mthumb -ffunction-sections
override CFLAGS += -Wall -Wno-main -O0 -gdwarf
# override CFLAGS += -fdata-sections

# override LDFLAGS += -nostdlib 
override LDFLAGS += --entry=ivt -L$(LIBDIR) 
#override LDFLAGS += -L$(LIBDIR)

export CFLAGS
export LDFLAGS

.PHONY: sam3x8e-%
sam3x8e-% :
	cd sam3x8e && $(MAKE) $(@:sam3x8e-%=%)

.PHONY: ht32-%
ht32-% :
	cd ht32 && $(MAKE) $(@:ht32-%=%)

.PHONY: bootloader
bootloader : sam3x8e-bootloader.bin

.PHONY: clean
clean :
	$(MAKE) -C sam3x8e clean
	rm -rf py/__pycache__/
	rm -rf py/sam3x8e/__pycache__/
	rm -rf py/debug/__pycache__/

.PHONY: test
test : test-unit test-integration

.PHONY: test-unit
test-unit :
	python3 -m pytest --cov=testbench -v -k tests/unit
	python3 -m coverage html

.PHONY: test-integration
test-integration :
	pytest-3 -v -k tests/integration
