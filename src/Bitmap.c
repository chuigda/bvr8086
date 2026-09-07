#include "Bitmap.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {

} BitmapHeader;

BvrBitmap *BvrLoadBitmap(char const *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }


}
