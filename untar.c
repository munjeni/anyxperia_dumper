/* by munjeni @ 2016 */

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
	#ifndef __USE_FILE_OFFSET64
		#define __USE_FILE_OFFSET64 1
	#endif
	#ifndef __USE_LARGEFILE64
		#define __USE_LARGEFILE64 1
	#endif
	#ifndef _LARGEFILE64_SOURCE
		#define _LARGEFILE64_SOURCE 1
	#endif
	#ifndef _FILE_OFFSET_BITS
		#define _FILE_OFFSET_BITS 64
	#endif
	#ifndef _FILE_OFFSET_BIT
		#define _FILE_OFFSET_BIT 64
	#endif
#endif

#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#ifdef HAS_STDINT_H
	#include <stdint.h>
#endif
#if (defined(unix)) || (defined(__APPLE__))
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
#else
	#include <direct.h>
	#include <io.h>
#endif

#if defined(USE_FILE32API)
	#define fopen64 fopen
	#define ftello64 ftell
	#define fseeko64 fseek
#else
	#ifdef __FreeBSD__
		#define fopen64 fopen
		#define ftello64 ftello
		#define fseeko64 fseeko
	#endif
	//#ifdef __ANDROID__
	//	#define fopen64 fopen
	//	#define ftello64 ftello
	//	#define fseeko64 fseeko
	//#endif
	#ifdef _MSC_VER
		#define fopen64 fopen
		#if (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
			#define ftello64 _ftelli64
			#define fseeko64 _fseeki64
		#else  /* old msc */
			#define ftello64 ftell
			#define fseeko64 fseek
		#endif
	#endif
#endif

#if defined(__APPLE__)
#define fopen64 fopen
#define ftello64 ftell
#define fseeko64 fseek
#endif

#ifndef O_BINARY
#define O_BINARY 0x8000
#endif

#define ENABLE_DEBUG 0

#if ENABLE_DEBUG
#define LOG printf
#else
#define LOG(...)
#endif

#include "sparse.h"
#include "lz4.h"

static unsigned short swap_uint16(unsigned short val)
{
	return (val >> 8) | (val << 8);
}

static unsigned long swap_uint32(unsigned long val) {
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF);
	return ((val << 16) | (val >> 16)) & 0xffffffff;
}

static unsigned long long swap_uint64(unsigned long long val) {
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
	return ((val << 32) | (val >> 32)) & 0xffffffffffffffffULL;
}

static char *basenamee(char *in) {
	char *ssc;
	int p = 0;
	ssc = strstr(in, "/");
	if(ssc == NULL) {
	  ssc = strstr(in, "\\");
	  if(ssc == NULL) {
	  	return in;
		}
	}
	do {
	    p = strlen(ssc) + 1;
	    in = &in[strlen(in)-p+2];
	    ssc = strstr(in, "/");
			if (ssc == NULL)
	      ssc = strstr(in, "\\");
	} while(ssc);

	return in;
}

/*
 * "untar" is an extremely simple tar extractor:
 *  * A single C source file, so it should be easy to compile
 *    and run on any system with a C compiler.
 *  * Extremely portable standard C.  The only non-ANSI function
 *    used is mkdir().
 *  * Reads basic ustar tar archives.
 *  * Does not require libarchive or any other special library.
 *
 * To compile: cc -o untar untar.c
 *
 * Usage:  untar <archive>
 *
 * In particular, this program should be sufficient to extract the
 * distribution for libarchive, allowing people to bootstrap
 * libarchive on systems that do not already have a tar program.
 *
 * To unpack libarchive-x.y.z.tar.gz:
 *    * gunzip libarchive-x.y.z.tar.gz
 *    * untar libarchive-x.y.z.tar
 *
 * Written by Tim Kientzle, March 2009.
 *
 * Released into the public domain.
 */

/* These are all highly standard and portable headers. */

#include <assert.h>

#include "zlib.h"

static uint16_t is_big_endian(void) {
	return (*(uint16_t *)"\0\xff" < 0x100) ? 1 : 0;
}

