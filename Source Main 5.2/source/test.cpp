///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "mu_file.h"


struct my_error_mgr
{
	struct jpeg_error_mgr pub;
	//jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	//longjmp(myerr->setjmp_buffer, 1);
}

typedef float vec3_t[3];

vec3_t TerrainLight[1000];

int main(){
	OpenJpegBuffer("/data/test.jpg",&TerrainLight[0][0]);
	return 0;
}

bool WriteJpeg(char *filename,int Width,int Height,unsigned char *Buffer,int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	MU_FILE * outfile;
	JSAMPROW row_pointer[1];	
	int row_stride;	
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	
	if ((outfile = MU_fopen(filename, "wb")) == NULL)
	{
		return FALSE;
	}
	MU_jpeg_stdio_dest(&cinfo, outfile);
	
	cinfo.image_width = Width; 
	cinfo.image_height = Height;
	cinfo.input_components = 3;		
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = cinfo.image_width * 3;
	
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = &Buffer[(Height-1-cinfo.next_scanline) * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	
	jpeg_finish_compress(&cinfo);
	MU_fclose(outfile);
	jpeg_destroy_compress(&cinfo);
	return TRUE;
}

void MU_jpeg_sdl_src(j_decompress_ptr cinfo, SDL_RWops* rw)
{
	MU_JpegSDLSource* src;

	if (cinfo->src == NULL)
	{
		cinfo->src = (jpeg_source_mgr*)
			(*cinfo->mem->alloc_small)(
				(j_common_ptr)cinfo,
				JPOOL_PERMANENT,
				sizeof(MU_JpegSDLSource)
				);

		memset(cinfo->src, 0, sizeof(MU_JpegSDLSource)); // important
	}

	src = (MU_JpegSDLSource*)cinfo->src;

	src->rw = rw;

	src->buffer = (JOCTET*)
		(*cinfo->mem->alloc_small)(
			(j_common_ptr)cinfo,
			JPOOL_PERMANENT,
			MU_JPEG_SRC_BUF_SIZE * sizeof(JOCTET)
			);

	src->pub.init_source = MU_JpegInitSource;
	src->pub.fill_input_buffer = MU_JpegFillInputBuffer;
	src->pub.skip_input_data = MU_JpegSkipInputData;
	src->pub.resync_to_restart = jpeg_resync_to_restart;
	src->pub.term_source = MU_JpegTermSource;

	src->pub.bytes_in_buffer = 0;
	src->pub.next_input_byte = NULL;
}

bool OpenJpegBuffer(char *filename,float *BufferFloat)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	MU_FILE * infile;		
	JSAMPARRAY buffer;	
	int row_stride;	
	
	char FileName[256];

	char NewFileName[256];
	int iTextcnt = 0;
	for(int i=0;i<(int)strlen(filename);i++)
	{
		iTextcnt = i;
		NewFileName[i] = filename[i];
		if(filename[i]=='.') break;
	}
	NewFileName[iTextcnt+1] = NULL;
	strcpy(FileName,"Data\\");
    strcat(FileName,NewFileName);
	strcat(FileName,"OZJ");

	if((infile = MU_fopen(FileName, "rb")) == NULL)
	{
		return false;
	}
	
	MU_fseek(infile,24,SEEK_SET);
	
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		MU_fclose(infile);
		return false;
	}*/
	jpeg_create_decompress(&cinfo);

	MU_jpeg_sdl_src(&cinfo, infile);

	(void) jpeg_read_header(&cinfo, TRUE);
	
	(void) jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	
	unsigned char *Buffer = (unsigned char*) new char [cinfo.output_width*cinfo.output_height*cinfo.output_components];
	while (cinfo.output_scanline < cinfo.output_height) 
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(Buffer+(cinfo.output_height-cinfo.output_scanline)*row_stride,buffer[0],row_stride);
	}
	int Index = 0;
	for(unsigned int y=0;y<cinfo.output_height;y++)
	{
		for(unsigned int x=0;x<cinfo.output_width;x++)
		{
			BufferFloat[Index  ] = (float)Buffer[Index  ]/255.f;
			BufferFloat[Index+1] = (float)Buffer[Index+1]/255.f;
			BufferFloat[Index+2] = (float)Buffer[Index+2]/255.f;
			Index += 3;
		}
	}
	//SAFE_DELETE_ARRAY(Buffer);
	
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	MU_fclose(infile);
	return true;
}

