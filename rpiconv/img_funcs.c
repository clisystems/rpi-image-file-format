#include "defs.h"

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


#include "img_funcs.h"

void bmp_free(bmp_image_t * img)
{
    if(!img) return;
    if(img->data)
    {
        free(img->data);
        img->data=NULL;
    }
    return;
}

typedef struct{
    uint32_t length;
    uint32_t reserved;
    uint32_t offset;
}bmp_header1_t;
typedef struct{
    uint32_t header_size;
    uint32_t width;
    uint32_t height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    uint32_t rez_h;
    uint32_t rez_w;
    uint32_t num_colors;
    uint32_t num_colors_important;
}bmp_header2_t;
bmp_image_t bmp_load(char * filename)
{
    bmp_image_t bmp;
    uint8_t header[14];
    int fd;
    int x;
    bmp_header1_t header1;
    bmp_header2_t header2;
    uint8_t * data;
    uint8_t * row;
    int row_size;

    // Defaults
    memset(&bmp,0,sizeof(bmp_image_t));
    bmp.size=-1;

    // Bounds check
    if(!filename) return bmp;

    // Open file
    fd = open(filename, O_RDONLY);
    if(!fd){
        DEBUG("Error loading file\n");
        return bmp;
    }

    // Read in header
    x = read(fd,header,sizeof(header));
    if(x!=sizeof(header)){
        printf("Error header read\n");
        goto load_error_exit;
    }

    // Check header
    if(header[0]!=0x42 || header[1]!=0x4D)
    {
        printf("Invalid BMP header\n");
        goto load_error_exit;
    }

    // Check offset to see what version BMP we have
    memcpy((uint8_t*)&header1,&(header[2]),sizeof(header1));
    DEBUG("header1 size %d bytes\n",header1.length);
    DEBUG("header1 offset %d\n",header1.offset);
    if(header1.offset!=54){
        printf("BMP format not supported, offset %d!=54\n",header1.offset);
        goto load_error_exit;
    }

    // Offset says we are good, this is a BITMAPINFOHEADER BMP

    // Read in the header2 data
    x = read(fd,(uint8_t*)&header2,sizeof(header2));
    if(x!=sizeof(header2)){
        printf("Error header2 read\n");
        goto load_error_exit;
    }

    // Dump info
    if(g_verbose){
    printf("header2.header_size: %d\n",header2.header_size);
    printf("header2.width: %d\n",header2.width);
    printf("header2.height: %d\n",header2.height);
    printf("header2.color_planes: %d\n",header2.color_planes);
    printf("header2.bits_per_pixel: %d\n",header2.bits_per_pixel);
    printf("header2.compression: %d\n",header2.compression);
    printf("header2.image_size: %d\n",header2.image_size);
    printf("header2.rez_h: %d\n",header2.rez_h);
    printf("header2.rez_w: %d\n",header2.rez_w);
    printf("header2.num_colors: %d\n",header2.num_colors);
    printf("header2.num_colors_important: %d\n",header2.num_colors_important);
    }

    // get in to our format
    bmp.width = header2.width;
    bmp.height = header2.height;
    bmp.channels = (header2.bits_per_pixel)/8;
    bmp.size = (bmp.width*bmp.height)*bmp.channels;


    // Malloc RAM to hold the pixels
    data = (uint8_t*)malloc(bmp.size);
    if(!data){
        printf("Malloc %d bytes failed\n",bmp.size);
        bmp.size=-1;
        goto load_error_exit;
    }
    bmp.data = data;

    // Malloc a row of data to read from the file

    // BMP data is padded out to align on 4byte boundaries, so
    // the start of a row is r*width*pixelsize but add 3 and
    // then mask off the lower 2 bits
    row_size = (((bmp.width*bmp.channels)+3)&0xFFFFFFFC);
    row = (uint8_t*)malloc(row_size);
    if(!row){
        printf("Error malloc row size %d bytes\n",row_size);
        free(data);
        bmp.data=0;
        bmp.size=-1;
        goto load_error_exit;
    }
    DEBUG("Allocated row_size=%d\n",row_size);

    // seek to the start of the data (offset 54)
    lseek(fd,54,SEEK_SET);

    // Read in a the image data row by row, and translate it
    // in to our RAM struct.

    for(int r=0;r<bmp.height;r++)
    {
        //printf("Loading row %d/%d\n",r,bmp.height);

        // Load a row from the file
        x = read(fd,row,row_size);
        if(x!=row_size){
            printf("Error reading row %d\n",r);
            goto load_data_error_exit;
        }

        // Loop over the pixels and copy in to our data
        for(int c=0;c<bmp.width;c++)
        {
            int index;
            int rindex;

            rindex = c*bmp.channels;

            // This is the normal way to write left->right in the array
            //index = ((r*bmp.width)+c) * bmp.channels;

            // Because the BMP 0,0 is at the bottom left rather than the top
            // left, we need to subtract the height from the row.
            index = (((bmp.height-r)*bmp.width)+(c)) * bmp.channels;

            // Looks like data is in BGR, but docs say GBR...
            // This works for tested BMPs
            bmp.data[index] = row[rindex+2];
            bmp.data[index+1] = row[rindex+1];
            bmp.data[index+2] = row[rindex];
        }
    }


load_data_error_exit:

    free(row);

load_error_exit:
    close(fd);
    return bmp;
}

