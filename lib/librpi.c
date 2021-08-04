#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <zlib.h>

#include "librpi.h"

// Local prototypes
static char * _format_enum_to_string(RPI_format_e fmt);
static int _format_enum_size(RPI_format_e fmt);
static void _print_header(RPI_header_t * header);


// Public functions
RPI_image_t * rpi_load_file(char * filename)
{
    RPI_header_t * header_ptr;
    RPI_image_t * image_ptr;
    int fd,x,bytes;
    int file_size;
    int payload_size;
    uint32_t checksum;

    if(!filename) return NULL;

    fd = open(filename, O_RDONLY);
    if(!fd){
        printf("Error opening file %s\n",filename);
        return NULL;
    }

    // Get file size
    {
        struct stat buf;
        fstat(fd, &buf);
        file_size = buf.st_size;
    }

    // Allocate enough space for the header
    header_ptr = (RPI_header_t*)malloc(sizeof(RPI_header_t));
    if(!header_ptr){
        printf("MALLOC error!\n");
        close(fd);
        return NULL;
    }

    // Read in the header
    x = read(fd,header_ptr,sizeof(RPI_header_t));
    if(x!=sizeof(RPI_header_t)){
        printf("Error in read %d != %lu\n",x,sizeof(RPI_header_t));
        close(fd);
        free(header_ptr);
        return NULL;
    }

    // Make sure the signature is valid
    if(header_ptr->signature!=RPI_SIGNATURE_WORD){
        printf("Signature mismatch, file is not an RPI file\n");
        close(fd);
        free(header_ptr);
        return NULL;
    }

    // Make sure the size makes sense
    payload_size = rpi_get_payload_size(header_ptr);
    if(file_size<payload_size+sizeof(RPI_header_t)){
        printf("Error file is too small, %d < %lu\n",file_size,payload_size+sizeof(RPI_header_t));
        close(fd);
        free(header_ptr);
        return NULL;
    }

    // Everything checks out, we can resize the buffer, and load the data

    // Resize the buffer
    image_ptr = (RPI_image_t*)realloc(header_ptr, payload_size+sizeof(RPI_header_t));
    if(!image_ptr){
        printf("Error in realloc!\n");
        free(header_ptr);
        close(fd);
        return NULL;
    }
    header_ptr=NULL;


    // Load in the data
    x=0;
    bytes=0;
    while(x<payload_size){
        int chunk = 512;
        if(payload_size-x<chunk) chunk=payload_size-x;
        if(read(fd,&(image_ptr->payload[x]),chunk)!=chunk)
        {
            printf("Error in read\n");
            free(image_ptr);
            close(fd);
            return NULL;
        }
        bytes+=chunk;
        x+=chunk;
    }
    //printf("Read done, read %d bytes\n",bytes);
    // Close the file
    close(fd);

    // Verify the checksum
    checksum = rpi_calculate_checksum(image_ptr);
    if(checksum!=image_ptr->header.checksum){
        printf("Checksum error 0x%08X != 0x%08X\n",checksum,image_ptr->header.checksum);
        free(image_ptr);
        return NULL;
    }

    // ALL GOOD!
    //_print_header((RPI_header_t*)image_ptr);
    //printf("sizeof RPI_header_t: %ld\n",sizeof(RPI_header_t));
    //printf("sizeof RPI_image_t: %ld\n",sizeof(RPI_image_t));

    return image_ptr;
}

bool rpi_save_file(char * filename, RPI_image_t * rpi)
{
    int fd;
    int payload_size;
    int index;
    uint8_t * ptr;
    uint32_t crc;

    if(!rpi || !filename) return false;

    payload_size = rpi_get_payload_size(&(rpi->header));

    printf("Saving image, %d bytes > %s\n",payload_size,filename);

    fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
    if(fd<0){
        printf("Error making file %s\n",filename);
        return false;
    }

    // Constants
    rpi->header.signature = RPI_SIGNATURE_WORD;
    rpi->header.revision = 0;

    // Get CRC
    crc = rpi_calculate_checksum(rpi);
    rpi->header.checksum = crc;

    // Write the header
    write(fd,rpi,sizeof(RPI_header_t));

    // Write the data
    index=0;
    while(index<payload_size){
        int chunk = 100;
        if(payload_size-index<chunk) chunk=payload_size-index;
        write(fd,&(rpi->payload[index]),chunk);
        index+=chunk;
    }

    close(fd);

    return true;
}

RPI_image_t * rpi_malloc(int width, int height, RPI_format_e format)
{
    RPI_image_t * image_ptr;
    int payload_size;

    if(width<=0 || height<=0) return NULL;

    payload_size = width * height * _format_enum_size(format);

    // Create a new image in RAM
    image_ptr = (RPI_image_t*)malloc(payload_size+sizeof(RPI_header_t));
    if(!image_ptr){
        printf("Error in alloc!\n");
        return NULL;
    }

    // Fill it out
    image_ptr->header.width = width;
    image_ptr->header.height = height;
    image_ptr->header.format_enum = format;

    return image_ptr;
}

void rpi_free(RPI_image_t * rpi)
{
    if(rpi) free(rpi);
    return;
}

void rpi_set_comment(RPI_image_t * rpi, char * comment)
{
    if(!rpi || !comment) return;
    strncpy((char*)rpi->header.comment,comment,sizeof(rpi->header.comment));
    return;
}

