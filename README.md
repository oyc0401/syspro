# libadd - simple Linux utility library

## Description
A minimal C library that provides one function:
- `int add(int a, int b);`

This project demonstrates:
- Building a static library (.a)
- Building a shared library (.so)
- Using the shared library with runtime dynamic loading (dlopen/dlsym)

## Files
- `add.c`, `add.h` : library source/header
- `test_static.c`  : static linking test
- `test_shared.c`  : shared linking test (compile-time linking)
- `test_dlopen.c`  : runtime loading test using dlopen/dlsym
- `Makefile`

## Build
```bash
make clean
make
````

## Run tests

### 1) Static library test

```bash
./test_static
```

Expected output:

```
add(2, 3) = 5
```

### 2) Shared library test (compile-time linking)

```bash
./test_shared
```

Expected output:

```
add(10, 20) = 30
```

Note: `test_shared` is built with RUNPATH using `-Wl,-rpath,'$ORIGIN'`,
so it can find `./libadd.so` without setting `LD_LIBRARY_PATH`.

### 3) Runtime dynamic loading test (dlopen/dlsym)

```bash
./test_dlopen
```

Expected output:

```
add(7, 8) = 15
```

## Screenshot


