#pragma once

#ifdef _MU_SDL_FILE

#include <SDL.h>
#include <string>
#include <vector>
#include <jpeglib.h>
#include <stdlib.h>
#include <string.h>

#define MU_JPEG_BUF_SIZE 4096
#define MU_JPEG_SRC_BUF_SIZE 4096

#ifndef BOOL
#define BOOL int
#endif

inline std::string MU_NormalizePath(const char* path)
{
    std::string p = path ? path : "";

    for (char& c : p)
    {
        if (c == '\\')
            c = '/';
    }

    return p;
}

using MU_FILE = SDL_RWops;

inline MU_FILE* MU_fopen(const char* path, const char* mode)
{
    char t[100] = { 0 };
    sprintf(t, "[SDL-DEBUG] MU_fopen %s", path);
    OutputDebugStringA(t);
    std::string fixed = MU_NormalizePath(path);
    return SDL_RWFromFile(fixed.c_str(), mode);
}

inline int MU_fclose(MU_FILE* fp)
{
    return fp ? SDL_RWclose(fp) : 0;
}

inline size_t MU_fread(void* ptr, size_t size, size_t count, MU_FILE* fp)
{
    if (!fp)
        return 0;

    return SDL_RWread(fp, ptr, size, count);
}

inline size_t MU_fwrite(const void* ptr, size_t size, size_t count, MU_FILE* fp)
{
    if (!fp)
        return 0;

    return SDL_RWwrite(fp, ptr, size, count);
}

inline int MU_fseek(MU_FILE* fp, long offset, int origin)
{
    if (!fp)
        return -1;

    return SDL_RWseek(fp, offset, origin) < 0 ? -1 : 0;
}

inline long MU_ftell(MU_FILE* fp)
{
    if (!fp)
        return -1;

    return (long)SDL_RWtell(fp);
}

inline int MU_fprintf(MU_FILE* fp, const char* fmt, ...)
{
    if (!fp || !fmt)
        return -1;

    char buffer[4096];

    va_list args;
    va_start(args, fmt);

    int len = SDL_vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    if (len < 0)
        return -1;

    int writeLen = len;

    if (writeLen >= (int)sizeof(buffer))
        writeLen = (int)sizeof(buffer) - 1;

    size_t written = SDL_RWwrite(fp, buffer, 1, writeLen);

    return (written == (size_t)writeLen) ? len : -1;
}

inline int MU_fscanf(MU_FILE* fp, const char* fmt, ...)
{
    if (!fp || !fmt)
        return -1;

    Sint64 cur = SDL_RWtell(fp);

    Sint64 end = SDL_RWsize(fp);

    if (cur < 0 || end < 0 || cur >= end)
        return EOF;

    Sint64 remain = end - cur;

    std::vector<char> buffer((size_t)remain + 1);

    size_t readBytes = SDL_RWread(fp, buffer.data(), 1, (size_t)remain);

    buffer[readBytes] = '\0';

    va_list args;
    va_start(args, fmt);

    int ret = vsscanf(buffer.data(), fmt, args);

    va_end(args);

    return ret;
}

inline int MU_putc(int c, MU_FILE* fp)
{
    if (!fp)
        return EOF;

    unsigned char ch = (unsigned char)c;

    size_t written = SDL_RWwrite(fp, &ch, 1, 1);

    return (written == 1) ? ch : EOF;
}

inline int MU_feof(MU_FILE* fp)
{
    if (!fp)
        return 1;

    Sint64 pos = SDL_RWtell(fp);
    Sint64 size = SDL_RWsize(fp);

    if (pos < 0 || size < 0)
        return 1;

    return (pos >= size) ? 1 : 0;
}

struct MU_JpegSDLDest
{
    jpeg_destination_mgr pub;
    SDL_RWops* rw;
    JOCTET* buffer;
};

inline static void MU_JpegInitDestination(j_compress_ptr cinfo)
{
    MU_JpegSDLDest* dest = (MU_JpegSDLDest*)cinfo->dest;

    dest->buffer = (JOCTET*)(*cinfo->mem->alloc_small)(
        (j_common_ptr)cinfo,
        JPOOL_IMAGE,
        MU_JPEG_BUF_SIZE * sizeof(JOCTET)
        );

    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = MU_JPEG_BUF_SIZE;
}

