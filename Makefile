CC      := gcc
CFLAGS  := -O2 -Wall -Wextra -pedantic
LDFLAGS :=

.PHONY: all clean

all: libadd.a libadd.so test_static test_shared test_dlopen

# ---- static library (.a) ----
add.o: add.c add.h
	$(CC) $(CFLAGS) -c add.c -o add.o

libadd.a: add.o
	ar rcs $@ $^

# ---- shared library (.so) ----
add.pic.o: add.c add.h
	$(CC) $(CFLAGS) -fPIC -c add.c -o add.pic.o

libadd.so: add.pic.o
	$(CC) -shared -o $@ $^

# ---- tests ----
# static link: link directly with .a to force static
test_static: test_static.c add.h libadd.a
	$(CC) $(CFLAGS) test_static.c ./libadd.a -o $@

# shared link at compile-time: -L. -ladd
# add RUNPATH so 실행 시 LD_LIBRARY_PATH 없이도 현재 폴더의 libadd.so를 찾게 함
test_shared: test_shared.c add.h libadd.so
	$(CC) $(CFLAGS) test_shared.c -L. -ladd -Wl,-rpath,'$$ORIGIN' -o $@

# runtime dlopen: must link with -ldl
test_dlopen: test_dlopen.c
	$(CC) $(CFLAGS) test_dlopen.c -ldl -o $@

clean:
	rm -f *.o *.a *.so test_static test_shared test_dlopen
