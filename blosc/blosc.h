/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/
#ifndef BLOSC_H
#define BLOSC_H

#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include "blosc-export.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Version numbers */
#define BLOSC_VERSION_MAJOR    2    /* for major interface/format changes  */
#define BLOSC_VERSION_MINOR    0    /* for minor interface/format changes  */
#define BLOSC_VERSION_RELEASE  0    /* for tweaks, bug-fixes, or development */

#define BLOSC_VERSION_STRING   "2.0.0a2"  /* string version.  Sync with above! */
#define BLOSC_VERSION_REVISION "$Rev$"   /* revision version */
#define BLOSC_VERSION_DATE     "$Date:: 2016-01-08 #$"    /* date version */

#define BLOSCLZ_VERSION_STRING "1.0.5"   /* the internal compressor version */

/* The *_FORMAT symbols should be just 1-byte long */
#define BLOSC_VERSION_FORMAT    2   /* Blosc format version, starting at 1 */

/* Minimum header length */
#define BLOSC_MIN_HEADER_LENGTH 16

/* The maximum overhead during compression in bytes.  This equals to
   BLOSC_MIN_HEADER_LENGTH now, but can be higher in future
   implementations */
#define BLOSC_MAX_OVERHEAD BLOSC_MIN_HEADER_LENGTH

/* Maximum source buffer size to be compressed */
#define BLOSC_MAX_BUFFERSIZE (INT_MAX - BLOSC_MAX_OVERHEAD)

/* Maximum typesize before considering source buffer as a stream of bytes */
#define BLOSC_MAX_TYPESIZE 255         /* Cannot be larger than 255 */

/* Codes for filters (see blosc_compress) */
#define BLOSC_NOSHUFFLE   0  /* no shuffle (for compatibility with Blosc1) */
#define BLOSC_NOFILTER    0  /* no filter */
#define BLOSC_SHUFFLE     1  /* byte-wise shuffle */
#define BLOSC_BITSHUFFLE  2  /* bit-wise shuffle */
#define BLOSC_DELTA       3  /* delta filter */

/* Maximum number of simultaneous filters */
#define BLOSC_MAX_FILTERS 5

/* Codes for internal flags (see blosc_cbuffer_metainfo) */
#define BLOSC_DOSHUFFLE     0x1  /* byte-wise shuffle */
#define BLOSC_MEMCPYED      0x2  /* plain copy */
#define BLOSC_DOBITSHUFFLE  0x4  /* bit-wise shuffle */
#define BLOSC_FILTER_SCHUNK 0x8  /* filter defined in super-chunk */

/* Codes for the different compressors shipped with Blosc */
#define BLOSC_BLOSCLZ        0
#define BLOSC_LZ4            1
#define BLOSC_LZ4HC          2
#define BLOSC_SNAPPY         3
#define BLOSC_ZLIB           4
#define BLOSC_ZSTD           5
#define BLOSC_LZ5            6
#define BLOSC_LZ5HC          7

/* Names for the different compressors shipped with Blosc */
#define BLOSC_BLOSCLZ_COMPNAME   "blosclz"
#define BLOSC_LZ4_COMPNAME       "lz4"
#define BLOSC_LZ4HC_COMPNAME     "lz4hc"
#define BLOSC_SNAPPY_COMPNAME    "snappy"
#define BLOSC_ZLIB_COMPNAME      "zlib"
#define BLOSC_ZSTD_COMPNAME      "zstd"
#define BLOSC_LZ5_COMPNAME       "lz5"
#define BLOSC_LZ5HC_COMPNAME     "lz5hc"

/* Codes for compression libraries shipped with Blosc (code must be < 8) */
#define BLOSC_BLOSCLZ_LIB    0
#define BLOSC_LZ4_LIB        1
#define BLOSC_SNAPPY_LIB     2
#define BLOSC_ZLIB_LIB       3
#define BLOSC_ZSTD_LIB       4
#define BLOSC_LZ5_LIB        5
#define BLOSC_SCHUNK_LIB     7   /* compressor library in super-chunk header */

