#include "peachos.h"
#include "stdlib.h"

int main(int argc, char** argv) {
    peachos_print("Hello from blank program!\n");

    // Allocate some memory
    size_t alloc_size = 64;
    void* mem = malloc(alloc_size);
    if (mem) {
        peachos_print("Memory allocated successfully.\n");
    } else {
        peachos_print("Memory allocation failed.\n");
    }

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