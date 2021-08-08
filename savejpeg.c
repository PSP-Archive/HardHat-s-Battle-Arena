#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <jpeglib.h>
#include "main.h"

void saveImageJpeg(const char* filename, Color* data, int width, int height, int lineSize)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *file;
	JSAMPROW row_pointer[1];
	unsigned char *row=0;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	if ((file = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return;
	}
	jpeg_stdio_dest(&cinfo, file);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);
	row=(unsigned char *)malloc(3*width);
	row_pointer[0]=(JSAMPROW)row;
	while (cinfo.next_scanline < cinfo.image_height) {
		int i;
		for(i=0;i<width;i++) {
			Color col=data[cinfo.next_scanline * lineSize + i];
			row[i*3]=col;
			row[i*3+1]=col>>8;
			row[i*3+2]=col>>16;
		}
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	fclose(file);
	jpeg_destroy_compress(&cinfo);
}