RGBA_t rpi_pixel_to_RGBA(RPI_image_t * image, int pixel)
{
    RGBA_t rgba;
    memset(&rgba,0,sizeof(RGBA_t));

    // Bounds checks
    if(!image || pixel<0) return rgba;
    if(pixel > image->header.width*image->header.height) return rgba;

    switch(image->header.format_enum){
    case RPI_FORMAT_RGB565:
    {
        uint16_t * ptr;
        uint16_t tmp16;
        ptr = (uint16_t*)image->payload;
        tmp16 = ptr[pixel];
        rgba.R = ((tmp16>>11)&0x1F)<<3;
        rgba.G = ((tmp16>>5)&0x3F)<<2;
        rgba.B = (tmp16&0x1F)<<3;
    }
    break;
    case RPI_FORMAT_BGR565:
    {
        uint16_t * ptr;
        uint16_t tmp16;
        ptr = (uint16_t*)image->payload;
        tmp16 = ptr[pixel];
        rgba.B = ((tmp16>>11)&0x1F)<<3;
        rgba.G = ((tmp16>>5)&0x3F)<<2;
        rgba.R = (tmp16&0x1F)<<3;
    }
    break;
    default: break;
    }


    return rgba;
}

bool rpi_set_pixel_RGBA(RPI_image_t * image, int pixel, RGBA_t rgba)
{
    // Bounds checks
    if(!image || pixel<0) return false;
    if(pixel > image->header.width*image->header.height) return false;

    switch(image->header.format_enum){
    case RPI_FORMAT_RGB565:
    {
        uint16_t * ptr;
        uint16_t tmp16;
        ptr = (uint16_t*)image->payload;

        tmp16 = (rgba.R>>3);
        tmp16 <<= 6;
        tmp16 |= (rgba.G>>2);
        tmp16 <<= 5;
        tmp16 |= (rgba.B>>3);

        ptr[pixel] = tmp16;
    }
    break;
    case RPI_FORMAT_BGR565:
    {
        uint16_t * ptr;
        uint16_t tmp16;
        ptr = (uint16_t*)image->payload;

        tmp16 = (rgba.B>>3);
        tmp16 <<= 6;
        tmp16 |= (rgba.G>>2);
        tmp16 <<= 5;
        tmp16 |= (rgba.R>>3);

        ptr[pixel] = tmp16;
    }
    break;
    default: break;
    }

    return true;
}

int rpi_get_payload_size(RPI_header_t * header)
{
    int s;
    if(!header) return 0;
    s = header->width*header->height*_format_enum_size(header->format_enum);
    return s;
}

int rpi_get_pixel_count(RPI_header_t * header)
{
    int x;
    int b;
    if(!header) return 0;
    x = rpi_get_payload_size(header);
    b = _format_enum_size(header->format_enum);
    if(!b) return 0;
    x = x/b;
    return x;
}

uint32_t rpi_calculate_checksum(RPI_image_t * image)
{
    uint32_t crc;
    int payload_size;

    if(!image) return 0;

    payload_size = rpi_get_payload_size(&(image->header));

    crc = crc32(RPI_CHECKSUM_POLYNOMIAL, image->payload, payload_size);

    return crc;
}

void rpi_print(RPI_image_t * rpi)
{
    if(!rpi) return;
    printf("RPI image data:\n");
    _print_header(&(rpi->header));
}

// Private functions

static char * _format_enum_to_string(RPI_format_e fmt)
{
    switch(fmt){
    case RPI_FORMAT_RGB565: return "RPI_FORMAT_RGB565";
    case RPI_FORMAT_BGR565: return "RPI_FORMAT_BGR565";
    case RPI_FORMAT_YUYV: return "RPI_FORMAT_YUYV";
    case RPI_FORMAT_UYUV: return "RPI_FORMAT_UYUV";
    case RPI_FORMAT_RGBA5551: return "RPI_FORMAT_RGBA5551";
    case RPI_FORMAT_RGAB5515: return "RPI_FORMAT_RGAB5515";
    }
    return "FORMAT_UNKNOWN";
}
static int _format_enum_size(RPI_format_e fmt)
{
    switch(fmt){
    case RPI_FORMAT_RGB565: return RPI_FORMAT_RGB565_BYTES;
    case RPI_FORMAT_BGR565: return RPI_FORMAT_BGR565_BYTES;
    case RPI_FORMAT_YUYV: return RPI_FORMAT_YUYV_BYTES;
    case RPI_FORMAT_UYUV: return RPI_FORMAT_UYUV_BYTES;
    case RPI_FORMAT_RGBA5551: return RPI_FORMAT_RGBA5551_BYTES;
    case RPI_FORMAT_RGAB5515: return RPI_FORMAT_RGAB5515_BYTES;
    }
    return 0;
}
static void _print_header(RPI_header_t * header)
{
    int payload_size;
    if(!header) return;
    printf(" Signature  : 0x%08X\n",header->signature);
    printf(" Width      : %d px\n",header->width);
    printf(" Height     : %d px\n",header->height);
    printf(" Format Enum: 0x%02X (%s)\n",header->format_enum,_format_enum_to_string((RPI_format_e)header->format_enum));
    printf(" revision   : %d\n",header->revision);
    printf(" Flags      : 0x%08X\n",header->flags);
    if(header->flags&RPI_FLAG_CHECKSUM_ALL_DATA) printf("  - RPI_FLAG_CHECKSUM_ALL_DATA\n");
    if(header->flags&RPI_FLAG_INVERT_PIXEL_DATA) printf("  - RPI_FLAG_INVERT_PIXEL_DATA\n");
    printf(" Checksum   : 0x%08X\n",header->checksum);
    printf(" Comment    : %s\n",header->comment);

    payload_size = rpi_get_payload_size(header);

    printf(" Payload size: %d\n", payload_size);
    return;
}


// EOF
