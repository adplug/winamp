/*
  Copyright (c) 1999 - 2002 Simon Peter <dn.tlp@gmx.net>
  Copyright (c) 2002 Nikita V. Kalaganov <riven@ok.ru>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "plugin.h"

void FileTypes::add(const char *type, const char *name, bool ignore)
{
	work.type.push_back(type);
	work.name.push_back(name);
	work.ignore.push_back(ignore);
}

char *FileTypes::export_filetypes(char *buf)
{
	char *retval = buf;

	for (int i=0;i<get_size();i++)
		if(!work.ignore[i]) {
			memcpy(buf,work.type[i].c_str(),work.type[i].length());
			buf += work.type[i].length();
			*buf++ = 0;

			memcpy(buf,work.name[i].c_str(),work.name[i].length());
			buf += work.name[i].length();
			*buf++ = 0;
		}

	*(int *)buf = 0;

	return retval;
}

int FileTypes::get_size()
{
	return work.type.size();
}

const char *FileTypes::get_name(int i)
{
	return work.name[i].c_str();
}

bool FileTypes::get_ignore(int i)
{
	return work.ignore[i];
}

void FileTypes::set_ignore(int i, bool val)
{
	work.ignore[i] = val;
}

bool FileTypes::grata(const char *fname)
{
	char *tmpstr = strrchr(fname,'.');
	if (!tmpstr)
		return true;

	char *p = (char *)malloc(strlen(++tmpstr)+1);
	if (!p)
		return true;

	strcpy(p,tmpstr);

	for (int i=0;i<get_size();i++)
	{
		if (!work.ignore[i])
			continue;

		string tmpxstr = work.type[i];

		const char *ext = tmpxstr.c_str();
		char       *str = strstr(ext,_strlwr(p));

		if (str)
		{
			// for "aaa;bbb;ccc" and "ccc"
			if (strlen(p) == strlen(str))
			{
				free(p);
				return false;
			}

			if (str[strlen(p)] == ';')
			{
				// for "aaa;bbb;ccc" and "aaa"
				if (ext == str)
				{
					free(p);
					return false;
				}

				// for "aaa;bbb;ccc" and "bbb"
				if (ext[str-ext-1] == ';')
				{
					free(p);
					return false;
				}
			}
		}
	}

	free(p);
	return true;
}

void FileTypes::set_ignore_list(const char *ignore_list)
{
	char *str,*spos;
	str = spos = (char *)ignore_list;

	while (*str)
	{
		while (*spos != ';')
			spos++;
		*spos = 0;

		unsigned int ipos = atoi(str);
		if (ipos < get_size())
			work.ignore[ipos] = true;

		*spos++ = ';';
		str = spos;
	}
}

const char *FileTypes::get_ignore_list()
{
	char tmpstr[11];

	for (int i=0;i<get_size();i++)
		if (work.ignore[i])
		{
			xstrlist.append(_itoa(i,tmpstr,10));
			xstrlist.append(";");
		}

	return xstrlist.c_str();
}
