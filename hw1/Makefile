CC ?= gcc
LD ?= gcc

CFLAGS := -g -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding \
	-mcmodel=kernel -Wall -Wextra -Werror -std=c99 \
	-Wframe-larger-than=4096 -Wstack-usage=4096 -Wno-unknown-warning-option -Wno-unused-variable
LFLAGS := -nostdlib -z max-page-size=0x1000

ASM := bootstrap.S videomem.S handlers.S
AOBJ:= $(ASM:.S=.o)
ADEP:= $(ASM:.S=.d)

SRC := main.c serial_port.c interrupt.c pit.c
OBJ := $(AOBJ) $(SRC:.c=.o)
DEP := $(ADEP) $(SRC:.c=.d)

all: kernel

kernel: $(OBJ) kernel.ld
	$(LD) $(LFLAGS) -T kernel.ld -o $@ $(OBJ)

%.o: %.S
	$(CC) -D__ASM_FILE__ -g -MMD -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(DEP)

.PHONY: clean
clean:
	rm -f kernel $(OBJ) $(DEP)
