#ifndef __OPERATIONS_H__
#define __OPERATIONS_H__

#include "librpi.h"

int do_jpeg2rpi(char * in_filename, char * out_filename, RPI_format_e format, char * comment);

int do_rpi2jpeg(char * in_filename, char * out_filename);

int do_bmp2rpi(char * in_filename, char * out_filename, RPI_format_e format, char * comment);

int do_dump_jpeg(char * in_filename);

int do_dump_rpi(char * in_filename);


#endif
