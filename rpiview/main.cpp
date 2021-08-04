#include "defs.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>

#include <FL/Fl_Image.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Shared_Image.H>
#include <X11/Xlib.h>

#include "RPIViewerWindow.h"

// Macros
// ---------------------------------------------------------------------------

// Definitions
// ----------------------------------------------------------------------------

// Types and enums
// ----------------------------------------------------------------------------

// Variables
// ----------------------------------------------------------------------------
bool g_running = false;
bool g_verbose = false;

static RPIViewerWindow *window;

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

    printf("Usage: rpiview [-i] <filename> [-vh?]\n");
    printf("   [-i] <filename>	Input image file\n");
    printf("   -v               Enable verbose output\n");
    printf("   -h?              Program help (This output)\n");
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
    int ret;
    
    memset(in_filename,0,sizeof(in_filename));
    
    // Info about program
    printf("RPI image viewer, v%s\n",PROG_VERSION);

    #if 0
    for(int x=0;x<argc;x++)
        DEBUG("Arg %d: '%s'\n",x,argv[x]);
    #endif

    struct option longopts[] = {
    { "verbose", no_argument,       0, 'v' },
    { 0, 0, 0, 0 }
    };

    // Process the command line options
    while ((optchar = getopt_long(argc, argv, "i:o:c:vh?", \
           longopts, NULL)) != -1)
    {
        DEBUG("Process arg '%c' - '%s'\n",optchar,optarg);
       switch (optchar)
       {
       case 'i':
            strncpy(in_filename,optarg,sizeof(in_filename)-1);
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
    }

    // Setup system
    g_running = true;
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    
    XInitThreads();

    DEBUG("in_filename: %s\n",in_filename);

    // Check the file
    if( access( in_filename, F_OK )<0 ) {
        // file doesn't exist
        printf("Can't open %s, does file exist?\n",in_filename);
        exit(1);
    }


    // Create the window and load the image
    window = new RPIViewerWindow(800,600,"RPI Viewer");
    Fl::focus(window);

    if (window)
    {
        if(strlen(in_filename)>0)
            window->load_image(in_filename);

        // Start the actual GUI
        window->show();
        ret = Fl::run();
        DEBUG("Application returned: %d\n",ret);

        // Shutdown system
        delete window;
    }

    
	// Shutdown system	
    DEBUG("*Normal exit\n");
    
    return EXIT_SUCCESS;
}

// EOF