#define CHUNK 16384

/* These are parameters to deflateInit2. See
   http://zlib.net/manual.html for the exact meanings. */

#define windowBits 15
#define GZIP_ENCODING 16

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK)
		return ret;

	/* compress until end of file */
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)deflateEnd(&strm);
			return Z_ERRNO;
		}
		flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;

		/* run deflate() on input until output buffer not full, finish
		  compression if all of source has been read in */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = deflate(&strm, flush);    /* no bad return value */
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */

		/* done when last data in file processed */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END);        /* stream will be complete */

	/* clean up and return */
	(void)deflateEnd(&strm);
	return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE *source, FILE *dest)
{
	int ret, progress=0;
	unsigned long long have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, 47);      /* automatic zlib or gzip decoding */
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;     /* and fall through */
					break;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
				default:
					break;
			}
			have = CHUNK - strm.avail_out;
			if ((have % 4294967296ULL) == 0)
			{
				progress += 1;
				printf(".");
				if (progress == 60) {
					progress = 0;
					printf("\n");
				}
			}
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	printf("\n");
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
	fputs("gzpipe: ", stderr);
	switch (ret)
	{
		case Z_ERRNO:
			if (ferror(stdin))
				fputs("error reading stdin\n", stderr);
			if (ferror(stdout))
				fputs("error writing stdout\n", stderr);
			break;
		case Z_STREAM_ERROR:
			fputs("invalid compression level\n", stderr);
			break;
		case Z_DATA_ERROR:
			fputs("invalid or incomplete deflate data\n", stderr);
			break;
		case Z_MEM_ERROR:
			fputs("out of memory\n", stderr);
			break;
		case Z_VERSION_ERROR:
			fputs("zlib version mismatch!\n", stderr);
			break;
		default:
			fputs("ok.\n", stderr);
			break;
	}
}

static int file_exist(char *file) {
	int ret;
	FILE *f = NULL;

	if ((f = fopen64(file, "rb")) == NULL) {
		ret = 0;
	} else {
		if (f) fclose(f);
		ret = 1;
	}
	return ret;
}

static void remove_file_exist(char *file) {
	if (file_exist(file)) {
		remove(file);
	}
}

int gziper(char *in, char *out)
{
	int ret;
	FILE *source = NULL;
	FILE *zip = NULL;
	FILE *zipped = NULL;
	FILE *back = NULL;

	if ((source = fopen64(in, "rb")) == NULL) {
		printf("Could not open %s!\n", in);
		return 1;
	}
	if ((zip = fopen64(out, "wb")) == NULL) {
		printf("Could not open %s for write!\n", out);
		if (source) fclose(source);
		return 1;
	}

	printf("defflating...\n");
	ret = def(source, zip, Z_DEFAULT_COMPRESSION);
	printf("defflate returned: %i\n", ret);
	if (source) fclose(source);
	if (zip) fclose(zip);

	if (ret == 0) {
		printf ("setting up infflate...\n");
		if ((zipped = fopen64(out, "rb")) == NULL) {
			printf("Could not open %s for verification!\n", out);
			return 1;
		}
		if ((back = fopen64("tempFcheck", "wb")) == NULL) {
			printf("Could not open for write temFcheck file for verification!\n");
			if (zipped) fclose(zipped);
			return 1;
		}
		printf ("infflating...\n");
		ret = inf(zipped, back);
		printf("infflate returned: %i\n", ret);
		zerr(ret);
	}

	if (zipped) fclose(zipped);
	if (back) fclose(back);

	if (ret != 0) {
		remove_file_exist(out);
		return 1;
	}

	remove_file_exist("tempFcheck");
	printf("gzip: ok.\n");
	return 0;
}

