#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "kernels.h"
#include "bmp.h"

#define fatal(message) do {fprintf(stderr, "%s:%d: %s", __FILE__, __LINE__, \
                           message); exit(EXIT_FAILURE);} while(0)

#define BUFSIZE 4096


void init_bmp(bmp_t *image){
    image->color_tab = NULL;
    image->pixels = NULL;
}

unsigned int 
get_width(bmp_t *im){
    unsigned int *b = (unsigned int*)(im->header + 18);
    return *b;
}

unsigned int 
get_height(bmp_t *im){
    unsigned int *b = (unsigned int*)(im->header + 22);
    return *b;
}

unsigned int 
get_bit_depth(bmp_t *im){
    unsigned int *b = (unsigned int*)(im->header + 28);
    return *b;
}

bool 
bmp_has_color_tab(bmp_t *im){
    unsigned int bd = get_bit_depth(im);
    return bd <= 8 ? true : false;
}

unsigned long long 
get_color_tab_size(bmp_t *im){
    unsigned int bit_depth = get_bit_depth(im);
    return (1 << bit_depth) * 4;
}


/* Load bmp image. 
*/
void 
load_bmp(char *path, bmp_t *image)
{
    FILE *f_in = NULL;
    unsigned char *pixel_data = NULL; 
    unsigned long long size = 0; // size of pixel data in bytes: width * height * bit_depth
    unsigned long long n = 0;
    unsigned int bit_depth = 0, width = 0, height = 0;
    char buf[BUFSIZE] = {0};

    f_in = fopen(path, "r");
    if(!f_in)
        fatal("Unable to open file for reading");
    
    if(!image)
        image = (bmp_t*) calloc(1, sizeof(bmp_t));

    if(!image)
        fatal("Cannot allocate memory for bmp\n");

    // header
    fread(image->header, 1, BMP_HEADER_SIZE, f_in);
    bit_depth = get_bit_depth(image);
    width = get_width(image);
    height = get_height(image);

    printf("bit_depth=%d\nwidth=%d\nheight=%d\n", bit_depth, width, height);
    // copy color table if it's present
    if(bmp_has_color_tab(image)){
        n = get_color_tab_size(image);
        printf("colortab size is %d\n", n);
        image->color_tab = (char*)calloc(1, n);
        fread(buf, 1, n, f_in);
        memcpy(image->color_tab, buf, n);       
    }

    // allocate memory for pixels
    size = bit_depth / 8 * width * height;
    pixel_data = image->pixels = (unsigned char*) calloc(size, 1);
    if(!pixel_data)
        fatal("Unable to allocate memory");

    // copy pixel data to allocated memory
    n = fread(buf, 1, BUFSIZE, f_in);
    while(n > 0){
        memcpy(pixel_data, buf, n);
        pixel_data += n;
        n = fread(buf, 1, BUFSIZE, f_in);
    }

    fclose(f_in);
}


void 
store_bmp(char *path, bmp_t *image)
{
    FILE *f_out = NULL;
    unsigned int bit_depth = 0, width = 0, height = 0;

    if(!image)
        fatal("Cannot store NULL image!\n");
   
    bit_depth = get_bit_depth(image);
    width = get_width(image);
    height = get_height(image);

    // copy header
    f_out = fopen(path, "w");
    if(!f_out)
        fatal("Unable to open file for writing");
    fwrite(image->header, 1, BMP_HEADER_SIZE, f_out);
    
    // copy color table if it's present
    if(image->color_tab)
        fwrite(image->color_tab, 1, 4 * (1 << bit_depth), f_out);

    // copy pixel data
    fwrite(image->pixels, 1, (unsigned long long)bit_depth / 8 * width * height, f_out);

    fclose(f_out);
}

void 
free_bmp(bmp_t im){
    if(im.color_tab)
        free(im.color_tab);
    if(im.pixels)
        free(im.pixels);
}

/* Note: If width in pixels of input image is not divisible by 4, 
   zero-padding should be performed.
   Two pictures will visually look the same although not being 
   binary identical. */
