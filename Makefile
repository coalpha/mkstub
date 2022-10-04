libpath := $(shell vendor/winsdk.exe --type:lib --arch:x64 --kit:um)

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
cflags += -Xlinker /align:16
cflags += -Xlinker /entry:start
cflags += -Xlinker /nodefaultlib
cflags += -Xlinker /subsystem:console
cflags += -Xlinker /libpath:"$(libpath)"

build: bin/mkstub.exe
	-
.PHONY: build

run: bin/mkstub.exe
	$<
.PHONY: run

clean:
	rd /s /q bin

# I'd like template.exe to be subsystem:windows but it fucks with console apps
# if I do.
bin/template.exe: template.c shared.h Makefile
	@-mkdir bin
	clang $< $(cflags)  -o $@
	llvm-strip $@

bin/template.h: bin/template.exe
	wsl xxd -i $< > $@

bin/mkstub.exe: mkstub.c bin/template.h
	clang $< $(cflags) -Xlinker /subsystem:console -o $@
	llvm-strip $@
