#ifndef __BMP_H__
#define __BMP_H__

#define BMP_HEADER_SIZE 54

typedef struct bmp_t{
    char header[BMP_HEADER_SIZE];
    char *color_tab;
    unsigned char *pixels;
} bmp_t;

#endif // __BMP_H__