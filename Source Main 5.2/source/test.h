#pragma once

#include <jpeglib.h>

extern bool OpenJpegBuffer(char *filename,float *BufferFloat);
extern bool WriteJpeg(char *filename,int Width,int Height,unsigned char *Buffer,int quality);