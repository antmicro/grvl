#ifndef __JPEGHELPER_H__
#define __JPEGHELPER_H__
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
void GetJPEGInfo(unsigned char* data, int32_t len, uint32_t* width, uint32_t* height);
void GetJPEGData(unsigned char* destination, unsigned char* data, int32_t len);
#ifdef __cplusplus
}
#endif

#endif
