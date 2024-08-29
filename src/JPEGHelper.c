#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <jpeglib.h>

void GetJPEGInfo(unsigned char* data, int32_t len, uint32_t* width, uint32_t* height)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr error;
    cinfo.err = jpeg_std_error(&error);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, len);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    *width = (uint32_t)cinfo.output_width;
    *height = (uint32_t)cinfo.output_height;
    cinfo.output_scanline = cinfo.output_width;
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}

void GetJPEGData(unsigned char* destination, unsigned char* data, int32_t len)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr error;
    cinfo.err = jpeg_std_error(&error);

    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, len);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    while(cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, &destination, 1);
        destination += cinfo.output_width * 3;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}
