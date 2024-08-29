#define PNG_DEBUG 3
#include <png.h>
#include <pngstruct.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "PngHelper.h"

static void PrintAndAbort(const char* s, ...)
{
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    // abort();
}

void UserReadData(png_structp png_ptr, png_bytep data, png_size_t length)
{
    memory_pointer_t* mp = (memory_pointer_t*)png_get_io_ptr(png_ptr);
    if((mp->position + length) >= mp->len) {
        // sanity check
        return;
    }
    memcpy((char*)data, (char*)((uintptr_t)mp->pointer + (uintptr_t)mp->position), length);
    mp->position += length;
}

#define MAX_PNG_PALETTE_SIZE 256
static uint32_t tempPalette[MAX_PNG_PALETTE_SIZE * 3]; // Palette size * RGB

void PNG_warning_function(png_structp png_ptr, png_const_charp error)
{
    // ignore
}

void PNG_error_function(png_structp png_ptr, png_const_charp warning)
{
    PrintAndAbort("[read_png_file] did not go well :(");
    /*    jmp_buf jmpbuf;
        memcpy(jmpbuf, png_ptr->jmpbuf, sizeof(jmp_buf));
        longjmp(jmpbuf, 1);*/
}

void GetPngInfo(png_info_t* info, unsigned char* data, int len, bool plteSupport)
{
    memory_pointer_t mp;
    png_structp png_ptr;
    png_infop info_ptr;
    mp.pointer = data;
    mp.len = len;
    mp.position = 0;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        PrintAndAbort("[read_png_file] png_create_read_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        PrintAndAbort("[read_png_file] png_create_info_struct failed");
    }
    png_set_error_fn(png_ptr, 0, PNG_error_function, PNG_warning_function);
    png_set_read_fn(png_ptr, &mp, UserReadData);

    png_read_info(png_ptr, info_ptr);
    info->width = png_get_image_width(png_ptr, info_ptr);
    info->height = png_get_image_height(png_ptr, info_ptr);
    info->row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    info->bytes_per_pixel = info->row_bytes / info->width;
    info->color_type = png_get_color_type(png_ptr, info_ptr);
    if(info->color_type == 3 && plteSupport) { // With palette
        png_colorp palette;
        int num_palette;

        png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

        // Copy palette to a safe place.
        memcpy(tempPalette, palette, num_palette * 3); // RGB

        info->plte = (uintptr_t)tempPalette;
        info->plte_length = num_palette;
        png_set_palette_to_rgb(png_ptr);
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
        png_read_update_info(png_ptr, info_ptr);
    } else {
        info->plte = 0;
        info->plte_length = 0;
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return;
}

void GetPngData(unsigned char* destination, unsigned char* data, int len, int alpha, bool plteSupport)
{
    memory_pointer_t mp;
    png_structp png_ptr;
    png_infop info_ptr;
    int height, y;
    png_bytep* row_pointers;

    mp.pointer = data;
    mp.len = len;
    mp.position = 0;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_set_error_fn(png_ptr, 0, PNG_error_function, PNG_warning_function);

    if(!png_ptr) {
        PrintAndAbort("[read_png_file] png_create_read_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        PrintAndAbort("[read_png_file] png_create_info_struct failed");
    }

    png_set_read_fn(png_ptr, &mp, UserReadData);

    png_read_info(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);

    png_set_bgr(png_ptr);

    if(!alpha) {
        png_set_strip_alpha(png_ptr);
    }

    if(png_get_color_type(png_ptr, info_ptr) == 3 && !plteSupport) { // With palette
        png_set_palette_to_rgb(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
    }

    png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    int row_bytes = png_get_rowbytes(png_ptr, info_ptr);

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)destination + (y * row_bytes);
    }
    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    free(row_pointers);
}
