#include "RPIViewerWindow.h"

#include <unistd.h>
#include <sys/time.h>

#include <FL/Fl_Return_Button.H>
#include <FL/Fl_RGB_Image.H>

#include "librpi.h"

#define HEAD_BOX_HIEGHT_PIXELS  30

RPIViewerWindow::RPIViewerWindow(int width, int height, const char * title)
    : Fl_Double_Window(width, height,title)
{

    head_group = new Fl_Group(0, 0, width, HEAD_BOX_HIEGHT_PIXELS, "");

    head_box = new Fl_Box(FL_NO_BOX, 0, 0, width, HEAD_BOX_HIEGHT_PIXELS, "Loading");
    head_box->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT );
    sprintf(head_box_text,"Loading...");
    head_box->label((const char *)head_box_text);

    btn1to1 = new Fl_Button(375, 0, 120, HEAD_BOX_HIEGHT_PIXELS, "Resize 1:1");
    btn1to1->callback(static_callback, this);

    // Align the text inside the box to the top left, like a
    // label widget in other toolkits
    box1 = new Fl_Box(FL_NO_BOX, 0, HEAD_BOX_HIEGHT_PIXELS, 100, 100, "");
    box1->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP);
#if 0
    box1->labeltype(FL_NO_LABEL);
    box1->labelfont(FL_HELVETICA_BOLD_ITALIC);
    box1->labelsize(20);
    box1->labelcolor(fl_rgb_color(0x00, 0x80, 0x00));
#endif
    box1->set_visible();
    box1->show();

    head_group->end();
    head_group->resizable(0);

    img = NULL;
    img_scaled = NULL;

    resizable(this);

    return;
}

RPIViewerWindow::~RPIViewerWindow()
{
    g_running = false;

    if(box1) delete box1;
    if(head_box) delete head_box;
    if(btn1to1) delete btn1to1;
    if(head_group) delete head_group;
    if(img){ DEBUG("Del image\n");  delete img;}
    if(img_scaled){ DEBUG("Del scaled image\n");  delete img_scaled;}
}


void RPIViewerWindow::load_image(char * filename)
{
    RPI_image_t * rpi;
    RPI_header_t * header;
    uint8_t * payload;
    int pixels;
    uint8_t * newimage_data;
    int newimage_size;

    Fl_RGB_Image * newimg = NULL;

    printf("Loading %s\n",filename);
    memset(img_filename,0,sizeof(img_filename));

    rpi = rpi_load_file(filename);
    if(!rpi){
        printf("Error loading file %s\n",filename);
        return;
    }

    header = &(rpi->header);
    payload = rpi->payload;

    pixels = rpi_get_pixel_count(header);

    // File is loaded!
    DEBUG("File loaded! %d bytes, %d pixels\n",rpi_get_payload_size(header), pixels);

    // Try and allocate an RGB image
    newimage_size = header->width * header->height * 3;
    newimage_data = (uint8_t*)malloc(newimage_size);
    if(!newimage_data){
        printf("Error allocating %d bytes for the image\n",newimage_size);
        rpi_free(rpi);
        return;
    }

    // Save the filename
    strncpy(img_filename,filename,sizeof(img_filename));

    // RGB image exists, now translate the RPI pixel data in to the RGB
    // pixel data
    {
        int r,c,i;
        uint16_t p;

        for(r=0;r<header->height;r++)
        {
            for(c=0;c<header->width;c++)
            {
                RGBA_t rgb;
                i = (r*header->width)+c;
                // Get the RGB vlaue of this pixel
                rgb = rpi_pixel_to_RGBA(rpi, i);
                // Set it in the RGB image data
                newimage_data[(i*3)] = rgb.R;
                newimage_data[(i*3)+1] = rgb.G;
                newimage_data[(i*3)+2] = rgb.B;
            }
        }
    }

    // Make the RGB image
    newimg = new Fl_RGB_Image(newimage_data, header->width, header->height);
    newimg->alloc_array=1; // Free the data when the image is destroyed

    // Save the new image as the image to display
    if(img){
        delete img;
        img=NULL;
    }
    img = newimg;

    // We don't need the RPI file any more, so free it
    rpi_free(rpi);
    rpi=NULL;

    display_image_1to1(img);

    return;
}

