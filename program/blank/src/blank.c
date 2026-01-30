#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char** argv) {
    peachos_print("Hello from blank program!\n");

    // Allocate some memory
    size_t alloc_size = 64;
    void* mem = malloc(alloc_size);
    if (mem) {
        peachos_print("Memory allocated successfully.\n");
        free(mem);
        peachos_print("Memory freed successfully.\n");
    } else {
        peachos_print("Memory allocation failed.\n");
    }

    int res = printf("Testing printf: Hello %s, char %c number %d\n", "World", 'A', 42);
    printf("printf returned: %d\n", res);

    while(1) {
        char c;
        if ((c = peachos_getchar()) != '\0') {
            peachos_print("You pressed: ");
            char buf[2] = {c, '\0'};
            peachos_print(buf);
            peachos_print("\n");
        }
    }
    return 0;
}