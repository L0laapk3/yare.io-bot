#include "interface.h"

#include <stdlib.h>


EXPORT("alloc")
char* alloc(int size) {
    return (char*)malloc(size * sizeof(char));
}