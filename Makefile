NASM = nasm
AR = ar

SRC = src/cpuid_check.asm
OBJ = build/cpu_check.o
LIB = build/libcpuinfo.a

PREFIX = /usr/local

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $(LIB) $(OBJ)

$(OBJ): $(SRC)
	mkdir -p build
	$(NASM) -f elf64 $(SRC) -o $(OBJ)

install: $(LIB)
	install -Dm644 include/cpuinfo.h $(PREFIX)/include/ci_include/cpuinfo.h
	install -Dm644 $(LIB) $(PREFIX)/lib/libcpuinfo.a

uninstall:
	rm -f $(PREFIX)/include/ci_include/cpuinfo.h
	rm -f $(PREFIX)/lib/libcpuinfo.a

clean:
	rm -rf build

.PHONY: all install uninstall clean