int gunziper(char *in, char *out)
{
	int ret;
	FILE *zipped = NULL;
	FILE *back = NULL;

	printf ("setting up infflate...\n");
	if ((zipped = fopen64(in, "rb")) == NULL) {
		printf("Could not open %s for infflating!\n", in);
		return 1;
	}
	if ((back = fopen64(out, "wb")) == NULL) {
		printf("Could not open %s for write!\n", out);
		if (zipped) fclose(zipped);
		return 1;
	}
	printf ("infflating...\n");
	ret = inf(zipped, back);
	printf("infflate returned: %i\n", ret);
	zerr(ret);

	if (zipped) fclose(zipped);
	if (back) fclose(back);

	if (ret != 0) {
		remove_file_exist(out);
		return 1;
	}

	printf("unziped: ok.\n");
	return 0;
}

/* Parse an octal number, ignoring leading and trailing nonsense. */
static int
parseoct(const char *p, size_t n)
{
	int i = 0;

	while (*p < '0' || *p > '7') {
		++p;
		--n;
	}

	while (*p >= '0' && *p <= '7' && n > 0)
	{
		i *= 8;
		i += *p - '0';
		++p;
		--n;
	}

	return (i);
}

/* Returns true if this is 512 zero bytes. */
static int
is_end_of_archive(const char *p)
{
	int n;
	for (n = 511; n >= 0; --n)
		if (p[n] != '\0')
			return (0);
	return (1);
}

/* Create a file, including parent directory as necessary. */
static FILE *
create_file(char *pathname, int mode)
{
	FILE *f = fopen64(pathname, "wb");
	return (f);
}

/* Verify the tar checksum. */
static int
verify_checksum(const char *p)
{
	int n, u = 0;
	for (n = 0; n < 512; ++n) {
		if (n < 148 || n > 155)
			/* Standard tar checksum adds unsigned bytes. */
			u += ((unsigned char *)p)[n];
		else
			u += 0x20;

	}
	return (u == parseoct(p + 148, 8));
}

