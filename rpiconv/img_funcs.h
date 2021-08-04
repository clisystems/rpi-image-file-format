#ifndef __JPEG_FUNCS_H__
#define __JPEG_FUNCS_H__

typedef struct{
    uint8_t * data;
    int size;
    int width;
    int height;
    int channels;
}jpeg_image_t;

typedef struct{
    uint8_t * data;
    int size;
    int width;
    int height;
    int channels;
}bmp_image_t;

void bmp_free(bmp_image_t * img);

bmp_image_t bmp_load(char * filename);

void jpeg_free(jpeg_image_t * img);

jpeg_image_t jpeg_load(char * filename);

jpeg_image_t jpeg_alloc(int width, int height, int channels);

int jpeg_save_file(char * filename, jpeg_image_t * img);
    

#endif
