#include "peachos.h"

int main(int argc, char** argv) {
    print("Hello from blank program!\n");
    while(1) {
        char c;
        if ((c = getchar()) != '\0') {
            print("You pressed: ");
            char buf[2] = {c, '\0'};
            print(buf);
            print("\n");
        }
    }
    return 0;
}