/* Names for the different compression libraries shipped with Blosc */
#define BLOSC_BLOSCLZ_LIBNAME   "BloscLZ"
#define BLOSC_LZ4_LIBNAME       "LZ4"
#define BLOSC_SNAPPY_LIBNAME    "Snappy"
#if defined(HAVE_MINIZ)
  #define BLOSC_ZLIB_LIBNAME    "Zlib (via miniz)"
#else
  #define BLOSC_ZLIB_LIBNAME    "Zlib"
#endif	/* HAVE_MINIZ */
#define BLOSC_ZSTD_LIBNAME      "Zstd"
#define BLOSC_LZ5_LIBNAME       "LZ5"

/* The codes for compressor formats shipped with Blosc */
#define BLOSC_BLOSCLZ_FORMAT  BLOSC_BLOSCLZ_LIB
#define BLOSC_LZ4_FORMAT      BLOSC_LZ4_LIB
/* LZ4HC and LZ4 share the same format */
#define BLOSC_LZ4HC_FORMAT    BLOSC_LZ4_LIB
#define BLOSC_SNAPPY_FORMAT   BLOSC_SNAPPY_LIB
#define BLOSC_ZLIB_FORMAT     BLOSC_ZLIB_LIB
#define BLOSC_ZSTD_FORMAT     BLOSC_ZSTD_LIB
#define BLOSC_LZ5_FORMAT      BLOSC_LZ5_LIB
/* LZ5HC and LZ5 share the same format */
#define BLOSC_LZ5HC_FORMAT    BLOSC_LZ5_LIB


/* The version formats for compressors shipped with Blosc */
/* All versions here starts at 1 */
#define BLOSC_BLOSCLZ_VERSION_FORMAT  1
#define BLOSC_LZ4_VERSION_FORMAT      1
#define BLOSC_LZ4HC_VERSION_FORMAT    1  /* LZ4HC and LZ4 share the same format */
#define BLOSC_SNAPPY_VERSION_FORMAT   1
#define BLOSC_ZLIB_VERSION_FORMAT     1
#define BLOSC_ZSTD_VERSION_FORMAT     1
#define BLOSC_LZ5_VERSION_FORMAT      1
#define BLOSC_LZ5HC_VERSION_FORMAT    1  /* LZ5HC and LZ5 share the same format */

/**
  Initialize the Blosc library environment.

  You must call this previous to any other Blosc call, unless you want
  Blosc to be used simultaneously in a multi-threaded environment, in
  which case you should *exclusively* use the
  blosc_compress_ctx()/blosc_decompress_ctx() pair (see below).
  */
BLOSC_EXPORT void blosc_init(void);


/**
  Destroy the Blosc library environment.

  You must call this after to you are done with all the Blosc calls,
  unless you have not used blosc_init() before (see blosc_init()
  above).
  */
BLOSC_EXPORT void blosc_destroy(void);


/**
  Compress a block of data in the `src` buffer and returns the size of
  compressed block.  The size of `src` buffer is specified by
  `nbytes`.  There is not a minimum for `src` buffer size (`nbytes`).

  `clevel` is the desired compression level and must be a number
  between 0 (no compression) and 9 (maximum compression).

  `doshuffle` specifies whether the shuffle compression preconditioner
  should be applied or not.  BLOSC_NOFILTER means not applying filters,
  BLOSC_SHUFFLE means applying shuffle at a byte level and
  BLOSC_BITSHUFFLE at a bit level (slower but may achieve better
  entropy alignment).

  `typesize` is the number of bytes for the atomic type in binary
  `src` buffer.  This is mainly useful for the shuffle preconditioner.
  For implementation reasons, only a 1 < typesize < 256 will allow the
  shuffle filter to work.  When typesize is not in this range, shuffle
  will be silently disabled.

  The `dest` buffer must have at least the size of `destsize`.  Blosc
  guarantees that if you set `destsize` to, at least,
  (`nbytes`+BLOSC_MAX_OVERHEAD), the compression will always succeed.
  The `src` buffer and the `dest` buffer can not overlap.

  Compression is memory safe and guaranteed not to write the `dest`
  buffer more than what is specified in `destsize`.

  If `src` buffer cannot be compressed into `destsize`, the return
  value is zero and you should discard the contents of the `dest`
  buffer.

  A negative return value means that an internal error happened.  This
  should never happen.  If you see this, please report it back
  together with the buffer data causing this and compression settings.
  */
