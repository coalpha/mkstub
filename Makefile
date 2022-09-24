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
cflags += -Xlinker /libpath:"$(libpath)"

build: bin/mkstub.exe
	-
.PHONY: build

clean:
	rd /s /q bin

bin/template.exe: template.c
	@-mkdir bin
	clang $< $(cflags) -Xlinker /subsystem:windows -o $@
	llvm-strip $@

bin/template.h: bin/template.exe
	wsl xxd -i $< > $@

bin/mkstub.exe: mkstub.c bin/template.h
	clang $< $(cflags) -Xlinker /subsystem:console -o $@
	llvm-strip $@