/* Extract a tar archive. */
void
untar(FILE *a, const char *path, char *outfolder)
{
	char buff[512];
	FILE *f = NULL;
	unsigned long bytes_read;
	unsigned long bytes_write;
	unsigned long filesize;
	unsigned int i=0;
	unsigned short sparse=0;
	char tmpp[256];
	char tmpg[256];
	unsigned short file_type = 1;
	unsigned short is_be = is_big_endian();

	printf("Extracting from %s\n", path);

	for (;;)
	{
		unsigned int chunk = 0;

		bytes_read = fread(buff, 1, 512, a);
		if (bytes_read < 512) {
			printf("Short read on %s: expected 512, got 0x%lx\n", path, bytes_read);
			return;
		}

		if (is_end_of_archive(buff))
		{
			printf("End of %s\n", path);

			if (sparse)
			{
				unsigned int j;
				FILE *in = NULL;
				FILE *out = NULL;

				unsigned long long ext4_file_size = 0;
				unsigned short searched = 0;

				memset(tmpg, 0, sizeof(tmpg));
				memcpy(tmpg, path, strlen(path)-4);
				snprintf(tmpp, sizeof(tmpp), "%s/%s.bin", outfolder, basenamee(tmpg));
				out = fopen64(tmpp, "wb");
				if (out == NULL) {
					printf("Cannot open output file %s\n", tmpp);
					fclose(f);
					exit(-1);
				}

				printf("found image with total %d chunks.\n", i-1);

				for (j=0; j<i-1; ++j)
				{
					struct SparseHeader sparseHeader;
					unsigned int si = 0;

					fseeko64(out, 0, SEEK_SET);

					memset(tmpg, 0, sizeof(tmpg));
					memcpy(tmpg, path, strlen(path)-4);
					snprintf(tmpp, sizeof(tmpp), "%s/%s.%03d", outfolder, basenamee(tmpg), j);

					in = fopen64(tmpp, "rb");
					if (in == NULL) {
						printf("Cannot open input file %s\n", tmpp);
						fclose(f);
						fclose(out);
						exit(-1);
					}

					printf("Processing %s\n", tmpp);

					bytes_read = fread(&sparseHeader, 1, sizeof(struct SparseHeader), in);
					if (bytes_read < sizeof(struct SparseHeader)) {
						printf("Short read SparseHeader: Expected 0x%lx, got 0x%lx\n", (unsigned long)sizeof(struct SparseHeader), bytes_read);
						fclose(f);
						fclose(in);
						fclose(out);
						return;
					}

					if (is_be) {
						sparseHeader.Magic = swap_uint32(sparseHeader.Magic);
						sparseHeader.MajorVersion = swap_uint16(sparseHeader.MajorVersion);
						sparseHeader.MinorVersion = swap_uint16(sparseHeader.MinorVersion);
						sparseHeader.FileHeaderSize = swap_uint16(sparseHeader.FileHeaderSize);
						sparseHeader.ChunkHeaderSize = swap_uint16(sparseHeader.ChunkHeaderSize);
						sparseHeader.BlockSize = swap_uint32(sparseHeader.BlockSize);
						sparseHeader.TotalBlocks = swap_uint32(sparseHeader.TotalBlocks);
						sparseHeader.TotalChunks = swap_uint32(sparseHeader.TotalChunks);
						sparseHeader.ImageChecksum = swap_uint32(sparseHeader.ImageChecksum);
					}

					if (sparseHeader.Magic != SPARSE_HEADER_MAGIC)
					{
						printf("Error, sparseHeader.Magic != 0xED26FF3A, got 0x%08x\n", sparseHeader.Magic);
						fclose(f);
						fclose(in);
						fclose(out);
						return;
					}

					if (sparseHeader.FileHeaderSize != 28)
					{
						printf("Error, sparseHeader.FileHeaderSize != 28, got %d\n", sparseHeader.FileHeaderSize);
						fclose(f);
						fclose(in);
						fclose(out);
						return;
					}

					if (sparseHeader.ChunkHeaderSize != 12)
					{
						printf("Error, sparseHeader.ChunkHeaderSize != 12, got %d\n", sparseHeader.ChunkHeaderSize);
						fclose(f);
						fclose(in);
						fclose(out);
						return;
					}

					if (sparseHeader.TotalChunks == 0)
					{
						printf("Error, sparseHeader.TotalChunks == 0!\n");
						fclose(f);
						fclose(in);
						fclose(out);
						return;
					}

					LOG("Sparse magic: 0x%08X\n", sparseHeader.Magic);
					LOG("Sparse MajorVersion: 0x%04X\n", sparseHeader.MajorVersion);
					LOG("Sparse MinorVersion: 0x%04X\n", sparseHeader.MinorVersion);
					LOG("Sparse FileHeaderSize: 0x%04X\n", sparseHeader.FileHeaderSize);
					LOG("Sparse ChunkHeaderSize: 0x%04X\n", sparseHeader.ChunkHeaderSize);
					LOG("Sparse BlockSize: 0x%08X\n", sparseHeader.BlockSize);
					LOG("Sparse TotalBlocks: 0x%08X\n", sparseHeader.TotalBlocks);
					LOG("Sparse TotalChunks: 0x%08X\n", sparseHeader.TotalChunks);
					LOG("Sparse ImageChecksum: 0x%08X\n", sparseHeader.ImageChecksum);

					while (si < sparseHeader.TotalChunks)
					{
						struct ChunkHeader chunkHeader;
						unsigned long long sj = 0LL;
						unsigned int sk = 0;
						unsigned long sparse_data_in_sz = 0L;
						unsigned long long sparse_data_out_sz = 0LL;
						char *tmp_buff = NULL;
						char *lz4_tmp_buff = NULL;
						int lz4_return_value;
						unsigned long long curr_out_poss = ftello64(out);

						bytes_read = fread(&chunkHeader, 1, sizeof(struct ChunkHeader), in);
						if (bytes_read < sizeof(struct ChunkHeader)) {
							printf("Short read ChunkHeader: Expected 0x%lx, got 0x%lx\n", (unsigned long)sizeof(struct ChunkHeader), bytes_read);
							fclose(f);
							fclose(in);
							fclose(out);
							return;
						}

						if (is_be) {
							chunkHeader.ChunkType = swap_uint16(chunkHeader.ChunkType);
							//chunkHeader.Reserved = swap_uint16(chunkHeader.Reserved);
							chunkHeader.ChunkBlocks = swap_uint32(chunkHeader.ChunkBlocks);
							chunkHeader.ChunkSize = swap_uint32(chunkHeader.ChunkSize);
						}

						sparse_data_in_sz = chunkHeader.ChunkSize - sparseHeader.ChunkHeaderSize;
						sparse_data_out_sz = (chunkHeader.ChunkBlocks + 0LL) * (sparseHeader.BlockSize + 0LL);

						LOG("ChunkHeader ChunkType: 0x%04X\n", chunkHeader.ChunkType);
						LOG("ChunkHeader Reserved: 0x%04X\n", chunkHeader.Reserved);
						LOG("ChunkHeader ChunkBlocks: 0x%08X\n", chunkHeader.ChunkBlocks);
						LOG("ChunkHeader ChunkSize: 0x%08X\n", chunkHeader.ChunkSize);

						switch (chunkHeader.ChunkType)
						{
							case CHUNK_TYPE_RAW:
								LOG("RAW NOW=%llX\n", curr_out_poss);
								for (sk = 0; sk < chunkHeader.ChunkBlocks; ++sk)
								{
									if ((tmp_buff = (char *)malloc(sparseHeader.BlockSize + 1)) == NULL) {
										printf("Error in CHUNK_TYPE_RAW, error allocating memory of the 0x%x bytes for temp_buff!\n", sparseHeader.BlockSize);
										fclose(f);
										fclose(in);
										fclose(out);
										return;
									}

									bytes_read = fread(tmp_buff, 1, sparseHeader.BlockSize, in);
									if ((unsigned int)bytes_read < sparseHeader.BlockSize) {
										printf("Error in CHUNK_TYPE_RAW, error copying 0x%x bytes to temp_buff, done 0x%lx!\n", sparseHeader.BlockSize, bytes_read);
										fclose(f);
										fclose(in);
										fclose(out);
										free(tmp_buff);
										return;
									}

									LOG("RAW=%llX\n", curr_out_poss + sparseHeader.BlockSize);
									bytes_write = fwrite(tmp_buff, 1, sparseHeader.BlockSize, out);
									if ((unsigned int)bytes_write < sparseHeader.BlockSize) {
										printf("Error in CHUNK_TYPE_RAW, error writing 0x%x bytes from temp_buff, done 0x%lx!\n", sparseHeader.BlockSize, bytes_write);
										fclose(f);
										fclose(in);
										fclose(out);
										free(tmp_buff);
										return;
									}
									curr_out_poss += sparseHeader.BlockSize;

									if (j == 0 && !searched)
									{
										unsigned int gg;
										for (gg=0; gg<sparseHeader.BlockSize - 4; ++gg)
										{
											if (memcmp(tmp_buff+gg, "\x7f\x45\x4c\x46", 4) == 0 && gg == 0) {
												file_type = 2; /* ELF */
												printf("Filetype ELF.\n");
												break;
											}
											else if (memcmp(tmp_buff+gg, "\x53\xef", 2) == 0) {
												file_type = 3;  /* EXT4 */
												ext4_file_size = *(unsigned long long *)&tmp_buff[gg-52] * sparseHeader.BlockSize;
												if (is_be)
													ext4_file_size = swap_uint64(ext4_file_size);
												printf("Found ext4 magic. Ext4 size: 0x%llx\n", ext4_file_size);
												break;
											}
											else if (memcmp(tmp_buff+gg, "\xeb\x3c\x90\x4d", 4) == 0 && gg == 0) {
												file_type = 4; /* MSDOS VFAT */
												printf("Filetype VFAT.\n");
												break;
											}
											else if (memcmp(tmp_buff+gg, "\x41\x4e\x44\x52", 4) == 0 && gg == 0) {
												file_type = 5; /* ANDROID IMG */
												printf("Filetype IMG.\n");
												break;
											}
										}
										searched = 1;
									}

									free(tmp_buff);
								}
								LOG("RAW END=%llX\n\n", curr_out_poss);
								break;

							case CHUNK_TYPE_FILL:
								LOG("FILL NOW=%llX\n", curr_out_poss);
								if ((tmp_buff = (char *)malloc(sparse_data_in_sz + 1)) == NULL) {
									printf("Error in CHUNK_TYPE_FILL, error allocating memory of the 0x%lx bytes for temp_buff!\n", sparse_data_in_sz);
									fclose(f);
									fclose(in);
									fclose(out);
									return;
								}

								bytes_read = fread(tmp_buff, 1, sparse_data_in_sz, in);
								if (bytes_read < sparse_data_in_sz) {
									printf("Error in CHUNK_TYPE_FILL, error copying 0x%lx bytes to temp_buff, done 0x%lx!\n", sparse_data_in_sz, bytes_read);
									fclose(f);
									fclose(in);
									fclose(out);
									free(tmp_buff);
									return;
								}

								for (sj = 0; sj < sparse_data_out_sz; sj += sparse_data_in_sz)
								{
									LOG("FILL=%llX\n", curr_out_poss + sparse_data_in_sz);
									bytes_write = fwrite(tmp_buff, 1, sparse_data_in_sz, out);
									if (bytes_write < sparse_data_in_sz) {
										printf("Error in CHUNK_TYPE_FILL, error writing 0x%lx bytes from temp_buff, done 0x%lx!\n", sparse_data_in_sz, bytes_write);
										fclose(f);
										fclose(in);
										fclose(out);
										free(tmp_buff);
										return;
									}
									curr_out_poss += sparse_data_in_sz;
								}

								free(tmp_buff);
								LOG("FILL END=%llX\n\n", curr_out_poss);
								break;

							case CHUNK_TYPE_DONT_CARE:
								LOG("DONTCARE NOW=%llX\n", curr_out_poss);
								curr_out_poss += sparse_data_out_sz;
								fseeko64(out, curr_out_poss, SEEK_SET);
								LOG("DONTCARE END=%llX\n\n", curr_out_poss);
								break;

							case CHUNK_TYPE_CRC32:
								LOG("CRC32\n\n");
								fseeko64(in, sparse_data_in_sz, SEEK_CUR);
								break;

							case CHUNK_TYPE_LZ4:
								LOG("LZ4 NOW=%llX\n", curr_out_poss);
								if ((tmp_buff = (char *)malloc(sparse_data_in_sz + 1)) == NULL) {
									printf("Error in CHUNK_TYPE_LZ4, error allocating memory of the 0x%lx bytes for temp_buff!\n", sparse_data_in_sz);
									fclose(f);
									fclose(in);
									fclose(out);
									return;
								}

								if ((lz4_tmp_buff = (char *)malloc((unsigned long)sparse_data_out_sz + 1)) == NULL) {
									printf("Error in CHUNK_TYPE_LZ4, error allocating memory of the 0x%lx bytes for lz4_temp_buff!\n", (unsigned long)sparse_data_out_sz);
									fclose(f);
									fclose(in);
									fclose(out);
									free(tmp_buff);
									return;
								}

								bytes_read = fread(tmp_buff, 1, sparse_data_in_sz, in);
								if (bytes_read < sparse_data_in_sz) {
									printf("Error in CHUNK_TYPE_LZ4, error copying 0x%lx bytes to temp_buff, done 0x%lx!\n", sparse_data_in_sz, bytes_read);
									fclose(f);
									fclose(in);
									fclose(out);
									free(tmp_buff);
									free(lz4_tmp_buff);
									return;
								}

								lz4_return_value = LZ4_decompress_safe(tmp_buff, lz4_tmp_buff, sparse_data_in_sz, (unsigned long)sparse_data_out_sz);

								if (lz4_return_value < 0)
									printf("A negative result from LZ4_decompress_fast indicates a failure trying to decompress the data.  See exit code (echo $?) for value returned!\n");
								if (lz4_return_value == 0)
									printf("I'm not sure this function can ever return 0.  Documentation in lz4.h doesn't indicate so!\n");

								if (lz4_return_value <= 0) {
									printf("Error decompessing lz4 chunk!\n");
									fclose(f);
									fclose(in);
									fclose(out);
									free(tmp_buff);
									free(lz4_tmp_buff);
									return;
								}

								LOG("LZ4=%llX\n", curr_out_poss + sparse_data_out_sz);
								bytes_write = fwrite(lz4_tmp_buff, 1, (unsigned long)sparse_data_out_sz, out);
								if (bytes_write < (unsigned long)sparse_data_out_sz) {
									printf("Error in CHUNK_TYPE_LZ4, error writing 0x%lx bytes from lz4_temp_buff, done 0x%lx!\n", (unsigned long)sparse_data_out_sz, bytes_write);
									fclose(f);
									fclose(in);
									fclose(out);
									free(tmp_buff);
									free(lz4_tmp_buff);
									return;
								}
								curr_out_poss += sparse_data_out_sz;

								if (j == 0 && !searched)
								{
									unsigned long gg;
									for (gg=0; gg < (unsigned long)sparse_data_out_sz - 4; ++gg)
									{
										if (memcmp(lz4_tmp_buff+gg, "\x7f\x45\x4c\x46", 4) == 0 && gg == 0) {
											file_type = 2; /* ELF */
											printf("Filetype ELF.\n");
											break;
										}
										else if (memcmp(lz4_tmp_buff+gg, "\x53\xef", 2) == 0) {
											file_type = 3;  /* EXT4 */
											ext4_file_size = *(unsigned long long *)&lz4_tmp_buff[gg-52] * sparseHeader.BlockSize;
											if (is_be)
												ext4_file_size = swap_uint64(ext4_file_size);
											printf("Found ext4 magic. Ext4 size: 0x%llx\n", ext4_file_size);
											break;
										}
										else if (memcmp(lz4_tmp_buff+gg, "\xeb\x3c\x90\x4d", 4) == 0 && gg == 0) {
											file_type = 4; /* MSDOS VFAT */
											printf("Filetype VFAT.\n");
											break;
										}
										else if (memcmp(lz4_tmp_buff+gg, "\x41\x4e\x44\x52", 4) == 0 && gg == 0) {
											file_type = 5; /* ANDROID IMG */
											printf("Filetype IMG.\n");
											break;
										}
									}
									searched = 1;
								}

								free(tmp_buff);
								free(lz4_tmp_buff);
								LOG("LZ4 END=%llX\n\n", curr_out_poss);
								break;

							default:
								printf("Error, unknown chunk type 0x%x !\n\n", chunkHeader.ChunkType);
								fclose(f);
								fclose(in);
								fclose(out);
								return;
						}
						si += 1;
					}

					fclose(in);
					remove(tmpp);
				}

				memset(tmpg, 0, sizeof(tmpg));
				memcpy(tmpg, path, strlen(path)-4);

				snprintf(tmpp, sizeof(tmpp), "%s/%s.bin", outfolder, basenamee(tmpg));

				if (ext4_file_size)
				{
					if (ftello64(out) <= ext4_file_size)
					{
						char nn[1];
						memset(nn, 0, sizeof(nn));
						fseeko64(out, ext4_file_size - 1, SEEK_SET);
						bytes_read = fread(nn, 1, 1, out);
						fseeko64(out, ext4_file_size - 1, SEEK_SET);
						fwrite(nn, 1, 1, out);
					}
				}

				fclose(out);

				switch (file_type)
				{
					case 2:
						snprintf(tmpp, sizeof(tmpp), "%s/%s.elf", outfolder, basenamee(tmpg));
						memcpy(tmpg, tmpp, sizeof(tmpp));
						tmpg[strlen(tmpp)-3] = 'b';
						tmpg[strlen(tmpp)-2] = 'i';
						tmpg[strlen(tmpp)-1] = 'n';
						remove(tmpp);
						rename(tmpg, tmpp);
						break;

					case 3:
						snprintf(tmpp, sizeof(tmpp), "%s/%s.ext4", outfolder, basenamee(tmpg));
						memcpy(tmpg, tmpp, sizeof(tmpp));
						tmpg[strlen(tmpp)-4] = 'b';
						tmpg[strlen(tmpp)-3] = 'i';
						tmpg[strlen(tmpp)-2] = 'n';
						tmpg[strlen(tmpp)-1] = '\0';
						remove(tmpp);
						rename(tmpg, tmpp);
						break;

					case 4:
						snprintf(tmpp, sizeof(tmpp), "%s/%s.vfat", outfolder, basenamee(tmpg));
						memcpy(tmpg, tmpp, sizeof(tmpp));
						tmpg[strlen(tmpp)-4] = 'b';
						tmpg[strlen(tmpp)-3] = 'i';
						tmpg[strlen(tmpp)-2] = 'n';
						tmpg[strlen(tmpp)-1] = '\0';
						remove(tmpp);
						rename(tmpg, tmpp);
						break;

					case 5:
						snprintf(tmpp, sizeof(tmpp), "%s/%s.img", outfolder, basenamee(tmpg));
						memcpy(tmpg, tmpp, sizeof(tmpp));
						tmpg[strlen(tmpp)-3] = 'b';
						tmpg[strlen(tmpp)-2] = 'i';
						tmpg[strlen(tmpp)-1] = 'n';
						remove(tmpp);
						rename(tmpg, tmpp);
						break;

					default:
						break;
				}

				printf("%s created.\n", tmpp);
			}

			return;
		}

		if (!verify_checksum(buff)) {
			printf("Checksum failure\n");
			return;
		}

		filesize = parseoct(buff + 124, 12);

		switch (buff[156])
		{
			case '1':
				printf("Ignoring hardlink %s\n", buff);
				break;
			case '2':
				printf("Ignoring symlink %s\n", buff);
				break;
			case '3':
				printf("Ignoring character device %s\n", buff);
					break;
			case '4':
				printf("Ignoring block device %s\n", buff);
				break;
			case '5':
				printf("Ignoring dir %s\n", buff);
				filesize = 0;
				break;
			case '6':
				printf("Ignoring FIFO %s\n", buff);
				break;
			default:
				if (i == 0) {
					memset(tmpg, 0, sizeof(tmpg));
					memcpy(tmpg, path, strlen(path)-4);
					snprintf(tmpp, sizeof(tmpp), "%s/%s%s", outfolder, basenamee(tmpg), ".crt");
				} else {
					memset(tmpg, 0, sizeof(tmpg));
					memcpy(tmpg, path, strlen(path)-4);
					snprintf(tmpp, sizeof(tmpp), "%s/%s.%03d", outfolder, basenamee(tmpg), i-1);
				}
				printf("Extracting file %s\n", tmpp);
				i += 1;
				f = create_file(tmpp, parseoct(buff + 100, 8));
				break;
		}

		while (filesize > 0)
		{
			bytes_read = fread(buff, 1, 512, a);
			if (bytes_read < 512) {
				printf("Short read on %s: Expected 512, got %d\n", path, (int)bytes_read);
				return;
			}

			if (chunk == 0) {
				if (memcmp(buff, "\x3a\xff\x26\xed", 4) == 0) {
					sparse = 1;
				}
			}

			if (filesize < 512)
				bytes_read = filesize;

			if (f != NULL)
			{
				if (fwrite(buff, 1, bytes_read, f) != bytes_read)
				{
					printf("Failed write\n");
					fclose(f);
					f = NULL;
				}
			}

			filesize -= bytes_read;
			chunk += 1;
		}

		if (f != NULL) {
			fclose(f);
			f = NULL;
		}
	}
}

/*
int
main(int argc, char *argv[])
{
	FILE *a;

	if (gunziper(argv[1], "fileout.tar"))
		return 1;

	a = fopen64("fileout.tar", "rb");
	if (a == NULL)
		printf("Unable to open %s\n", *argv);
	else {
		untar(a, argv[1]);
		fclose(a);
	}

	remove("fileout.tar");
	return (0);
}
*/

