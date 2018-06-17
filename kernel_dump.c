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

#undef basename
#define basename basenamee

#include "lz4.h"
#include "zlib.h"

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

//#define EXTERNAL_KERNEL_DUMP

#define ELFMAG0		0x7f	/* Magic number byte 0 */
#define ELFMAG1		'E'	/* Magic number byte 1 */
#define ELFMAG2		'L'	/* Magic number byte 2 */
#define ELFMAG3		'F'	/* Magic number byte 3 */

struct Elf32_Phdr {
	unsigned int	p_type;			/* Segment type */
	unsigned int	p_offset;		/* Segment file offset */
	unsigned int	p_vaddr;		/* Segment virtual address */
	unsigned int	p_paddr;		/* Segment physical address */
	unsigned int	p_filesz;		/* Segment size in file */
	unsigned int	p_memsz;		/* Segment size in memory */
	unsigned int	p_flags;		/* Segment flags */
	unsigned int	p_align;		/* Segment alignment */
};

struct Elf64_Phdr {
	unsigned int		p_type;		/* Segment type */
	unsigned int		p_flags;	/* Segment flags */
	unsigned long long	p_offset;	/* Segment file offset */
	unsigned long long	p_vaddr;	/* Segment virtual address */
	unsigned long long	p_paddr;	/* Segment physical address */
	unsigned long long	p_filesz;	/* Segment size in file */
	unsigned long long	p_memsz;	/* Segment size in memory */
	unsigned long long	p_align;	/* Segment alignment */
};

struct Elf32_Ehdr {
	unsigned char	e_ident[16];		/* Magic number and other info */
	unsigned short	e_type;			/* Object file type */
	unsigned short	e_machine;		/* Architecture */
	unsigned int	e_version;		/* Object file version */
	unsigned int	e_entry;	  	/* Entry point virtual address */
	unsigned int	e_phoff;		/* Program header table file offset */
	unsigned int	e_shoff;		/* Section header table file offset */
	unsigned int	e_flags;		/* Processor-specific flags */
	unsigned short	e_ehsize;		/* ELF header size in bytes */
	unsigned short	e_phentsize;		/* Program header table entry size */
	unsigned short	e_phnum;		/* Program header table entry count */
	unsigned short	e_shentsize;		/* Section header table entry size */
	unsigned short	e_shnum;		/* Section header table entry count */
	unsigned short	e_shstrndx;		/* Section header string table index */
};

struct Elf64_Ehdr {
	unsigned char		e_ident[16];	/* Magic number and other info */
	unsigned short		e_type;		/* Object file type */
	unsigned short		e_machine;	/* Architecture */
	unsigned int		e_version;	/* Object file version */
	unsigned long long	e_entry;	/* Entry point virtual address */
	unsigned long long	e_phoff;	/* Program header table file offset */
	unsigned long long	e_shoff;	/* Section header table file offset */
	unsigned int		e_flags;	/* Processor-specific flags */
	unsigned short		e_ehsize;	/* ELF header size in bytes */
	unsigned short		e_phentsize;	/* Program header table entry size */
	unsigned short		e_phnum;	/* Program header table entry count */
	unsigned short		e_shentsize;	/* Section header table entry size */
	unsigned short		e_shnum;	/* Section header table entry count */
	unsigned short		e_shstrndx;	/* Section header string table index */
};

extern int gunziper(char *in, char *out);
extern void untar(FILE *a, const char *path, char *outfolder);

unsigned long swap_uint32(unsigned long val) {
	val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF);
	return ((val << 16) | (val >> 16)) & 0xffffffff;
}

unsigned long long swap_uint64(unsigned long long val) {
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
	return ((val << 32) | (val >> 32)) & 0xffffffffffffffffULL;
}

void fread_unus_res(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t in;
	in = fread(ptr, size, nmemb, stream);
	if (in) {
		/* satisfy warn unused result */
	}
}

static int fflushall(void)
{
#ifdef _WIN32
	_flushall();
#endif
	return 0;
}

