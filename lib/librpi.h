#ifndef __LIBRPI_H__
#define __LIBRPI_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint32_t signature;
    uint16_t width;
    uint16_t height;
    uint8_t format_enum;
    uint8_t revision;
    uint16_t flags;
    uint32_t checksum;
    int8_t comment[16];
}RPI_header_t;

typedef struct{
    RPI_header_t header;
    uint8_t payload[];
}RPI_image_t;

typedef struct{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A;
}RGBA_t;

#define RPI_SIGNATURE_WORD              0x52504931

#define RPI_CHECKSUM_POLYNOMIAL         0x04C11DB7

typedef enum{
    RPI_FORMAT_RGB565   = 0x00,
    RPI_FORMAT_BGR565   = 0x01,
    RPI_FORMAT_YUYV     = 0x02,
    RPI_FORMAT_UYUV     = 0x03,
    RPI_FORMAT_RGBA5551 = 0x04,
    RPI_FORMAT_RGAB5515 = 0x05,
}RPI_format_e;

#define RPI_FORMAT_RGB565_BYTES         sizeof(uint16_t)
#define RPI_FORMAT_BGR565_BYTES         sizeof(uint16_t)
#define RPI_FORMAT_YUYV_BYTES           sizeof(uint32_t)
#define RPI_FORMAT_UYUV_BYTES           sizeof(uint32_t)
#define RPI_FORMAT_RGBA5551_BYTES       sizeof(uint16_t)
#define RPI_FORMAT_RGAB5515_BYTES       sizeof(uint16_t)


#define RPI_FLAG_CHECKSUM_ALL_DATA      0x0001
#define RPI_FLAG_INVERT_PIXEL_DATA      0x0002

#ifdef __cplusplus
extern "C" {
#endif

RPI_image_t * rpi_load_file(char * filename);

bool rpi_save_file(char * filename, RPI_image_t * rpi);

RPI_image_t * rpi_malloc(int width, int height, RPI_format_e format);

void rpi_free(RPI_image_t * rpi);

void rpi_set_comment(RPI_image_t * rpi, char * comment);

RGBA_t rpi_pixel_to_RGBA(RPI_image_t * image, int pixel);

bool rpi_set_pixel_RGBA(RPI_image_t * image, int pixel, RGBA_t rgba);

int rpi_get_payload_size(RPI_header_t * header);

int rpi_get_pixel_count(RPI_header_t * header);

uint32_t rpi_calculate_checksum(RPI_image_t * image);

void rpi_print(RPI_image_t * rpi);

#ifdef __cplusplus
}
#endif


#endif
