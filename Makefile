MBED = $(firstword $(wildcard mbed mbed-os core .))
TARGET = K64F
TOOLCHAIN = GCC_ARM

SRC ?= $(firstword $(wildcard TESTS/*/*))
BUILD ?= BUILD
BOARD ?= BOARD

VENDOR ?= MBED ARM
VENDOR ?= MBED ARM
BAUD ?= 9600
DEV ?= /dev/$(notdir $(firstword $(foreach v,$(VENDOR), \
               $(shell udevadm trigger -v -n -s block -p ID_VENDOR=$(v)))))
TTY ?= /dev/$(notdir $(firstword $(foreach v,$(VENDOR), \
               $(shell udevadm trigger -v -n -s tty -p ID_VENDOR=$(v)))))

MFLAGS += -j0
ifdef VERBOSE
	MFLAGS += -v
endif
ifdef DEBUG
	#MFLAGS += -o debug-info
	MFLAGS += --profile debug
endif
ifdef SMALL
	#MFLAGS += --profile small
	MFLAGS += --profile release
endif


all build:
	    mkdir -p $(BUILD)/$(TARGET)/$(TOOLCHAIN)
	        echo '*' > $(BUILD)/$(TARGET)/$(TOOLCHAIN)/.mbedignore
		    python $(MBED)/tools/make.py -t $(TOOLCHAIN) -m $(TARGET)   \
			            $(addprefix --source=, . $(SRC))                        \
				            $(addprefix --build=, $(BUILD)/$(TARGET)/$(TOOLCHAIN))  \
					            $(MFLAGS)
doxy-all: doxy-clean doxy doxy-fw
	

doxy:
	doxygen doxyfile_options 

doxy-fw:
	sudo cp -r BUILD/ /media/sf_mbed

doxy-clean:
	rm -r BUILD/
	mkdir BUILD

board:
	    mkdir -p $(BOARD)
	        sudo umount $(BOARD) || true
		    sudo mount $(DEV) $(BOARD)

flash:
	    mkdir -p $(BOARD)
	        sudo umount $(BOARD) || true
		    sudo mount $(DEV) $(BOARD)
		        sudo cp $(BUILD)/$(TARGET)/$(TOOLCHAIN)/*.bin $(BOARD)
			    sync $(BOARD)
			        sudo umount $(BOARD)

tty:
	    sudo screen $(TTY) $(BAUD)

cat:
	    sudo stty -F $(TTY) sane nl $(BAUD)
	        sudo cat $(TTY)

reset:
	    sudo python -c '__import__("serial").Serial("$(TTY)").send_break()'

debug:
	    sudo killall pyocd-gdbserver || true
	        @sleep 0.5
		    sudo pyocd-gdbserver &
		        arm-none-eabi-gdb $(BUILD)/$(TARGET)/$(TOOLCHAIN)/*.elf \
				        -ex "target remote :3333"
			    @sleep 0.5

ctags:
	    ctags $$(find -regex '.*\.\(h\|c\|hpp\|cpp\)')

clean:
	    rm -rf $(BUILD) $(BOARD)
