#include "defs.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>

#include "operations.h"

// Definitions
// ----------------------------------------------------------------------------

// Types and enums
// ----------------------------------------------------------------------------

// Variables
// ----------------------------------------------------------------------------
bool g_running = false;
bool g_verbose = false;


// Local prototypes
// ----------------------------------------------------------------------------

// Functions
// ----------------------------------------------------------------------------
void sigint_handler(int arg)
{
	printf("<---- catch ctrl-c\n");
    g_running = false;
}


void usage(bool allinfo)
{
    if(allinfo){
        printf("\nBuild %s %s\n",__DATE__,__TIME__);
        printf("CLI Systems LLC - admin@clisystems.com\n\n");
    }

    printf("Usage: rpiconv [-i] <input file> [-o] <output file> [-f format] [-c cmd] [-C comment] [-Nvh?]\n");
    printf("   [-i] file	Input file\n");
    printf("   [-o] file	Output file\n");
    printf("\n");
    printf("   -c command   Operation to run\n");
    printf("                    - dumprpi (default)\n");
    printf("                    - jpeg2rpi\n");
    printf("                    - rpi2jpeg\n");
    printf("                    - bmp2rpi\n");
    printf("   -f format    Pixel format\n");
    printf("                    - RGB565 (default)\n");
    printf("                    - BGR565\n");
    printf("                    - YUYV\n");
    printf("                    - UYUV\n");
    printf("                    - RGB5551\n");
    printf("                    - RGB5515\n");
    printf("   -C           RPI comment string (max 15 bytes)\n");
    printf("   -N           No comment\n");
    printf("   -v           Enable verbose output\n");
    printf("   -h?          Program help (This output)\n");
    exit(0);
}


// Main function
// ----------------------------------------------------------------------------
int main(int argc,char** argv)
{
    int tty_fd;
    fd_set rfds;
    struct timeval tv;
    int retval;
    int optchar;
    char in_filename[100];
    char out_filename[100];
    char command[20];
    char format_str[20];
    char comment[16];
    RPI_format_e format = RPI_FORMAT_RGB565;
    
    memset(in_filename,0,sizeof(in_filename));
    memset(out_filename,0,sizeof(out_filename));
    memset(command,0,sizeof(command));
    
    // Default comment
    sprintf(comment,"RPITool %s",PROG_VERSION);

    // Info about program
    printf("RPI conversion tool, v%s\n",PROG_VERSION);

    struct option longopts[] = {
    { "verbose", no_argument,       0, 'v' },
    { 0, 0, 0, 0 }
    };

    // Process the command line options
    while ((optchar = getopt_long(argc, argv, "NC:f:i:o:c:vh?", \
           longopts, NULL)) != -1)
    {
        switch (optchar)
        {
        case 'i':
            strncpy(in_filename,optarg,sizeof(in_filename)-1);
            break;
        case 'o':
            strncpy(out_filename,optarg,sizeof(out_filename)-1);
            break;
        case 'f':
            strncpy(format_str,optarg,sizeof(format_str)-1);
            break;
        case 'C':
            strncpy(comment,optarg,sizeof(comment));
            DEBUG("Comment: '%s'\n",comment);
            break;
        case 'N':
            memset(comment,0,sizeof(comment));
            break;
        case 'c':
            DEBUG("Command: %s\n",optarg);
            strncpy(command,optarg,sizeof(command)-1);
            break;
        case 'v':
            DEBUG("Verbose = true\n");
            g_verbose = true;
            break;
        case 'h':
            usage(false);
            break;
        default:
           usage(true);
           break;
        }
    };
    
    // Process non-flagged arguments
    for (int index = optind; index < argc; index++)
    {
        DEBUG("Non-option argument %s\n", argv[index]);

        // Handle the input filename
        if(index==optind && strlen(in_filename)<=0)
            strncpy(in_filename,argv[index],sizeof(in_filename)-1);

        // Handle the output filename
        if(index==optind+1 && strlen(out_filename)<=0)
            strncpy(out_filename,argv[index],sizeof(out_filename)-1);
    }


    // Setup system
    g_running = true;
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    
    // Process format argument
    if(cmp_const(format_str,"RGB565")){
        format = RPI_FORMAT_RGB565;
    }else if(cmp_const(format_str,"BGR565")){
        format = RPI_FORMAT_BGR565;
    }else if(cmp_const(format_str,"YUYV")){
        format = RPI_FORMAT_YUYV;
    }else if(cmp_const(format_str,"UYUV")){
        format = RPI_FORMAT_UYUV;
    }else if(cmp_const(format_str,"RGB5551")){
        format = RPI_FORMAT_RGBA5551;
    }else if(cmp_const(format_str,"RGB5515")){
        format = RPI_FORMAT_RGAB5515;
    }

    if(strlen(in_filename)<=0)
    {
        printf("Must specify an input file name, -h or -? for help\n");
        exit(0);
    }

    // Do operation
    if(cmp_const(command,"jpeg2rpi")){
        
        do_jpeg2rpi(in_filename,out_filename, format, comment);
 
    }else if(cmp_const(command,"rpi2jpeg")){
        
        do_rpi2jpeg(in_filename,out_filename);

    }else if(cmp_const(command,"bmp2rpi")){

        do_bmp2rpi(in_filename,out_filename, format, comment);

    //}else if(cmp_const(command,"dumprpi")){
    }else{
        do_dump_rpi(in_filename);
    }
    
	// Shutdown system	
    DEBUG("*Normal exit\n");
    
    return EXIT_SUCCESS;
}

// EOF
