// test_dlopen.c
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef char* (*big_fn)(const char*, const char*);

int main(void) {
    void* handle;
    big_fn big_add = NULL;
    big_fn big_sub = NULL;
    char* error;

    // 1) dlopen
    handle = dlopen("./libbig.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    // 2) dlsym: big_add
    *(void**)(&big_add) = dlsym(handle, "big_add");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "%s\n", error);
        dlclose(handle);
        exit(1);
    }

    // 3) dlsym: big_sub
    *(void**)(&big_sub) = dlsym(handle, "big_sub");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "%s\n", error);
        dlclose(handle);
        exit(1);
    }

    // 4) call functions
    char* r1 = big_add("999999999999999999999999", "1");
    char* r2 = big_sub("5", "7");


    printf("add = %s\n", r1);
    printf("sub = %s\n", r2);

    free(r1);
    free(r2);

    // 5) dlclose
    if (dlclose(handle) < 0) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    return 0;
}