// Private and inherited functions

#if 0
// https://www.fltk.org/doc-1.3/events.html
// https://www.fltk.org/doc-1.3/enumerations.html
#include <Fl/names.h>
int RPIViewerWindow::handle(int event)
{

#if 0
    DEBUG("Event: %s (%d)\n", fl_eventnames[event],event);
#endif
#if 1
    switch(event){
    case FL_FOCUS:
        DEBUG("Focus attempt\n");
        return 1;
    #if 0
    case FL_KEYUP:
        DEBUG("Key up!\n");
        //if(handler) return handler->key_up_handler(event);
        break;
    case FL_KEYDOWN:
        DEBUG("Key down!\n");
        //if(handler) return handler->key_down_handler(event);
        break;
    #endif
    default:
        //DEBUG("Event: %s (%d)\n", fl_eventnames[event],event);
        break;
    }
#endif
    // 0 is we did NOT handle the event
    return 0;
}
#endif
void RPIViewerWindow::button_handler(Fl_Widget* w)
{
    if(w==btn1to1)
    {
        display_image_1to1(img);

    }
    return;
}

void RPIViewerWindow::display_image_1to1(Fl_Image * img_ptr)
{
    int w,h;
    if(!img_ptr) return;

#if 0
    w=800;
    h=600;

    if(img_ptr->w()<w) w=img_ptr->w();
    if(img_ptr->h()<h) h=img_ptr->h();
#else
    w = img_ptr->w();
    h = img_ptr->h();
#endif

    if(w<500) w=500;
    if(h<100) h=100;

    resize(x(),y(), w, h+HEAD_BOX_HIEGHT_PIXELS);
}

void RPIViewerWindow::display_image(Fl_Image * img_ptr)
{
    int win_w,win_h;
    float scale_w,scale_h;
    float scale;
    int new_w,new_h;
    Fl_Image * oldimg;

    if(!img_ptr) return;

    win_w = w();
    win_h = h()-HEAD_BOX_HIEGHT_PIXELS;

    DEBUG("img %dx%d, win %dx%d\n",img_ptr->w(),img_ptr->h(),win_w,win_h);
    scale_w = img_ptr->w()/(float)win_w;
    scale_h = img_ptr->h()/(float)win_h;
    DEBUG("Scale %0.4f x %0.4f\n",scale_w,scale_h);

    // Find the maximum to scale by
    scale = ((scale_w>scale_h)?scale_w:scale_h);

    // Save the old image
    oldimg = img_scaled;

    // make a new scaled image
    new_w = img_ptr->w()/scale;
    new_h = img_ptr->h()/scale;
    DEBUG("New size: %dx%d\n",new_w,new_h);
    img_scaled = img_ptr->copy(new_w,new_h);

    // Draw the image
    box1->image(img_scaled);
    box1->show();

    // Delete the old scaled image
    if(oldimg ){
        delete oldimg;
    }

    // Set the header box
#if 1
    sprintf(head_box_text,"File: %s\nOriginal Res: %dx%d - Display Res: %dx%d",
            img_filename,img_ptr->w(),img_ptr->h(),
            new_w,new_h);
#endif

    Fl::check();

    return;
}

void RPIViewerWindow::resize(int X, int Y, int W, int H)
{
    Fl_Double_Window::resize(X,Y,W,H);

    // TODO: Should find a better way to do this.  FLTK doesn't provide
    // an easy method to detect when the resize is stopped (aka when
    // the mouse button is released).  If there was, we should set a flag
    // and when the final resize happens we should resize the image.  But
    // since this doesn't exist, just resize constantly (more difficult
    // on the CPU, but hell, this is an image viewer not a daemon)
    display_image(img);

}

// EOF


