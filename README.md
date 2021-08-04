# Raw Pixel Image (RPI) image file format

## Overview

This project is for the Raw Pixel Image (RPI) file format, a format used to hold image information
and uncompressed pixel data in multiple pixel encodings.

RPI format is designed to allow for the low
processing overhead handling of pixel data to and from pixel based displays and input sources while
providing a format compatible with editing and display applications.

RPI format supports multiple pixels
encodings, both with and without alpha channel.

## Sub-projects
 - librpi - C library to work with RPI files
 - rpiconv - C program to convert JPEG, BMP, and RPI files
 - rpiview - FLTK application to view RPI files

## Building

The Makefile in the root will build both rpiconv and rpiview

### Libraries
 - FLTK
 - libz
 - libjpeg
 - X11 headers (for viewer)

### Library apt 
apt install libjpeg-dev zlib1g libx11-dev libfltk-dev

# rpiconv

Conversion and inspection tool for RPI files. 

## Functions
- RPI inspection
- JPEG -> RPI
- RPI -> JPEG
- BMP -> RPI

## Building
```
cd rpiconv
make
./rpiconv
```
## Libraries
 - libz
 - libjpeg

## Usage
```
Usage: rpiconv [-i] <input file> [-o] <output file> [-f format] [-c cmd] [-C comment] [-Nvh?]
   [-i] file	Input file
   [-o] file	Output file

   -c command   Operation to run
                    - dumprpi (default)
                    - jpeg2rpi
                    - rpi2jpeg
                    - bmp2rpi
   -f format    Pixel format
                    - RGB565 (default)
                    - BGR565
                    - YUYV
                    - UYUV
                    - RGB5551
                    - RGB5515
   -C           RPI comment string (max 15 bytes)
   -N           No comment
   -v           Enable verbose output
   -h?          Program help (This output)
```

# rpiview

Viewer application for RPI files

## Building
```
cd rpiview
make
./rpiview
```

## Libraries
 - libz
 - FLTK 1.3

## Usage
```
Usage: rpiview [-i] <filename> [-vh?]
   [-i] <filename>	Input image file
   -v               Enable verbose output
   -h?              Program help (This output)
```
