#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int (*add_fn)(int, int);

int main(void) {
    void *handle = dlopen("./libadd.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }

    dlerror(); // clear existing errors

    add_fn addp = (add_fn)dlsym(handle, "add");
    const char *err = dlerror();
    if (err) {
        fprintf(stderr, "dlsym failed: %s\n", err);
        dlclose(handle);
        return 1;
    }

    printf("add(7, 8) = %d\n", addp(7, 8));

    if (dlclose(handle) != 0) {
        fprintf(stderr, "dlclose failed: %s\n", dlerror());
        return 1;
    }
    return 0;
}
