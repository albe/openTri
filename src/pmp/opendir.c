/*
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
"opendir" c code (~3 lines of perl -> ~100 lines of c :s)
*/


#include "opendir.h"


static SceIoDirent directory_entry;


void opendir_safe_constructor(struct opendir_struct *p)
	{
	p->directory = -1;

	p->directory_entry = 0;
	}


void opendir_close(struct opendir_struct *p)
	{
	if (!(p->directory < 0)) sceIoDclose(p->directory);

	if (p->directory_entry != 0) free_64(p->directory_entry);


	opendir_safe_constructor(p);
	}



static int	strcmpupr( char* s1, char* s2 )
	{
	while (*s1 && *s2)
		{
		char c1 = *s1++;
		char c2 = *s2++;

		if ((c1 >= 'a') && (c1 <= 'z'))
			c1 -= 'a' - 'A';
		if ((c2 >= 'a') && (c2 <= 'z'))
			c2 -= 'a' - 'A';

		if (c1 > c2)
			return 1;
		if (c1 < c2)
			return -1;
		}
	
	if (*s1)
		return 1;
	if (*s2)
		return -1;

	return 0;
	}


static int datecmp( ScePspDateTime *dt1, ScePspDateTime *dt2 )
	{
	if (dt1->year>dt2->year)
		return(1);
	if (dt1->year<dt2->year)
		return(-1);
		
	if (dt1->month>dt2->month)
		return(1);
	if (dt1->month<dt2->month)
		return(-1);

	if (dt1->day>dt2->day)
		return(1);
	if (dt1->day<dt2->day)
		return(-1);

	if (dt1->hour>dt2->hour)
		return(1);
	if (dt1->hour<dt2->hour)
		return(-1);

	if (dt1->minute>dt2->minute)
		return(1);
	if (dt1->minute<dt2->minute)
		return(-1);

	if (dt1->second>dt2->second)
		return(1);
	if (dt1->second<dt2->second)
		return(-1);

	return(0);
	}


char *opendir_open(struct opendir_struct *p, char *directory, char **filter, int sort)
	{
	opendir_safe_constructor(p);


	if (chdir(directory) < 0)
		{
		opendir_close(p);
		return("opendir_open: chdir failed");
		}




	p->directory = sceIoDopen(directory);
	if (p->directory < 0)
		{
		opendir_close(p);
		return("opendir_open: sceIoDopen failed");
		}




	unsigned int number_of_directory_entries = 0;


	while (1)
		{
		memset(&directory_entry, 0, sizeof(SceIoDirent));
		int result = sceIoDread(p->directory, &directory_entry);

		if (result == 0)
			{
			break;
			}
		else if (result > 0)
			{
			number_of_directory_entries++;
			}
		else
			{
			opendir_close(p);
			return("opendir_open: sceIoDread failed");
			}
		}




	sceIoDclose(p->directory);
	p->directory = -1;




	p->directory = sceIoDopen(directory);
	if (p->directory < 0)
		{
		opendir_close(p);
		return("opendir_open: sceIoDopen failed");
		}


	p->directory_entry = malloc_64(sizeof(SceIoDirent) * number_of_directory_entries);
	if (p->directory_entry == 0)
		{
		opendir_close(p);
		return("opendir_open: malloc_64 failed on directory_entry");
		}




	p->number_of_directory_entries = 0;


	int i = 0;

	for (; i < number_of_directory_entries; i++)
		{
		memset(&directory_entry, 0, sizeof(SceIoDirent));
		int result = sceIoDread(p->directory, &directory_entry);

		if (result == 0)
			{
			break;
			}
		else if (result > 0)
			{
			if (FIO_SO_ISREG(directory_entry.d_stat.st_attr))
			if (filter!=0)
				{
				int j = 0;
				while (filter[j]!=0)
					{
					char name[1024];
					strncpy(name,directory_entry.d_name + strlen(directory_entry.d_name) - strlen(filter[j]),1024);
					if (strcmpupr(name,filter[j])==0)
						{
						p->directory_entry[p->number_of_directory_entries] = directory_entry;
						p->number_of_directory_entries++;
						break;
						}
					j++;
					}
				} else {
					p->directory_entry[p->number_of_directory_entries] = directory_entry;
					p->number_of_directory_entries++;
				}
			}
		else
			{
			opendir_close(p);
			return("opendir_open: sceIoDread failed");
			}
		}


	sceIoDclose(p->directory);
	p->directory = -1;




	if (p->number_of_directory_entries == 0)
		{
		opendir_close(p);
		return("opendir_open: number_of_directory_entries == 0");
		}


	if ((sort&SORT_MASK)>0)
		{
		
		int		swap = 1;
		while (swap)
			{
			swap = 0;

			for (i = 0; i < p->number_of_directory_entries - 1; i++)
				{
				if (((sort&SORT_MASK)==SORT_NAME && strcmpupr( p->directory_entry[i].d_name , p->directory_entry[i+1].d_name) > 0) ||
					((sort&SORT_MASK)==SORT_SIZE && p->directory_entry[i].d_stat.st_size > p->directory_entry[i+1].d_stat.st_size) ||
					((sort&SORT_MASK)==SORT_MDATE && datecmp( &p->directory_entry[i].d_stat.st_mtime, &p->directory_entry[i+1].d_stat.st_mtime) > 0))
					{
					swap = 1;
					
					SceIoDirent temp = p->directory_entry[i];
					p->directory_entry[i] = p->directory_entry[i+1];
					p->directory_entry[i+1] = temp;
					}
				}
			}
		}
		
	if ((sort&SORT_REVERSE)==SORT_REVERSE)
		{
		for (i = 0; i < p->number_of_directory_entries/2; i++)
			{
			SceIoDirent temp = p->directory_entry[i];
			p->directory_entry[i] = p->directory_entry[p->number_of_directory_entries-i-1];
			p->directory_entry[p->number_of_directory_entries-i-1] = temp;
			}
		}

	return(0);
	}
