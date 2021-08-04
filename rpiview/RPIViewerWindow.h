#ifndef __RPI_VIEWER_WINDOW_H__
#define __RPI_VIEWER_WINDOW_H__

#include "defs.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Window.H>

#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>

class RPIViewerWindow: public Fl_Double_Window
{

public:
    RPIViewerWindow(int width, int height, const char * title);
    ~RPIViewerWindow();

    Fl_Box * box1;
    Fl_Box * head_box;

    Fl_Button * btn1to1;

    Fl_Group * head_group;

    Fl_Image * img;
    Fl_Image * img_scaled;

    void load_image(char * filename);

    void display_image(Fl_Image * img_ptr);

    void resize(int X, int Y, int W, int H);

    static void static_callback(Fl_Widget* w, void* data) {
        ((RPIViewerWindow*)data)->button_handler(w);
    }

private:

    char img_filename[100];
    char head_box_text[200];

    //int handle(int event);
    void button_handler(Fl_Widget* w);

    void display_image_1to1(Fl_Image * img_ptr);

};

#endif
