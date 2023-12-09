#ifndef __KERNELS_H__
#define __KERNELS_H__

typedef struct kernel_t{
    const char *data;
    int height;
    int width;
} kernel_t;

const char GAUSSIAN_BLUR_3x3[3][3] = {{1,2,1},
                                      {2,4,2},
                                      {1,2,1}};

const char GAUSSIAN_BLUR_5x5[5][5] = {{1,4,6,4,1},
                                      {4,16,24,16,4},
                                      {6,24,36,24,6},
                                      {4,16,24,16,4},
                                      {1,4,6,4,1}};

const char EDGE_DETECTION_3x3[3][3] = {{-1,-1,-1},
                                       {-1,8,-1},
                                       {-1,-1,-1}};

const char LAPLACIAN_5x5[5][5] = {{-1,-1,-1,-1,-1},
                                  {-1,-1,-1,-1,-1},
                                  {-1,-1,24,-1,-1},
                                  {-1,-1,-1,-1,-1},
                                  {-1,-1,-1,-1,-1}};


const char SHARPEN_3x3[3][3] = {{0,-4,0},
                                {-4,17,-4},
                                {0,-4,-0}};

#endif // __KERNELS_H__
