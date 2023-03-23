# PROGS := $(wildcard tests/uart-test-lib.c)
PROGS := $(wildcard tests/uart-test-wifi*.c)
PROGS := $(wildcard tests/uart-test*.c)

LIBESP_PATH=../libesp
LIB_ESP=$(LIBESP_PATH)/libesp-pi.a

CFLAGS = -I$(LIBESP_PATH) -DRPI_OS
# CFLAGS = -I$(LIBESP_PATH) -DRPI_UNIX
# LIBS = -lutil $(LIB_ESP)

# DEPS += $(LIB_ESP)

# COMMON_SRC := esp-helpers.c 
# COMMON_SRC += ../libesp/esp-parse.c  
# COMMON_SRC += sw-uart.c sw-uart-veneer.c

# STAFF_OBJS += $(CS140E_2023_PATH)/libpi/staff-objs/new-spi.o 
STAFF_OBJS += $(CS140E_2023_PATH)/libpi/staff-objs/sw-uart.o
# STAFF_OBJS += staff-nrf-driver.o

COMMON_SRC += $(LIBESP_PATH)/esp-commands.c 
COMMON_SRC += $(LIBESP_PATH)/esp-lex.c 
COMMON_SRC += $(LIBESP_PATH)/esp-run.c 
COMMON_SRC += $(LIBESP_PATH)/esp-lex-simple.c 
COMMON_SRC += $(LIBESP_PATH)/esp-parse.c 
COMMON_SRC += $(LIBESP_PATH)/pi-support.c 

# uncomment if you want it to automatically run.
RUN=1

# define this if you need to give the device for your pi
BOOTLOADER = my-install

# GREP_STR := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
EXCLUDE ?= grep -v simple_boot
# libs::
# 	make -C $(LIBESP_PATH)

# test-run:
# 	my-install ./tests/uart-test-lib.bin
include $(CS140E_2023_PATH)/libpi/mk/Makefile.template-v2
