#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "triArchive.h"
#include "triLog.h"
 
triArchive* triArchiveOpen(triChar* archivename,triChar* key)
{
	//sanity checks
	if(strlen(archivename) >= TRI_ARCHIVE_MAX_FNLEN)
	{
		triLogError("Archivename is too long\n");
		return NULL;
	}
	
	triArchiveHeader header;
	memset(&header,0,sizeof(triArchiveHeader));
	FILE *fd = fopen(archivename,"r+b");
	if(!fd)
	{
		triLogError("Couldn't open Archive %s\n",archivename);
		return NULL;
	}
	
	fread(&header,sizeof(header),1,fd);
	if(memcmp((char*)header.magic,TRI_ARCHIVE_MAGIC,4) || header.version != 1)
	{
		triLogError("Archiveheader from %s is malformed\n",archivename);
		return NULL;
	}
	
	//get entry table
	int entrytablesize = sizeof(triArchiveEntry) * header.numfiles;
	triArchiveEntry* entries = (triArchiveEntry*)malloc(entrytablesize);
	if(!entries)
	{
		triLogError("Couldn't allocate %d memory for entry table\n",entrytablesize);
		return NULL;
	}
	
	fseek(fd,header.off_entry_table,SEEK_SET);
	if(fread(entries,entrytablesize,1,fd) != entrytablesize)
	{
		triLogError("Couldn't read entry table\n");
		free(entries);
		fclose(fd);
		return NULL;
	}
	fclose(fd);
	
	triArchive* archive = (triArchive*)malloc(sizeof(triArchive));
	if(!archive)
	{
		triLogError("Couldn't allocate triArchive\n");
		free(entries);
		return NULL;
	}
	memset(archive,0,sizeof(triArchive));
	strncpy(archive->archivename,archivename,TRI_ARCHIVE_MAX_FNLEN);
	archive->version = header.version;
	archive->numfiles = header.numfiles;
	if(key)
		memcpy(archive->key,key,TRI_ARCHIVE_KEYLEN);
	archive->entries = entries;
	archive->off_file_table = header.off_file_table;
	
	return archive;
}
 
void triArchiveClose(triArchive *archive)
{
	if(archive)
		free(archive);
}

void triArchiveFileFree(triArchiveFile *archivefile)
{
	if(archivefile)
		free(archivefile);
}
 
triArchiveFile* triArchiveRead(triChar *file, triArchive* archive)
{
	if(strlen(file) >= TRI_ARCHIVE_MAX_FNLEN || archive == NULL)
		return NULL;
	
	triArchiveEntry *entry = archive->entries;
	int i;
	int found=0;
	for(i=0; i<archive->numfiles; i++,entry++)
	{
		if(!strncmp(entry->filename,file,TRI_ARCHIVE_MAX_FNLEN))
		{
			found = 1;
			break;
		}
	}
	
	if(!found)
		return NULL;
	
	unsigned char flags = (unsigned char)entry->filesize>>24;
	int size = entry->filesize&0x00FFFFFF;
	char *buffer = (char*)malloc(size);
	if(!buffer)
		return NULL;
	//open archive and read in file;
	FILE *fd = fopen(archive->archivename,"r+b");
	if(!fd)
	{
		free(buffer);
		return NULL;
	}
	fseek(fd,archive->off_file_table + entry->off_file_table,SEEK_SET);
	if(fread(buffer,size,1,fd) != size)
	{
		free(buffer);
		fclose(fd);
		return NULL;
	}
	fclose(fd);
	
	triArchiveFile *arfile = (triArchiveFile*)malloc(sizeof(triArchiveFile));
	if(!arfile)
		return NULL;
	
	arfile->data = buffer;
	arfile->size = size;
	
	if(flags & TRI_ARCHIVE_ENC == TRI_ARCHIVE_ENC)
	{
		int i,y=0;
		for(i=0; i<size; i++)
		{
			buffer[i]^=archive->key[y++];
			if(y == TRI_ARCHIVE_KEYLEN)
				y=0;
		}
	}
	
	if(flags & TRI_ARCHIVE_COMP == TRI_ARCHIVE_COMP)
	{
		triUChar comptype = (triUChar)entry->uncomp_filesize>>24;
		if(comptype & TRI_ARCHIVE_ZLIB == TRI_ARCHIVE_ZLIB)
		{
			triU32 uncompsize = entry->uncomp_filesize&0x00FFFFFF;
			char *buffer2 = (char*)malloc(uncompsize);
			if(!buffer2)
			{
				free(arfile);
				return NULL;
			}
	
			if(uncompress ((triUChar*)buffer2, &uncompsize, (triUChar*)buffer, size) != Z_OK)
			{
				free(arfile);
				free(buffer2);
				return NULL;
			}
			free(buffer);
			arfile->data = buffer2;
			arfile->size = uncompsize;
		}
	}
	
	if(triCRC32(buffer,size) != entry->crc)
	{
		free(arfile);
		return NULL;
	}
	
	return arfile;
}
 
//Adler 32 (c)Mark Adler
triU32 triCRC32(triChar* data, triU32 len)
{
	triU32 a = 1, b = 0;
	
	while (len > 0)
	{
		size_t tlen = len > 5550 ? 5550 : len;
		len -= tlen;
		do
		{
			a += *data++;
			b += a;
		} while (--tlen);
	
		a %= 65521;
		b %= 65521;
	}
	
	return (b << 16) | a;
}
