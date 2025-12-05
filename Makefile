SRCDIR = src
INCDIR = include
RESDIR = res
OUTDIR = out
OBJDIR = $(OUTDIR)/objects

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CFLAGS = -DDEBUG -Wall -O2 -ffreestanding  -nostdlib -mcpu=cortex-a72+nosimd -I $(INCDIR)

all: clean $(OUTDIR)/kernel8.img

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OBJDIR)/start.o: $(SRCDIR)/start.S | $(OBJDIR)
	clang --target=aarch64-elf $(CFLAGS) -c $(SRCDIR)/start.S -o $(OBJDIR)/start.o

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	clang --target=aarch64-elf $(CFLAGS) -c $< -o $@

$(OBJDIR)/font_psf.o : $(RESDIR)/font.psf | $(OBJDIR)
	ld.lld -m aarch64elf -r -b binary -o $(OBJDIR)/font_psf.o $(RESDIR)/font.psf

$(OUTDIR)/kernel8.img: $(OBJDIR)/start.o $(OBJDIR)/font_psf.o $(OBJS) | $(OUTDIR)
	ld.lld -m aarch64elf -nostdlib $(OBJDIR)/start.o $(OBJDIR)/font_psf.o $(OBJS) -T link.ld -o $(OUTDIR)/kernel8.elf
	llvm-objcopy -O binary $(OUTDIR)/kernel8.elf $(OUTDIR)/kernel8.img

clean:
	rm -rf $(OUTDIR) >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi4b -kernel $(OUTDIR)/kernel8.img -serial stdio -drive file=test.dd,if=sd,format=raw