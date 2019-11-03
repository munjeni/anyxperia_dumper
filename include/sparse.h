/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SPARSE_H_
#define _SPARSE_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

struct SparseHeader
{
	unsigned int Magic;
	unsigned short MajorVersion;
	unsigned short MinorVersion;
	unsigned short FileHeaderSize;
	unsigned short ChunkHeaderSize;
	unsigned int BlockSize;
	unsigned int TotalBlocks;
	unsigned int TotalChunks;
	unsigned int ImageChecksum;
};

struct ChunkHeader
{
	unsigned short ChunkType;
	unsigned short Reserved;
	unsigned int ChunkBlocks;
	unsigned int ChunkSize;
};

#define SPARSE_HEADER_MAGIC	0xed26ff3a

#define CHUNK_TYPE_RAW		0xCAC1
#define CHUNK_TYPE_FILL		0xCAC2
#define CHUNK_TYPE_DONT_CARE	0xCAC3
#define CHUNK_TYPE_CRC32    0xCAC4
#define CHUNK_TYPE_LZ4    0xCAC5

#ifdef	__cplusplus
}
#endif

#endif
