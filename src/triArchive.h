
#ifndef __TRI_ARCHIVE_H__
#define __TRI_ARCHIVE_H__
 
#include "triTypes.h"
 
#define PACKED __attribute__((packed))
 
#define TRI_ARCHIVE_MAX_FS      (0xFFFFFF)  //16MB max file entry
#define TRI_ARCHIVE_MAX_FNLEN   (112)       //112byts max filename
#define TRI_ARCHIVE_MAGIC       ("triA")    //magic id
#define TRI_ARCHIVE_KEYLEN      (0x10)      //16 byte keylen
 
//archive flags
#define TRI_ARCHIVE_ENC         (0x01)
#define TRI_ARCHIVE_COMP        (0x02)
 
//compression types
#define TRI_ARCHIVE_ZLIB        (0x00)
 
typedef struct triArchiveHeader
{
	triU32 magic;
	triU16 version;
	triU16 numfiles;
	triU32 off_entry_table;
	triU32 off_file_table;
	triUChar resevered[8];
}PACKED triArchiveHeader;
 
typedef struct triArchiveEntry
{
	triChar filename[TRI_ARCHIVE_MAX_FNLEN];
	triU32 filesize;
	triU32 uncomp_filesize;
	triU32 off_file_table;
	triU32 crc;
}PACKED triArchiveEntry;
 
typedef struct triArchiveFile
{
	triChar* data;
	triU32 size;
}PACKED triArchiveFile;
 
typedef struct triArchive
{
	triU16 numfiles;
	triU16 version;
	triUChar key[TRI_ARCHIVE_KEYLEN];
	triChar archivename[TRI_ARCHIVE_MAX_FNLEN];
	triArchiveEntry* entries;
	triU32 off_file_table;
}PACKED triArchive;
 
triArchive*		triArchiveOpen(triChar* archivename,triChar* key);
triArchiveFile*	triArchiveRead(triChar *file, triArchive* archive);
void			triArchiveFileFree(triArchiveFile* archivefile);
void			triArchiveClose(triArchive *archive);
triU32			triCRC32(triChar* data, triU32 len);
 
#endif

