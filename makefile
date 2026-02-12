#############################################
# User paths
#############################################

C2000WARE = /opt/ti/c2000/C2000Ware_5_01_00_00
COMPILER  = /opt/ti/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS

DRIVERLIB = $(C2000WARE)/driverlib/f28003x/driverlib
DEVICEINC = $(C2000WARE)/device_support/f28003x/common/include
DEVICESRC = $(C2000WARE)/device_support/f28003x/common/source

#############################################

CC = $(COMPILER)/bin/cl2000

CFLAGS = -v28 \
         -ml \
         --float_support=fpu32 \
         -I$(DRIVERLIB) \
         -I$(DEVICEINC) \
         -O2

LDFLAGS = \
         -z \
         --stack_size=0x400 \
         --heap_size=0x400 \
         -i$(COMPILER)/lib \
         rts2800_fpu32.lib

#############################################

all: sci_test.out

sci_test.out: main.obj
	$(CC) $(LDFLAGS) -o $@ $^ F28003x_generic_ram_lnk.cmd \
        $(DRIVERLIB)/driverlib.lib

main.obj: main.c
	$(CC) $(CFLAGS) --compile_only $<

clean:
	rm -f *.obj *.out
