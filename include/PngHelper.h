#ifndef GRVL_PNGHELPER_H_
#define GRVL_PNGHELPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    unsigned char* pointer;
    int len;
    int position;
} memory_pointer_t;

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel;
    unsigned int row_bytes;
    unsigned int color_type;
    uintptr_t plte;
    unsigned int plte_length;
} png_info_t;

void GetPngInfo(png_info_t* info, unsigned char* data, int len, bool plteSupport);
void GetPngData(unsigned char* destination, unsigned char* data, int len, int alpha, bool plteSupport);

#ifdef __cplusplus
}

#endif

#endif /* GRVL_PNGHELPER_H_ */