void jpeg_free(jpeg_image_t * img)
{
    if(!img) return;
    if(img->data)
    {
        free(img->data);
        img->data=NULL;
    }
    return;
}
    

// Original: https://stackoverflow.com/questions/5616216/need-help-in-reading-jpeg-file-using-libjpeg
#include <jpeglib.h>    
#include <jerror.h>
#include <setjmp.h>
struct jpegErrorManager {
    /* "public" fields */
    struct jpeg_error_mgr pub;
    /* for return to caller */
    jmp_buf setjmp_buffer;
};
char jpegLastErrorMsg[JMSG_LENGTH_MAX];
void jpegErrorExit (j_common_ptr cinfo)
{
    /* cinfo->err actually points to a jpegErrorManager struct */
    struct jpegErrorManager* myerr = (struct jpegErrorManager*) cinfo->err;
    /* note : *(cinfo->err) is now equivalent to myerr->pub */

    /* output_message is a method to print an error message */
    /*(* (cinfo->err->output_message) ) (cinfo);*/

    /* Create the message */
    ( *(cinfo->err->format_message) ) (cinfo, jpegLastErrorMsg);

    /* Jump to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);

}


jpeg_image_t jpeg_load(char * filename)
{
    unsigned long width, height;
    unsigned int texture_id;
    unsigned long data_size;     // length of the file
    int channels;               //  3 =>RGB   4 =>RGBA 
    unsigned int type;  
    uint8_t * rowptr[1];    // pointer to an array
    unsigned char * jdata;        // data for the image
    struct jpeg_decompress_struct info; //for our jpeg info
    FILE * file=NULL;
    jpeg_image_t imgstruct;

    struct jpegErrorManager jerr;
    info.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpegErrorExit;

    memset(&imgstruct,0,sizeof(jpeg_image_t));

    //open the file
    file = fopen(filename, "rb");
    if(!file) {
        printf("Error reading JPEG file %s!", filename);
        return imgstruct;
    }


    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        printf("Error in JPEG decompress\n");
        jpeg_destroy_decompress(&info);
        if(file) fclose(file);
        memset(&imgstruct,0,sizeof(jpeg_image_t));
        imgstruct.size=-1;
        return imgstruct;
    }
    
    jpeg_create_decompress(& info);   //fills info structure
    jpeg_stdio_src(&info, file);
    HERE();
    jpeg_read_header(&info, TRUE);   // read jpeg file header
    HERE();
    // decompress the file
    jpeg_start_decompress(&info);
    HERE();
    #if 1
    DEBUG("info image_width: %d\n",info.image_width);
    DEBUG("info image_hiehgt: %d\n",info.image_height);
    DEBUG("info num_components: %d\n",info.num_components);
    DEBUG("info jpeg_color_space: %d\n",info.jpeg_color_space);
    DEBUG("info out_color_space: %d\n",info.out_color_space);
    DEBUG("info output_width: %d\n",info.output_width);
    DEBUG("info output_hiehgt: %d\n",info.output_height);
    DEBUG("info output_scanline: %d\n",info.output_scanline);
    DEBUG("info raw_data_out: %d\n",info.raw_data_out);
    #endif
    
    //set width and height
    width = info.output_width;
    height = info.output_height;
    channels = info.num_components;

    data_size = width * height * channels;

    // read scanlines one at a time & put bytes 
    //    in jdata[] array. Assumes an RGB image
    jdata = (unsigned char *)malloc(data_size);
    
    while (info.output_scanline < info.output_height) // loop
    {
        // Enable jpeg_read_scanlines() to fill our jdata array
        rowptr[0] = (unsigned char *)jdata +  3* info.output_width * info.output_scanline; 

        jpeg_read_scanlines(&info, rowptr, 1);
    }

    //finish decompressing  
    jpeg_finish_decompress(&info);

    jpeg_destroy_decompress(&info);
    
    // close the file
    fclose(file);
    file=NULL;

    // Load up the data structure and return it
    imgstruct.data = jdata;
    imgstruct.width =width;
    imgstruct.height = height;
    imgstruct.channels =channels;
    imgstruct.size = data_size;

    return imgstruct;
}

jpeg_image_t jpeg_alloc(int width, int height, int channels)
{
    jpeg_image_t img;
    uint8_t * data;
    int size;

    memset(&img,0,sizeof(jpeg_image_t));

    size = width*height*channels;

    data= (uint8_t*)malloc(size);
    if(!data){
        printf("Error in JPEG allocation %d btyes\n",size);
        return img;
    }

    img.width = width;
    img.height = height;
    img.size = size;
    img.channels = channels;
    img.data = data;

    //printf("Array at %p\n",data);

    return img;
}

// https://gist.github.com/kentakuramochi/f64e7646f1db8335c80f131be8359044
int jpeg_save_file(char * filename, jpeg_image_t * img)
{
    struct jpeg_compress_struct cinfo;
    FILE * fp=NULL;
    uint8_t *row;
    uint32_t stride;
    int channels = 3;

    struct jpegErrorManager jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpegErrorExit;


    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        printf("Error in JPEG compress\n");
        jpeg_destroy_compress(&cinfo);
        if(fp) fclose(fp);
        return -1;
    }

    jpeg_create_compress(&cinfo);

    // https://stackoverflow.com/questions/4664087/write-a-jpeg-with-libjpeg-seg-fault
    //cinfo.err = jpeg_std_error(&jerr);

    fp = fopen(filename, "wb");
    if(!fp) {
        printf("Error: failed to open %s\n", filename);
        return -1;
    }

    jpeg_stdio_dest(&cinfo, fp);


    cinfo.image_width      = img->width;
    cinfo.image_height     = img->height;
    cinfo.input_components = img->channels;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, true);

    //printf("Here @ %s:%d\n",__func__,__LINE__);fflush(0);
    row = img->data;
    stride = img->width * channels;
    for (int y = 0; y < img->height; y++)
    {
        //printf("Write from %p\n",row);
       jpeg_write_scanlines(&cinfo, &row, 1);
       row += stride;
    }
    //printf("Here @ %s:%d\n",__func__,__LINE__);fflush(0);

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(fp);
    fp=NULL;

    return 0;
}

// EOF

