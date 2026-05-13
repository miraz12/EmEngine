// Single translation unit for audio decoder implementations.
// These are header-only libraries that need exactly one TU with
// the IMPLEMENTATION define.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#define DR_MP3_IMPLEMENTATION
#include <dr_mp3.h>

// stb_vorbis: include without HEADER_ONLY to compile the implementation
#include <stb_vorbis.c>

#pragma clang diagnostic pop