BLOSC_EXPORT int blosc_compress(int clevel, int doshuffle, size_t typesize,
                                size_t nbytes, const void* src, void* dest,
                                size_t destsize);


/**
  Context interface to blosc compression. This does not require a call
  to blosc_init() and can be called from multithreaded applications
  without the global lock being used, so allowing Blosc be executed
  simultaneously in those scenarios.

  It uses the same parameters than the blosc_compress() function plus:

  `compressor`: the string representing the type of compressor to use.

  `blocksize`: the requested size of the compressed blocks.  If 0, an
   automatic blocksize will be used.

  `numinternalthreads`: the number of threads to use internally.

  A negative return value means that an internal error happened.  This
  should never happen.  If you see this, please report it back
  together with the buffer data causing this and compression settings.
*/
BLOSC_EXPORT int blosc_compress_ctx(int clevel, int doshuffle, size_t typesize,
                                    size_t nbytes, const void* src, void* dest,
                                    size_t destsize, const char* compressor,
                                    size_t blocksize, int numinternalthreads);


/**
  Decompress a block of compressed data in `src`, put the result in
  `dest` and returns the size of the decompressed block.

  The `src` buffer and the `dest` buffer can not overlap.

  Decompression is memory safe and guaranteed not to write the `dest`
  buffer more than what is specified in `destsize`.

  If an error occurs, e.g. the compressed data is corrupted or the
  output buffer is not large enough, then 0 (zero) or a negative value
  will be returned instead.
*/
BLOSC_EXPORT int blosc_decompress(const void* src, void* dest, size_t destsize);


/**
  Context interface to blosc decompression. This does not require a
  call to blosc_init() and can be called from multithreaded
  applications without the global lock being used, so allowing Blosc
  be executed simultaneously in those scenarios.

  It uses the same parameters than the blosc_decompress() function plus:

  `numinternalthreads`: number of threads to use internally.

  Decompression is memory safe and guaranteed not to write the `dest`
  buffer more than what is specified in `destsize`.

  If an error occurs, e.g. the compressed data is corrupted or the
  output buffer is not large enough, then 0 (zero) or a negative value
  will be returned instead.
*/
BLOSC_EXPORT int blosc_decompress_ctx(const void* src, void* dest,
                                      size_t destsize, int numinternalthreads);


/**
  Get `nitems` (of typesize size) in `src` buffer starting in `start`.
  The items are returned in `dest` buffer, which has to have enough
  space for storing all items.

  Returns the number of bytes copied to `dest` or a negative value if
  some error happens.
  */
BLOSC_EXPORT int blosc_getitem(const void* src, int start, int nitems, void* dest);


/**
  Initialize a pool of threads for compression/decompression.  If
  `nthreads` is 1, then the serial version is chosen and a possible
  previous existing pool is ended.  If this is not called, `nthreads`
  is set to 1 internally.

  Returns the previous number of threads.
  */
BLOSC_EXPORT int blosc_set_nthreads(int nthreads);


/**
  Select the compressor to be used.  The supported ones are "blosclz",
  "lz4", "lz4hc", "snappy", "zlib", "ztsd" "lz5" and "lz5hc".  If this
  function is not called, then "blosclz" will be used.

  In case the compressor is not recognized, or there is not support
  for it in this build, it returns a -1.  Else it returns the code for
  the compressor (>=0).
  */
BLOSC_EXPORT int blosc_set_compressor(const char* compname);


/**
  Get the `compname` associated with the `compcode`.

  If the compressor code is not recognized, or there is not support
  for it in this build, -1 is returned.  Else, the compressor code is
  returned.
 */