void 
copy_bmp(char *dest_path, char *src_path)
{
    bmp_t image;
    init_bmp(&image);
    load_bmp(src_path, &image);
    store_bmp(dest_path, &image);
    free_bmp(image);
}

/*
    Perform 2D Discrete Convolution on image_in using 
    kernel mask and produce image_out.
*/
void 
convolve(bmp_t *image_out, bmp_t *image_in, kernel_t *kernel)
{
    int m = 0, n = 0, val = 0, 
            idx = 0, jdx = 0, color_tab_size = 0;

    unsigned int i, j;
    unsigned int img_hei = get_height(image_in);
    unsigned int img_wid = get_width(image_in);
 
    image_out->pixels = (unsigned char*)calloc(img_hei*img_wid, 1);
    if(!image_out->pixels)
        fatal("Couldn't allocate memory");


    memcpy(image_out->header, image_in->header, BMP_HEADER_SIZE);
    if(bmp_has_color_tab(image_in)){
        color_tab_size = get_color_tab_size(image_in);
        image_out->color_tab = (char*)calloc(1, color_tab_size);
        memcpy(image_out->color_tab, image_in->color_tab, color_tab_size);
    }

    for(i = 0; i < img_hei; i++){
        for(j = 0; j < img_wid; j++){
            val = 0;
            for(m = 0; m < kernel->height; m++){
                    idx = i - m;
                for(n = 0; n < kernel->width; n++){
                    jdx = j - n;
                    if(idx >= 0 && jdx >= 0)
                        val += image_in->pixels[idx*img_wid + jdx] * kernel->data[m*kernel->width + n];
                }
            }
            if(val > 255)
                val = 255;
            else if(val < 0)
                val = 0;
           //image_out->pixels[i*img_wid + j] = image_in->pixels[i*img_wid + j];
           image_out->pixels[i*img_wid + j] = (unsigned char)val;
        }
    }
}

void 
init_kernel(kernel_t *k)
{
    k->data = NULL;
    k->width = k->height = 0;
}

int 
main(int argc, char **argv)
{
    if(argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))){
        printf("%s input-bmp-image output kernel\n\n", argv[0]);
        printf("Supported kernels:\n");
        printf("  laplacian_5x5\n  gaussian_blur_3x3\n  gaussian_blur_5x5\n"
            "  edge_detection_3x3\n  sharpen_3x3\n");
        return 0;
    }
    else if(argc != 4)
        fatal("Usage: ./exe <input-image> <output> <kernel>\n");
    
    
    char *bmp_name_in = argv[1], *bmp_name_out = argv[2];
    kernel_t kernel;
    //init_kernel(&kernel);

    if(!strcmp(argv[3], "gaussian_blur_3x3")){
        kernel.width = kernel.height = 3;
        kernel.data = (const char*)GAUSSIAN_BLUR_3x3;
    }
    else if(!strcmp(argv[3], "gaussian_blur_5x5")){
        kernel.width = kernel.height = 5;
        kernel.data = (const char*)GAUSSIAN_BLUR_5x5;
    }
    else if(!strcmp(argv[3], "laplacian_5x5")){
        kernel.width = kernel.height = 5;
        kernel.data = (const char*)LAPLACIAN_5x5;
    }
    else if(!strcmp(argv[3], "edge_detection_3x3")){
        kernel.width = kernel.height = 3;
        kernel.data = (const char*)EDGE_DETECTION_3x3;
    }
    else if(!strcmp(argv[3], "sharpen_3x3")){
        kernel.width = kernel.height = 3;
        kernel.data = (const char*)SHARPEN_3x3;
    }
    else{
        fatal("Invalid kernel\n");
    }

    bmp_t image_in, image_out;
    init_bmp(&image_in);
    init_bmp(&image_out);
    load_bmp(bmp_name_in, &image_in);
    convolve(&image_out, &image_in, &kernel);
    store_bmp(bmp_name_out, &image_out);
    free_bmp(image_out);
    free_bmp(image_in);

    //copy_bmp(bmp_name_out, bmp_name_in);
    
    return 0;
}
