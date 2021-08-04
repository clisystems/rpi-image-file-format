#include "defs.h"

#include <string.h>
#include <stdlib.h>

#include "img_funcs.h"
#include "operations.h"


int do_jpeg2rpi(char * in_filename, char * out_filename, RPI_format_e format, char * comment)
{
    jpeg_image_t jpeg_img;
    RPI_image_t * rpi_img;
    
    if(strlen(in_filename)<1){
        printf("Error, Input filename is empty!\n");
        return -1;
    }
    if(strlen(out_filename)<1){
        printf("Error, Output filename is empty!\n");
        return -1;
    }
        
    printf("Converting to RPI: %s -> %s\n",in_filename,out_filename);

    // Load the JPEG
    jpeg_img = jpeg_load(in_filename);
    if(jpeg_img.size<0){
        printf("Error, loading JPEG image %s\n",in_filename);
        return -1;
    }

    if(g_verbose){
        printf("JPEG ptr: %p\n",jpeg_img.data);
        printf("JPEG size: %d\n",jpeg_img.size);
        printf("JPEG width: %d\n",jpeg_img.width);
        printf("JPEG height: %d\n",jpeg_img.height);
        printf("JPEG channels: %d\n",jpeg_img.channels);
    }
    
    // Make a new RPI based on the image
    rpi_img = rpi_malloc(jpeg_img.width, jpeg_img.height, format);
    if(!rpi_img){
        printf("Error with RPI allocate\n");
        jpeg_free(&jpeg_img);
        return -1;
    }

    // Translate JPEG to RPI
    {
        int r,c,i;
        uint16_t p;

        for(r=0;r<jpeg_img.height;r++)
        {
            for(c=0;c<jpeg_img.width;c++)
            {
                RGBA_t rgba;
                i = (r*jpeg_img.width)+c;

                rgba.R = jpeg_img.data[(i*3)];
                rgba.G = jpeg_img.data[(i*3)+1];
                rgba.B = jpeg_img.data[(i*3)+2];
                rgba.A=0;

                rpi_set_pixel_RGBA(rpi_img, i, rgba);

            }
        }
    }
    
    rpi_set_comment(rpi_img,comment);

    // Save file
    rpi_save_file(out_filename, rpi_img);
    
    // Free the allocated mem
    jpeg_free(&jpeg_img);

    rpi_free(rpi_img);

    return 0;
}

int do_rpi2jpeg(char * in_filename, char * out_filename)
{
    jpeg_image_t jpeg_img;
    RPI_image_t * rpi_img;
    uint8_t * data;
    
    if(strlen(in_filename)<1){
        printf("Error, Input filename is empty!\n");
        return -1;
    }
    if(strlen(out_filename)<1){
        printf("Error, Output filename is empty!\n");
        return -1;
    }
        
    printf("Converting to JPEG: %s -> %s\n",in_filename,out_filename);
    
    // Load the RPI
    rpi_img = rpi_load_file(in_filename);
    if(!rpi_img){
        printf("Error loading RPI image %s\n",in_filename);
        return -1;
    }

    // Allocate the JPEG
    jpeg_img = jpeg_alloc(rpi_img->header.width, rpi_img->header.height, 3);

    // Translate RPI to JPEG
    {
        int r,c,i;
        uint16_t p;

        for(r=0;r<rpi_img->header.height;r++)
        {
            for(c=0;c<rpi_img->header.width;c++)
            {
                RGBA_t rgba;
                i = (r*jpeg_img.width)+c;
                rgba = rpi_pixel_to_RGBA(rpi_img, i);

                jpeg_img.data[(i*3)] = rgba.R;
                jpeg_img.data[(i*3)+1] = rgba.G;
                jpeg_img.data[(i*3)+2] = rgba.B;

            }
        }
    }

    // Save file
    jpeg_save_file(out_filename, &jpeg_img);

    // Free the allocated mem
    jpeg_free(&jpeg_img);

    rpi_free(rpi_img);

    return 0;
}


int do_bmp2rpi(char * in_filename, char * out_filename, RPI_format_e format, char * comment)
{
    bmp_image_t bmp_img;
    RPI_image_t * rpi_img;

    if(strlen(in_filename)<1){
        printf("Error, Input filename is empty!\n");
        return -1;
    }
    if(strlen(out_filename)<1){
        printf("Error, Output filename is empty!\n");
        return -1;
    }

    printf("Converting to RPI: %s -> %s\n",in_filename,out_filename);

    // Load the BMP
    bmp_img = bmp_load(in_filename);
    if(bmp_img.size<0){
        printf("Error, loading BMP image %s\n",in_filename);
        return -1;
    }

    // Show BMP info
    if(g_verbose){
        printf("BMP ptr: %p\n",bmp_img.data);
        printf("BMP size: %d\n",bmp_img.size);
        printf("BMP width: %d\n",bmp_img.width);
        printf("BMP height: %d\n",bmp_img.height);
        printf("BMP channels: %d\n",bmp_img.channels);
    }

    // Make a new RPI based on the image
    rpi_img = rpi_malloc(bmp_img.width, bmp_img.height, format);
    if(!rpi_img){
        printf("Error with RPI allocate\n");
        bmp_free(&bmp_img);
        return -1;
    }

    // Translate BMP to RPI
    {
        int r,c,i;
        uint16_t p;

        for(r=0;r<bmp_img.height;r++)
        {
            for(c=0;c<bmp_img.width;c++)
            {
                RGBA_t rgba;
                i = (r*bmp_img.width)+c;

                rgba.R = bmp_img.data[(i*3)];
                rgba.G = bmp_img.data[(i*3)+1];
                rgba.B = bmp_img.data[(i*3)+2];
                rgba.A=0;

                rpi_set_pixel_RGBA(rpi_img, i, rgba);

            }
        }
    }

    rpi_set_comment(rpi_img,comment);

    // Save file
    rpi_save_file(out_filename, rpi_img);

    // Free the allocated mem
    bmp_free(&bmp_img);

    rpi_free(rpi_img);


    return 0;
}


int do_dump_jpeg(char * in_filename)
{
    jpeg_image_t imgstruct;
    

    if(strlen(in_filename)<1){
        printf("Error, Input filename is empty!\n");
        return -1;
    }
    
        
    printf("Loading image: %s\n",in_filename);
    imgstruct = jpeg_load(in_filename);
    
    printf("Data ptr: %p\n",imgstruct.data);
    printf("Data size: %d\n",imgstruct.size);
    printf("Data width: %d\n",imgstruct.width);
    printf("Data height: %d\n",imgstruct.height);
    printf("Data channels: %d\n",imgstruct.channels);
    
    jpeg_free(&imgstruct);

    return 0;
}
int do_dump_rpi(char * in_filename)
{
    RPI_image_t * rpi;

    if(!in_filename || strlen(in_filename)<=0){
        printf("Error, filename empty\n");
        return -1;
    }

    printf("Loading image: %s\n",in_filename);
    rpi = rpi_load_file(in_filename);
    if(!rpi){
        printf("Error loading RPI %s\n",in_filename);
        return -1;
    }

    rpi_print(rpi);

    rpi_free(rpi);

    return 0;
}
    

// EOF
