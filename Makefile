libpath := $(shell misc/winsdk.exe --type:lib --arch:x64 --kit:um)

cflags += -Oz
cflags += -std=c11
cflags += -Wall
cflags += -Wextra
cflags += -pedantic
cflags += -nostdlib
cflags += -ffreestanding
cflags += -fno-stack-check
cflags += -fno-stack-protector
cflags += -mno-stack-arg-probe
cflags += -lkernel32
cflags += -luser32
cflags += -fuse-ld=lld
# cflags += -Xlinker /align:16
cflags += -Xlinker /entry:start
cflags += -Xlinker /nodefaultlib
cflags += -Xlinker /subsystem:console
cflags += -Xlinker /libpath:"$(libpath)"

build: bin/green.exe
	$<
.PHONY: build

bin/shrub.exe: shrub.c
	@-mkdir bin
	clang $< $(cflags) -o $@
	llvm-strip $@

bin/shrub.h: bin/shrub.exe
	wsl xxd -i $< > $@

bin/green.exe: green.c bin/shrub.h
	clang $< $(cflags) -o $@
	llvm-strip $@
