// this provides a more efficient compression algorithm for STBIW, as per notice in header
#ifdef ZLIB_NG_FOUND
#include <zlib-ng.h>
#endif

extern "C" {
#ifdef ZLIB_NG_FOUND
static uint8_t* stbiw_compress(uint8_t* data, int dataLen, int* outLenOut, [[maybe_unused]] int quality)
{
    uint8_t* output = new uint8_t[dataLen];
    size_t outLen = dataLen;

    if (zng_compress2(output, &outLen, data, dataLen, Z_BEST_COMPRESSION) != Z_OK)
    {
        delete[] output;
        output = nullptr;
        outLen = 0;
    }

    *outLenOut = outLen;
    return output;
}
#define STBIW_ZLIB_COMPRESS stbiw_compress
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb/stb_image_write.h"
}