BLOSC_EXPORT int blosc_compcode_to_compname(int compcode, char** compname);


/**
  Return the compressor code associated with the compressor name.

  If the compressor name is not recognized, or there is not support
  for it in this build, -1 is returned instead.
 */
BLOSC_EXPORT int blosc_compname_to_compcode(const char* compname);


/**
  Get a list of compressors supported in the current build.  The
  returned value is a string with a concatenation of "blosclz", "lz4",
  "lz4hc", "snappy", "zlib", "zstd", "lz5" or "lz5hc" separated by
  commas, depending on which ones are present in the build.

  This function does not leak, so you should not free() the returned
  list.

  This function should always succeed.
  */
BLOSC_EXPORT char* blosc_list_compressors(void);


/**
  Return the version of blosc in string format.

  Useful for dynamic libraries.
*/
BLOSC_EXPORT char* blosc_get_version_string(void);


/**
  Get info from compression libraries included in the current build.
  In `compname` you pass the compressor name that you want info from.
  In `complib` and `version` you get the compression library name and
  version (if available) as output.

  In `complib` and `version` you get a pointer to the compressor
  library name and the version in string format respectively.  After
  using the name and version, you should free() them so as to avoid
  leaks.

  If the compressor is supported, it returns the code for the library
  (>=0).  If it is not supported, this function returns -1.
  */
BLOSC_EXPORT int blosc_get_complib_info(char* compname, char** complib, char** version);


/**
  Free possible memory temporaries and thread resources.  Use this
  when you are not going to use Blosc for a long while.  In case of
  problems releasing the resources, it returns a negative number, else
  it returns 0.
  */
BLOSC_EXPORT int blosc_free_resources(void);


/**
  Return information about a compressed buffer, namely the number of
  uncompressed bytes (`nbytes`) and compressed (`cbytes`).  It also
  returns the `blocksize` (which is used internally for doing the
  compression by blocks).

  You only need to pass the first BLOSC_MIN_HEADER_LENGTH bytes of a
  compressed buffer for this call to work.

  This function should always succeed.
  */
BLOSC_EXPORT void blosc_cbuffer_sizes(const void* cbuffer, size_t* nbytes,
                                      size_t* cbytes, size_t* blocksize);


/**
  Return information about a compressed buffer, namely the type size
  (`typesize`), as well as some internal `flags`.

  The `flags` is a set of bits, where the currently used ones are:
    * bit 0: whether the shuffle filter has been applied or not
    * bit 1: whether the internal buffer is a pure memcpy or not

  You can use the `BLOSC_DOSHUFFLE`, `BLOSC_DOBITSHUFFLE` and
  `BLOSC_MEMCPYED` symbols for extracting the interesting bits
  (e.g. ``flags & BLOSC_DOSHUFFLE`` says whether the buffer is
  byte-shuffled or not).

  This function should always succeed.
  */
BLOSC_EXPORT void blosc_cbuffer_metainfo(const void* cbuffer, size_t* typesize,
                                         int* flags);


/**
  Return information about a compressed buffer, namely the internal
  Blosc format version (`version`) and the format for the internal
  Lempel-Ziv compressor used (`versionlz`).

  This function should always succeed.
  */
BLOSC_EXPORT void blosc_cbuffer_versions(const void* cbuffer, int* version,
                                         int* versionlz);


/**
  Return the compressor library/format used in a compressed buffer.

  This function should always succeed.
  */
BLOSC_EXPORT char* blosc_cbuffer_complib(const void* cbuffer);


/*********************************************************************

  Super-chunk related structures and functions.

*********************************************************************/

#define BLOSC_HEADER_PACKED_LENGTH 96 /* the length of the header for a packed super-chunk */