inline static BOOL MU_JpegEmptyOutputBuffer(j_compress_ptr cinfo)
{
    MU_JpegSDLDest* dest = (MU_JpegSDLDest*)cinfo->dest;

    size_t written = SDL_RWwrite(dest->rw, dest->buffer, 1, MU_JPEG_BUF_SIZE);

    if (written != MU_JPEG_BUF_SIZE)
        return FALSE;

    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = MU_JPEG_BUF_SIZE;

    return TRUE;
}

inline static void MU_JpegTermDestination(j_compress_ptr cinfo)
{
    MU_JpegSDLDest* dest = (MU_JpegSDLDest*)cinfo->dest;

    size_t datacount = MU_JPEG_BUF_SIZE - dest->pub.free_in_buffer;

    if (datacount > 0)
    {
        SDL_RWwrite(dest->rw, dest->buffer, 1, datacount);
    }
}

inline void MU_jpeg_stdio_dest(j_compress_ptr cinfo, SDL_RWops* rw)
{
    MU_JpegSDLDest* dest;

    if (cinfo->dest == NULL)
    {
        cinfo->dest = (jpeg_destination_mgr*)
            (*cinfo->mem->alloc_small)(
                (j_common_ptr)cinfo,
                JPOOL_PERMANENT,
                sizeof(MU_JpegSDLDest)
                );
    }

    dest = (MU_JpegSDLDest*)cinfo->dest;
    dest->rw = rw;

    dest->pub.init_destination = MU_JpegInitDestination;
    dest->pub.empty_output_buffer = MU_JpegEmptyOutputBuffer;
    dest->pub.term_destination = MU_JpegTermDestination;
}

struct MU_JpegSDLSource
{
    jpeg_source_mgr pub;

    SDL_RWops* rw;

    JOCTET* buffer;

    BOOL start_of_file;
};


inline static void MU_JpegInitSource(j_decompress_ptr cinfo)
{
    MU_JpegSDLSource* src = (MU_JpegSDLSource*)cinfo->src;

    src->start_of_file = TRUE;
}

inline static BOOL MU_JpegFillInputBuffer(j_decompress_ptr cinfo)
{
    MU_JpegSDLSource* src = (MU_JpegSDLSource*)cinfo->src;

    size_t nbytes = SDL_RWread(
        src->rw,
        src->buffer,
        1,
        MU_JPEG_SRC_BUF_SIZE
    );

    if (nbytes <= 0)
    {
        // fake EOI marker
        src->buffer[0] = (JOCTET)0xFF;
        src->buffer[1] = (JOCTET)JPEG_EOI;

        nbytes = 2;
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;

    src->start_of_file = FALSE;

    return TRUE;
}

inline static void MU_JpegSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
    MU_JpegSDLSource* src = (MU_JpegSDLSource*)cinfo->src;

    if (num_bytes > 0)
    {
        while (num_bytes > (long)src->pub.bytes_in_buffer)
        {
            num_bytes -= (long)src->pub.bytes_in_buffer;

            MU_JpegFillInputBuffer(cinfo);
        }

        src->pub.next_input_byte += num_bytes;
        src->pub.bytes_in_buffer -= num_bytes;
    }
}

inline static void MU_JpegTermSource(j_decompress_ptr cinfo)
{
}

inline void MU_jpeg_stdio_src(j_decompress_ptr cinfo, SDL_RWops* rw)
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
    }

    src = (MU_JpegSDLSource*)cinfo->src;

    if (src->buffer == NULL)
    {
        src->buffer = (JOCTET*)
            (*cinfo->mem->alloc_small)(
                (j_common_ptr)cinfo,
                JPOOL_PERMANENT,
                MU_JPEG_SRC_BUF_SIZE * sizeof(JOCTET)
                );
    }

    src->rw = rw;

    src->pub.init_source = MU_JpegInitSource;
    src->pub.fill_input_buffer = MU_JpegFillInputBuffer;
    src->pub.skip_input_data = MU_JpegSkipInputData;
    src->pub.resync_to_restart = jpeg_resync_to_restart;
    src->pub.term_source = MU_JpegTermSource;

    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = NULL;
}

#else

#include <cstdio>

using MU_FILE = FILE;

#define MU_fopen  fopen
#define MU_fclose fclose
#define MU_fread  fread
#define MU_fseek  fseek
#define MU_ftell  ftell
#define MU_fwrite fwrite
#define MU_feof feof
#define MU_fscanf fscanf
#define MU_putc putc
#define MU_fprintf fprintf

#define MU_jpeg_stdio_src jpeg_stdio_src
#define MU_jpeg_stdio_dest jpeg_stdio_dest

#endif