unsigned long long file_size(char *filename) {
	unsigned long long size;
	FILE *fp = fopen64(filename, "rb");

	if (fp == NULL) {
		return 0;
	}

	fseeko64(fp, 0, SEEK_END);
	size = ftello64(fp);
	fseeko64(fp, 0, SEEK_SET);
	fclose(fp);

	return size;
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

struct MMCF_Header {
	unsigned char mmcf_magic[4];
	unsigned int gptp_len;
};

struct MMCF_Data {
	unsigned char gptp_magic[4];
	unsigned int gptp_size;
	unsigned char gptp_uuid[16];
};

int convert_sin_to_elf(char *filein, char *fileout)
{
	struct MMCF_Header mmcf_h;
	struct MMCF_Data mmcf_d;
	FILE *sin = NULL;
	FILE *elf = NULL;
	size_t sin_sz;

	char sin_header[1024];
	char out[2];
	unsigned long i;
	unsigned long sin_header_sz = 0;
	unsigned long long sin_data_sz;
	unsigned long sin_data_first_sz = 0;
	unsigned long t;
	int non_loader = 0;
	int parts = 0;
	int file_type = 0;

	unsigned int sin_version = 0;

	unsigned long long revert_sin_poss = 0;
	unsigned long long start_sin_poss = 0;
	unsigned long long now_sin_poss = 0;

	unsigned long long ext4_file_size = 0;
	unsigned long long out_next_poss = 0;

	unsigned long long packed_data_offset;
	unsigned long long unpacked_data_size;
	unsigned long long packed_data_size;
	unsigned long long unpacked_data_offset;

	unsigned long data_type = 0;
	unsigned long data_type_header_size = 0;

	char *temp_buff = NULL;
	char *new_src = NULL;

	int return_value;

	memset(sin_header, '\0', 1024);

	if ((sin = fopen64(filein, "rb")) == 0) {
		printf("Error: unable to open %s!\n", filein);
		return 1;
	}

	fseeko64(sin, 0, SEEK_END);
	sin_sz = ftello64(sin);
	fseeko64(sin, 0, SEEK_SET);

	if (sin_sz < 1024) {
		printf("Error: is %s valid?\n", filein);
		if (sin) fclose(sin);
		return 1;
	}

	fread_unus_res(sin_header, 1024, 1, sin);

	if (memcmp(sin_header, "\x01", 1) == 0)
		sin_version = 1;

	if (memcmp(sin_header, "\x02", 1) == 0)
		sin_version = 2;

	if (memcmp(sin_header, "\x03", 1) == 0)
		sin_version = 3;

	printf("Proccessing %s...\n", filein);

	if (sin_version > 3 || sin_version == 0) {
		printf("Error: found unsupported sin version: %d!!!\n", sin_version);
		if (sin) fclose(sin);
		return 1;
	}

	if (sin_version == 3) {
		if (memcmp(sin_header, "\x03\x53\x49\x4E", 4) != 0) {
			printf("Error: file is not a sin!?\n");
			if (sin) fclose(sin);
			return 1;
		}
	}

	printf("Sin version: %d\n", sin_version);
	printf("File size: 0x%lX\n", (unsigned long)sin_sz);

	if (sin_version < 3) {
		memcpy(&sin_header_sz, sin_header+2, 4);
		sin_header_sz = swap_uint32(sin_header_sz);

		sin_data_sz = sin_sz - (sin_header_sz + 16);
	}
	else {
		memcpy(&sin_header_sz, sin_header+4, 4);
		sin_header_sz = swap_uint32(sin_header_sz);

		memcpy(&sin_data_sz, sin_header+0x18, 4);
		sin_data_sz = swap_uint32(sin_data_sz);
	}

	printf("Sin header size: 0x%lX\n", sin_header_sz);
	printf("Sin data size: 0x%llX\n", sin_data_sz);

	if (sin_header_sz < 1 || sin_data_sz < 1) {
		printf("Error: sin header size or sin data size is 0!\n");
		if (sin) fclose(sin);
		return 1;
	}

	if (sin_version < 3)
		sin_header_sz += 16;

	fseeko64(sin, sin_header_sz, SEEK_SET);

	fread_unus_res(&mmcf_h, 1, sizeof(struct MMCF_Header), sin);

	if (memcmp(mmcf_h.mmcf_magic, "MMCF", 4) == 0)
	{
		printf("Found MMCF magic string!\n");
		non_loader = 1;
		mmcf_h.gptp_len = swap_uint32(mmcf_h.gptp_len);
		if (mmcf_h.gptp_len)
		{
			printf("GPTP len: 0x%x\n", mmcf_h.gptp_len);

			fread_unus_res(&mmcf_d, 1, sizeof(struct MMCF_Data), sin);

			printf("GPTP_UUID: ");
			for (t=0; t<16; ++t) {
				printf("%02x", mmcf_d.gptp_uuid[t] & 0xff);
				if (t==3 || t==5 || t==7 || t==9) printf("-");
			}
			printf("\n");

			if ((elf = fopen64(fileout, "wb")) == NULL) {
				printf("Error, cant open %s for write!\n", fileout);
				if (sin) fclose(sin);
				return 1;
			}

			t = 0;
			while (t < (mmcf_h.gptp_len - sizeof(struct MMCF_Data)))
			{
				fread_unus_res(&data_type, 1, 4, sin);
				switch (data_type)
				{
					case 0x41345A4C:  /* LZ4A */
						parts += 1;
						printf("Extracting part %d type: LZ4A\n", parts);
						revert_sin_poss = ftello64(sin) - 4;
						fread_unus_res(&data_type_header_size, 1, 4, sin);
						fread_unus_res(&packed_data_offset, 1, 8, sin);
						fread_unus_res(&unpacked_data_size, 1, 8, sin);
						fread_unus_res(&packed_data_size, 1, 8, sin);
						fread_unus_res(&unpacked_data_offset, 1, 8, sin);
						data_type_header_size = swap_uint32(data_type_header_size);
						packed_data_offset = swap_uint64(packed_data_offset);
						unpacked_data_size = swap_uint64(unpacked_data_size);
						packed_data_size = swap_uint64(packed_data_size);
						unpacked_data_offset = swap_uint64(unpacked_data_offset);
						printf("|-- packed data offset: 0x%llx\n", packed_data_offset);
						printf("|-- unpacked data size: 0x%llx\n", unpacked_data_size);
						printf("|-- packed data size: 0x%llx\n", packed_data_size);
						printf("|-- unpacked data offset: 0x%llx\n", unpacked_data_offset);

						if (out_next_poss < unpacked_data_offset)
						{
							printf("|------ fill 0xFF: 0x%llx\n", unpacked_data_offset - out_next_poss);

							if ((temp_buff = (char *)malloc(unpacked_data_offset - out_next_poss)) == NULL) {
								printf("Failed to allocate 0x%llx bytes of the memory for FF!\n", unpacked_data_offset - out_next_poss);
								if (elf) fclose(elf);
								if (sin) fclose(sin);
								return 1;
							}

							memset(temp_buff, 0xff, unpacked_data_offset - out_next_poss);
							fwrite(temp_buff, 1, unpacked_data_offset - out_next_poss, elf);
							printf("|------ poss: 0x%llx expected: 0x%llx\n",
								 (unsigned long long)ftello64(elf),
								 (unsigned long long)unpacked_data_offset);

							free(temp_buff);

							if ((unsigned long long)ftello64(elf) != (unsigned long long)unpacked_data_offset) {
								printf("error, position didn't match expected! Please check free space on partition!\n");
								if (elf) fclose(elf);
								if (sin) fclose(sin);
								return 1;
							}
						}

						if ((new_src = (char *)malloc(unpacked_data_size)) == NULL) {
							printf("Failed to allocate memory of the 0x%llx bytes for *new_src!\n", unpacked_data_size);
							if (elf) fclose(elf);
							if (sin) fclose(sin);
							return 1;
						}

						if ((temp_buff = (char *)malloc(packed_data_size)) == NULL) {
							printf("Failed to allocate 0x%llx bytes of the memory for *temp_buff for lz4 ex!\n", packed_data_size);
							if (elf) fclose(elf);
							if (sin) fclose(sin);
							free(new_src);
							return 1;
						}

						start_sin_poss = sin_header_sz;
						start_sin_poss += mmcf_h.gptp_len;
						start_sin_poss += sizeof(struct MMCF_Header);
						start_sin_poss += packed_data_offset;
						printf("|------ curr sin poss: 0x%llx\n", revert_sin_poss);
						printf("|------ start sin poss: 0x%llx\n", start_sin_poss);

						fseeko64(sin, start_sin_poss, SEEK_SET);

						fread_unus_res(temp_buff, 1, packed_data_size, sin);

						return_value = LZ4_decompress_safe(
									(const char *)temp_buff,
									 new_src,
									 packed_data_size,
									 unpacked_data_size
						);

						if (return_value < 0)
							printf("A negative result from LZ4_decompress_fast indicates a failure trying to decompress the data.  See exit code (echo $?) for value returned!\n");
						if (return_value == 0)
							printf("I'm not sure this function can ever return 0.  Documentation in lz4.h doesn't indicate so!\n");

						if (return_value <= 0) {
							free(new_src);
							free(temp_buff);
							if (elf) fclose(elf);
							if (sin) fclose(sin);
							return 1;
						}

						now_sin_poss = ftello64(sin);
						printf("|------ now sin poss: 0x%llx\n", now_sin_poss);
						printf("|------ now out poss: 0x%llx\n", (unsigned long long)ftello64(elf));

						printf("|------ successfully decompressed part %d data.\n", parts);
						fwrite(new_src, 1, unpacked_data_size, elf);
						out_next_poss = ftello64(elf);

						if (parts == 1) {
							unsigned long long gg;
							for (gg=0; gg<unpacked_data_size; ++gg) {
								if (*(unsigned int *)&new_src[gg] == 0x464c457f && gg == 0) {
									file_type = 2; /* ELF */
									printf("|------ Filetype ELF.\n");
									break;
								}
								if (*(unsigned int *)&new_src[gg] == 0x0001ef53) {
									file_type = 3;  /* EXT4 */
									ext4_file_size = *(unsigned long long *)&new_src[gg-52] * 4096ULL;
									printf("|------ Found ext4 magic. Ext4 size: 0x%llx\n", ext4_file_size);
									break;
								}
							}
						}

						free(new_src);
						free(temp_buff);

						fseeko64(sin, revert_sin_poss + data_type_header_size, SEEK_SET);
						printf("|------ now sin poss next: 0x%llx\n", revert_sin_poss + data_type_header_size);
						break;

					case 0x52444441:  /* ADDR */
						parts += 1;
						printf("Extracting part %d type: ADDR\n", parts);
						revert_sin_poss = ftello64(sin) - 4;
						fread_unus_res(&data_type_header_size, 1, 4, sin);
						fread_unus_res(&packed_data_offset, 1, 8, sin);
						fread_unus_res(&packed_data_size, 1, 8, sin);
						fread_unus_res(&unpacked_data_offset, 1, 8, sin);
						data_type_header_size = swap_uint32(data_type_header_size);
						packed_data_offset = swap_uint64(packed_data_offset);
						packed_data_size = swap_uint64(packed_data_size);
						unpacked_data_offset = swap_uint64(unpacked_data_offset);
						printf("|-- packed data offset: 0x%llx\n", packed_data_offset);
						printf("|-- packed data size: 0x%llx\n", packed_data_size);
						printf("|-- unpacked data offset: 0x%llx\n", unpacked_data_offset);

						if (out_next_poss < unpacked_data_offset)
						{
							printf("|------ fill 0xFF: 0x%llx\n", unpacked_data_offset - out_next_poss);

							if ((temp_buff = (char *)malloc(unpacked_data_offset - out_next_poss)) == NULL) {
								printf("Failed to allocate 0x%llx bytes of the memory for FF!\n", unpacked_data_offset - out_next_poss);
								if (elf) fclose(elf);
								if (sin) fclose(sin);
								return 1;
							}

							memset(temp_buff, 0xff, unpacked_data_offset - out_next_poss);
							fwrite(temp_buff, 1, unpacked_data_offset - out_next_poss, elf);
							printf("|------ poss: 0x%llx expected: 0x%llx\n",
								 (unsigned long long)ftello64(elf),
								 (unsigned long long)unpacked_data_offset);

							free(temp_buff);

							if ((unsigned long long)ftello64(elf) != (unsigned long long)unpacked_data_offset) {
								printf("error, position didn't match expected! Please check free space on partition!\n");
								if (elf) fclose(elf);
								if (sin) fclose(sin);
								return 1;
							}
						}

						if ((temp_buff = (char *)malloc(packed_data_size)) == NULL) {
							printf("Failed to allocate 0x%llx bytes of the memory for *temp_buff for lz4 ex!\n", packed_data_size);
							if (elf) fclose(elf);
							if (sin) fclose(sin);
							return 1;
						}

						start_sin_poss = sin_header_sz;
						start_sin_poss += mmcf_h.gptp_len;
						start_sin_poss += sizeof(struct MMCF_Header);
						start_sin_poss += packed_data_offset;
						printf("|------ curr sin poss: 0x%llx\n", revert_sin_poss);
						printf("|------ start sin poss: 0x%llx\n", start_sin_poss);

						fseeko64(sin, start_sin_poss, SEEK_SET);

						fread_unus_res(temp_buff, 1, packed_data_size, sin);

						now_sin_poss = ftello64(sin);
						printf("|------ now sin poss: 0x%llx\n", now_sin_poss);
						printf("|------ now out poss: 0x%llx\n", (unsigned long long)ftello64(elf));

						printf("|------ successfully extracted part %d data.\n", parts);
						fwrite(temp_buff, 1, packed_data_size, elf);
						out_next_poss = ftello64(elf);

						if (parts == 1) {
							unsigned long long gg;
							for (gg=0; gg<packed_data_size; ++gg) {
								if (*(unsigned int *)&temp_buff[gg] == 0x464c457f && gg == 0) {
									file_type = 2; /* ELF */
									printf("|------ Filetype ELF.\n");
									break;
								}
								if (*(unsigned int *)&temp_buff[gg] == 0x0001ef53) {
									file_type = 3;  /* EXT4 */
									ext4_file_size = *(unsigned long long *)&temp_buff[gg-52] * 4096ULL;
									printf("|------ Found ext4 magic. Ext4 size: 0x%llx\n", ext4_file_size);
									break;
								}
							}
						}

						free(temp_buff);

						fseeko64(sin, revert_sin_poss + data_type_header_size, SEEK_SET);
						printf("|------ now sin poss next: 0x%llx\n", revert_sin_poss + data_type_header_size);
						break;

					default:
						printf("t=%lx Unknown part type!\n", t);
						if (sin) fclose(sin);
						return 1;
						break;
				}

				if (data_type_header_size)
					t += data_type_header_size;
				else
					t += 1;
				printf("|------ t=0x%lx d=0x%lx\n", t, data_type_header_size);
			}

			if (ext4_file_size) {
				unsigned long long g;
				g = ftello64(elf);
				if (g < ext4_file_size) {
					printf("finishing...\n");
					printf("|--- fill FF 0x%llx\n", ext4_file_size - g);

					if ((temp_buff = (char *)malloc(ext4_file_size - g)) == NULL) {
						printf("Failed to allocate 0x%llx bytes of the memory for FF!\n", ext4_file_size - g);
						if (elf) fclose(elf);
						if (sin) fclose(sin);
						return 1;
					}

					memset(temp_buff, 0xff, ext4_file_size - g);
					fwrite(temp_buff, 1 , ext4_file_size - g, elf);
					free(temp_buff);
				}
			}

			ext4_file_size = ftello64(elf);
			printf("|--- now out poss: 0x%llx\n", ext4_file_size);

			if (elf) fclose(elf);
			if (sin) fclose(sin);
			fflushall();

			if (strstr(fileout, "/dev/"))
			{
				printf("No sin file verification when flashing.\n");
			}
			else
			{
				printf("Sin file verification ");
				if (file_size(fileout) != ext4_file_size) {
					printf("error: %llx != %llx!\n", file_size(fileout), ext4_file_size);
					return 1;
				}
				printf("ok.\n");
				printf("Extracting...\n");
				printf("%s extracted.\n", fileout);
			}
			return file_type;
		}
	}
	else if (memcmp(mmcf_h.mmcf_magic, "\x7F" "ELF", 4) == 0)
	{
		printf("Loader?\n");
		file_type = 2;
		non_loader = 0;
	}
	else if (sin_header_sz+sin_data_sz != sin_sz)
	{
		printf("No MMCF. Need to calculate data size.\n");
		non_loader = 1;
	}
	else if (sin_header_sz+sin_data_sz == sin_sz) {
          printf("No MMCF. Size calc no need.\n");
		non_loader = 0;
	}
	else
	{
		printf("Error: sorry I am unable to calculate sin data size!\n");
		if (sin) fclose(sin);
		return 1;
	}

	if (non_loader) {
		printf("Calculating sin data size...\n");
		sin_data_first_sz += sin_data_sz;
		sin_data_sz = sin_sz - sin_header_sz - sin_data_sz;
		printf("New sin data size: 0x%llX\n", sin_data_sz);
	}
  
	if (sin_header_sz + sin_data_sz + sin_data_first_sz != sin_sz)
	{
		printf("Sin file verification failed!\n");
		if (sin)
			fclose(sin);
		return 1;
	}
	else
	{
		printf("Sin file verification ok.\n");
		printf("Extracting...\n");
    
		if ((elf = fopen64(fileout, "wb")) == NULL) {
			printf("Error, cant open %s for write!\n", fileout);
			if (sin)
				fclose(sin);
			return 1;
		}

		fseeko64(sin, sin_header_sz + sin_data_first_sz, SEEK_SET);

		for (i=0; i<sin_data_sz; ++i) {
			fread_unus_res(out, 1, 1, sin);
			fwrite(out, 1, 1, elf);
		}
		if (elf)
			fclose(elf);
		printf("%s extracted.\n", fileout);
	}

	if (sin)
		fclose(sin);

	fflushall();
  
	return file_type;
}

static int command(char *what) {
	int ret;
	static char buffer[300];
	snprintf(buffer, sizeof(buffer), "%s", what);
	ret = system(buffer);
#if 0
	printf("%s\n", buffer);
	printf("returned=%d OK.\n", ret);
#endif
	return ret;
}

int copyf(char *source_file, char *target_file)
{
	char ch;
	FILE *source, *target;
  
	if ((source = fopen64(source_file, "rb")) == NULL) {
		printf("source file %s not exist!\n", source_file);
		return 1;
	}
  
	if ((target = fopen64(target_file, "wb")) == NULL) {
		fclose(source);
		printf("Can't write target file %s!\n", target_file);
		return 1;
	}
  
	while(1)
	{
		ch = fgetc(source);
    
		if (!feof(source))
			fputc(ch, target);
		else
			break;
	}
  
	fclose(source);
	fclose(target);
	fflushall();
  
	return 0;
}

int extract_elf(char *dest, char *in)
{
	char fname[256];
	char fnameoff[256];
	FILE *fo = NULL;
	FILE *fooff = NULL;
	FILE *fi = NULL;
	char *buff;
	int tmp = 1;
	int i;

	unsigned long last_address = 0;
	unsigned short hdrphnum = 0;

	struct Elf32_Ehdr header32;
	struct Elf32_Phdr *phdr32 = NULL;
	struct Elf64_Ehdr header64;
	struct Elf64_Phdr *phdr64 = NULL;

	unsigned char head[6];
	int determined = 0;

	fi = fopen64(in, "rb");
	if (fi == NULL) {
		printf("unable to open %s\n", in);
		return 1;
	}

	printf("Extracting file %s\n", in);
	
	fseeko64(fi, 0, SEEK_SET);
	fread_unus_res(head, 1, 6, fi);
	determined = *(unsigned char *)&head[4];
	fseeko64(fi, 0, SEEK_SET);

	if (determined == 2)
	{
		fread_unus_res(&header64, 1, sizeof(header64), fi);

		if (header64.e_ident[0] != ELFMAG0 || header64.e_ident[1] != ELFMAG1
		 || header64.e_ident[2] != ELFMAG2 || header64.e_ident[3] != ELFMAG3)
		{
			printf("ELF magic not found, exiting\n");
			if (fi) fclose(fi);
			return 1;
		}
		printf("ELF magic found\n");
		printf("Entry point          : 0x%08llX\n", header64.e_entry);
		printf("Class                : 64-bit objects\n");
		printf("Program Header start : 0x%llx\n", header64.e_phoff);
		printf("Program Header size  : 0x%x\n", header64.e_phentsize);
		printf("Program Header count : %d\n" , header64.e_phnum);

		phdr64 = malloc(sizeof(struct Elf64_Phdr) * header64.e_phnum);
		fseeko64(fi, header64.e_phoff, SEEK_SET);
		for(i = 0; i < header64.e_phnum ; i++) {
			fread_unus_res(&phdr64[i], 1, sizeof(struct Elf64_Phdr), fi);
			printf("   PH[%d], type=%d, offset=0x%08llX, virtual=0x%08llX, phy=0x%08llX, size=0x%08llX\n",
				 i,
				 phdr64[i].p_type,
				 phdr64[i].p_offset,
				 phdr64[i].p_vaddr,
				 phdr64[i].p_paddr,
				 phdr64[i].p_filesz
			);
		}

		hdrphnum = header64.e_phnum;
		if (hdrphnum) {
			snprintf(fnameoff, sizeof(fnameoff), "%s/boot.img-base", dest);
			if ((fooff = fopen64(fnameoff, "wb")) != NULL) {
				fprintf(fooff, "%08llx\n", header64.e_entry & 0xf0000000);
				fclose(fooff);
			}
		}
	}
	else
	{
		fread_unus_res(&header32, 1, sizeof(header32), fi);

		if (header32.e_ident[0] != ELFMAG0 || header32.e_ident[1] != ELFMAG1 ||
		    header32.e_ident[2] != ELFMAG2 || header32.e_ident[3] != ELFMAG3) {
			printf("ELF magic not found, exiting\n");
			if (fi) fclose(fi);
			return 1;
		}

		printf("ELF magic found\n");
		printf("Entry point          : 0x%08X\n", header32.e_entry);
		printf("Class                : 32-bit objects\n");
		printf("Program Header start : 0x%x\n", header32.e_phoff);
		printf("Program Header size  : 0x%x\n", header32.e_phentsize);
		printf("Program Header count : %d\n" , header32.e_phnum);

		phdr32 = malloc(sizeof(struct Elf32_Phdr) * header32.e_phnum);
		fseeko64(fi, header32.e_phoff, SEEK_SET);

		for(i = 0; i < header32.e_phnum ; i++) {
			fread_unus_res(&phdr32[i], 1, sizeof(struct Elf32_Phdr), fi);
			printf("   PH[%d], type=%d, offset=0x%08X, virtual=0x%08X, phy=0x%08X, size=0x%08X\n",
				 i,
				 phdr32[i].p_type,
				 phdr32[i].p_offset,
				 phdr32[i].p_vaddr,
				 phdr32[i].p_paddr,
				 phdr32[i].p_filesz
			);
		}

		hdrphnum = header32.e_phnum;
		if (hdrphnum) {
			snprintf(fnameoff, sizeof(fnameoff), "%s/boot.img-base", dest);
			if ((fooff = fopen64(fnameoff, "wb")) != NULL) {
				fprintf(fooff, "%08x\n", header32.e_entry & 0xf0000000);
				fclose(fooff);
			}
		}
	}

	for(i = 0; i < hdrphnum; i++) {
		if (determined == 2)
			buff = malloc(phdr64[i].p_filesz);
		else
			buff = malloc(phdr32[i].p_filesz);

		snprintf(fname, sizeof(fname), "%d", i);
		switch (i)
		{
			case 0:
				snprintf(fname, sizeof(fname), "%s/boot.img-zImage", dest);
				snprintf(fnameoff, sizeof(fnameoff), "%s/boot.img-kerneloff", dest);
				break;
			case 1:
				snprintf(fname, sizeof(fname), "%s/boot.img-ramdisk.gz", dest);
				snprintf(fnameoff, sizeof(fnameoff), "%s/boot.img-ramdiskoff", dest);
				break;
			case 2:
				snprintf(fname, sizeof(fname), "%s/boot.img-dt", dest);
				snprintf(fnameoff, sizeof(fnameoff), "%s/boot.img-tagsoff", dest);
				break;
			case 3:
				snprintf(fname, sizeof(fname), "%s/certificate", dest);
				snprintf(fnameoff, sizeof(fnameoff), "%s/certificateoff", dest);
				break;
			default:
				snprintf(fname, sizeof(fname), "%s/unknown.%d.bin", dest, i);
				snprintf(fnameoff, sizeof(fnameoff), "%s/unknown.%d.off", dest, i);
				break;
		}
		printf("%d. Dumping %s\n", i, fname);
		fo = fopen64(fname, "wb");
		fooff = fopen64(fnameoff, "wb");
    
		if (fo == NULL || fooff == NULL) {
			printf("unable to open fo or foof for writting!\n");
			if (fi) fclose(fi);
			if (fo) fclose(fo);
			if (fooff) fclose(fooff);
			free(buff);
			return 1;
		}

		if (determined == 2)
		{
			fseeko64(fi, phdr64[i].p_offset, SEEK_SET);
			fread_unus_res(buff, 1, phdr64[i].p_filesz, fi);
			fwrite(buff, 1, phdr64[i].p_filesz, fo);
			/* cmdline position */
			last_address = phdr64[i].p_offset + phdr64[i].p_filesz + 0x8;
			/* write fileoff */
			fprintf(fooff, "%08llx\n", phdr64[i].p_paddr);
		}
		else
		{
			fseeko64(fi, phdr32[i].p_offset, SEEK_SET);
			fread_unus_res(buff, 1, phdr32[i].p_filesz, fi);
			fwrite(buff, 1, phdr32[i].p_filesz, fo);
			/* cmdline position */
			last_address = phdr32[i].p_offset + phdr32[i].p_filesz + 0x8;
			/* write fileoff */
			fprintf(fooff, "%08x\n", phdr32[i].p_paddr);
		}

		if (fo) fclose(fo);
		if (fooff) fclose(fooff);
		free(buff);
	}

	printf("   Seeking to cmdline address = 0x%08lX\n", last_address);
	snprintf(fname, sizeof(fname), "%s/boot.img-cmdline", dest);	
	fo = fopen64(fname, "wb");
	if (fo == NULL) {
		printf("unable to open %s\n", fname);
		if (fi) fclose(fi);
		return 1;
	}
	printf("%d. Dumping cmdline to %s\n", i, fname);

	do {
		buff = malloc(1);
		fseeko64(fi, last_address, SEEK_SET);
		fread_unus_res(buff, 1, 1, fi);
		tmp = getc(fi);
		fwrite(buff, 1, 1, fo);
		last_address += 1;
		fseeko64(fi, last_address, SEEK_SET);
		free(buff);
	} while (tmp != 0);

	if (fo)
		fclose(fo);
	if (fi)
		fclose(fi);

  fflushall();

	return 0;
}

int dump_boot(char* filein, char* fileout);

#ifndef EXTERNAL_KERNEL_DUMP
int main(int argc, char *argv[])
#else
int kernel_dump_main(int argc, char *argv[])
#endif
{
	FILE *fi = NULL;
	int fld_cbck;
	char fld[256];
	char fldren[256];
	char mkbtfmt[0x121];

#ifndef EXTERNAL_KERNEL_DUMP
	printf("--------------------------------------------------------\n");
	printf("       Sony Any kernel Dumper by Munjeni @ 2016         \n");
	printf("--------------------------------------------------------\n");
#endif

	if (argc != 3) {
#if !defined(_WIN32)
		printf("Syntax: kernel_dump OUTPUT_FOLDER /dev/block/BOOT_PARTITION\n");
#endif
		printf("Syntax: kernel_dump OUTPUT_FOLDER PATH_TO_THE_BOOT.IMG\n");
		printf("Syntax: kernel_dump OUTPUT_FOLDER PATH_TO_THE_KERNEL.ELF\n");
		printf("Syntax: kernel_dump OUTPUT_FOLDER PATH_TO_THE_KERNEL.SIN\n");
		printf("Syntax: kernel_dump OUTPUT_FOLDER PATH_TO_THE_ANYFILE.SIN\n");
		printf("To write sin directly to block device: kernel_dump OUTPUT_BLOCKDEV PATH_TO_THE_ANYFILE.SIN\n\n");
		return 1;
	}

	if (strstr(argv[1], "/dev/"))
	{
		printf("writing to %s\n", argv[1]);
	}
	else
	{
		snprintf(fld, sizeof(fld), "%s/", argv[1]);
		if (0 != access(fld, F_OK)) {
			if (ENOENT == errno) {
				snprintf(fld, sizeof(fld), "mkdir %s", argv[1]);
				fld_cbck = command(fld);
				if (fld_cbck == 0) {
					printf("Created ouput folder \"%s\"\n", argv[1]);
				} else {
					printf("FAILURE to create output folder %s!\nPllease try another folder!\n", argv[1]);
					return 1;
				}
			}

			if (ENOTDIR == errno) {
				printf("FAILURE to create output folder '%s' because there is file called '%s'!!!\n"
					"Try another one folder!\n", argv[1], argv[1]);
				return 1;
			}

		} else {
			printf("Using folder \"%s\"\n", argv[1]);
		}
	}

	snprintf(fld, sizeof(fld), "%s", argv[2]);

	printf("opening %s\n", fld);
	fi = fopen64(fld, "rb");
	if (fi == NULL) {
		printf("unable to open %s\n", fld);
		return 1;
	}
	fseeko64(fi, 0, SEEK_SET);
	fread_unus_res(mkbtfmt, 1, 0x120, fi);
	if(fi) fclose(fi);

	if (strstr(mkbtfmt,"ANDROID") != NULL)
	{
		printf("%s is Android image format.\n", fld);
		printf("Dumping to %s...\n", argv[1]);
		if (strstr(argv[1], "/dev/")) {
			printf("Error, you can't use img file to flash directly to %s! Only sin file!\n", argv[1]);
			return 1;
		} else {
			dump_boot(fld, argv[1]);
		}
	}
	else if (strstr(mkbtfmt,"SIN") != NULL || strcmp(mkbtfmt, "\x02\x00\x00\x00") == 0 || strcmp(mkbtfmt, "\x01\x00\x00\x00") == 0)
	{
		printf("%s is Sony sin format.\n", fld);

		if (strstr(argv[1], "/dev/"))
		{
			printf("Converting and flashing...\n");
			int cste = convert_sin_to_elf(argv[2], argv[1]);
			return cste;
		}
		else
		{
			printf("Converting...\n");
			snprintf(fld, sizeof(fld), "%s/converted.file", argv[1]);
			int cste = convert_sin_to_elf(argv[2], fld);
			switch (cste)
			{
				case 1:
					printf("Error converting!\n");
					remove(fld);
					return 1;

				case 2:
					snprintf(fldren, sizeof(fldren), "%s/%s.elf", argv[1], basename(argv[2]));
					break;

				case 3:
					snprintf(fldren, sizeof(fldren), "%s/%s.ext4", argv[1], basename(argv[2]));
					break;

				default:
					snprintf(fldren, sizeof(fldren), "%s/%s.unknown", argv[1], basename(argv[2]));
					break;
			}

			if (rename(fld, fldren) < 0) {
				remove(fldren);
				if (rename(fld, fldren) < 0) {
					printf("Error renaming \"%s\" to \"%s\"!\n", fld, fldren);
					remove(fld);
					return 1;
				}
			}

			printf("Renaming to %s succeed.\n", fldren);
		}
	}
	else if (strstr(mkbtfmt, "\x7f" "ELF") != NULL)
	{
		if (strstr(argv[1], "/dev/"))
		{
			printf("Error, you can't use elf file to flash directly to %s! Only sin file!\n", argv[1]);
			return 1;
		}
		else
		{
			if (extract_elf(argv[1], argv[2]) != 0) {
				printf("elf extraction returned an error!\n");
				return 1;
			}
		}
	}
	else if (memcmp(mkbtfmt, "\x1F\x8B", 2) == 0)
	{
     	FILE *a = NULL;
		printf("%s is Sony sin v4 format.\n", fld);

          snprintf(fld, sizeof(fld), "%s/converted.file", argv[1]);

		if (gunziper(argv[2], fld))
			return 1;

		a = fopen64(fld, "rb");
		if (a == NULL)
			fprintf(stderr, "Unable to open %s\n", fld);
		else {
			untar(a, argv[2], argv[1]);
			fclose(a);
		}

		remove(fld);
		return 0;
	}
	else if (memcmp(mkbtfmt+0x101, "\x75\x73\x74\x61", 4) == 0)
	{
     	FILE *a = NULL;
		printf("%s is Sony sin v5 format.\n", fld);

		a = fopen64(argv[2], "rb");
		if (a == NULL)
			fprintf(stderr, "Unable to open %s\n", argv[2]);
		else {
			untar(a, argv[2], argv[1]);
			fclose(a);
		}

	}
	else
	{
		printf("%s is usuported format!\n", fld);
		return 1;
	}

	printf("Done.\n");
	fflushall();
	return 0;
}