typedef struct {
  /* struct blosc_context* parent_context; */
  uint8_t version;
  uint8_t flags1;
  uint8_t flags2;
  uint8_t flags3;
  uint16_t compressor;
  /* The default compressor.  Each chunk can override this. */
  uint16_t clevel;
  /* The compression level and other compress params */
  uint16_t filters;
  /* The (sequence of) filters.  3-bit per filter. */
  uint16_t filters_meta;
  /* Metadata for filters */
  uint32_t chunksize;
  /* Size of each chunk.  0 if not a fixed chunksize. */
  int64_t nchunks;
  /* Number of chunks in super-chunk */
  int64_t nbytes;
  /* data size + metadata size + header size (uncompressed) */
  int64_t cbytes;
  /* data size + metadata size + header size (compressed) */
  uint8_t* filters_chunk;
  /* Pointer to chunk hosting filter-related data */
  uint8_t* codec_chunk;
  /* Pointer to chunk hosting codec-related data */
  uint8_t* metadata_chunk;
  /* Pointer to schunk metadata */
  uint8_t* userdata_chunk;
  /* Pointer to user-defined data */
  uint8_t** data;
  /* Pointer to chunk data pointers */
  uint8_t* ctx;
  /* Context for the thread holder.  NULL if not acquired. */
  uint8_t* reserved;
  /* Reserved for the future. */
} schunk_header;

typedef struct {
  uint8_t compressor;
  /* the default compressor */
  uint8_t clevel;
  /* the compression level and other compress params */
  uint8_t filters[BLOSC_MAX_FILTERS];
  /* the (sequence of) filters */
  uint16_t filters_meta;   /* metadata for filters */
} schunk_params;

/* Create a new super-chunk. */
BLOSC_EXPORT schunk_header* blosc2_new_schunk(schunk_params* params);

/* Set a delta reference for the super-chunk */
BLOSC_EXPORT int blosc2_set_delta_ref(schunk_header* sc_header, size_t nbytes, void* ref);

/* Free all memory from a super-chunk. */
BLOSC_EXPORT int blosc2_destroy_schunk(schunk_header* sc_header);

/* Append an existing `chunk` to a super-chunk. */
BLOSC_EXPORT size_t blosc2_append_chunk(schunk_header* sc_header, void* chunk, int copy);

/* Append a `src` data buffer to a super-chunk.

 `typesize` is the number of bytes of the underlying data type and
 `nbytes` is the size of the `src` buffer.

 This returns the number of chunk in super-chunk.  If some problem is
 detected, this number will be negative.
 */
BLOSC_EXPORT size_t blosc2_append_buffer(schunk_header* sc_header,
                                         size_t typesize,
                                         size_t nbytes, void* src);

BLOSC_EXPORT void* blosc2_packed_append_buffer(void* packed, size_t typesize, size_t nbytes, void* src);

/* Decompress and return the `nchunk` chunk of a super-chunk.

 If the chunk is uncompressed successfully, it is put in the `*dest`
 pointer.  `nbytes` is the size of the area pointed by `*dest`.  You
 must make sure that you have space enough to store the uncompressed
 data.

 The size of the decompressed chunk is returned.  If some problem is
 detected, a negative code is returned instead.
 */
BLOSC_EXPORT int blosc2_decompress_chunk(schunk_header* sc_header, int64_t nchunk, void* dest, int nbytes);

BLOSC_EXPORT int blosc2_packed_decompress_chunk(void* packed, int nchunk, void** dest);

/* Pack a super-chunk by using the header. */
BLOSC_EXPORT void* blosc2_pack_schunk(schunk_header* sc_header);

/* Unpack a packed super-chunk */
BLOSC_EXPORT schunk_header* blosc2_unpack_schunk(void* packed);


/*********************************************************************

  Low-level functions follows.  Use them only if you are an expert!

*********************************************************************/


/**
  Force the use of a specific blocksize.  If 0, an automatic
  blocksize will be used (the default).
  */
BLOSC_EXPORT void blosc_set_blocksize(size_t blocksize);

/* Set pointer to super-chunk.  If NULL, no super-chunk will be
   available (the default). */
BLOSC_EXPORT void blosc_set_schunk(schunk_header* schunk);


#ifdef __cplusplus
}
#endif


#endif
