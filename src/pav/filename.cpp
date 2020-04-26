/* Copyright (C) Jose Adrian Rodriguez Fonollosa 2004.
 *               Universitat Politecnica de Catalunya, Barcelona, Spain.
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */
#include <limits.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "filename.h"

#ifdef _MSC_VER
#include <conio.h>
#include "direct.h"
#else
#include <sys/select.h>
#include <termios.h>

int _kbhit() {

  struct termios initial;
  struct termios current;

  tcgetattr(0, &initial);
  current = initial;

  /* Line-by-line mode off */
  current.c_lflag &= ~ICANON;
  tcsetattr(0, TCSANOW, &current);


  struct timeval tv;
  fd_set read_fd;

  /* Do not wait at all, not even a microsecond */
  tv.tv_sec=0;
  tv.tv_usec=0;

  /* Must be done first to initialize read_fd */
  FD_ZERO(&read_fd);

  /* Makes select() ask if input is ready:
   * 0 is the file descriptor for stdin    */
  FD_SET(0,&read_fd);

  int hit = 0;

  /* The first parameter is the number of the
   * largest file descriptor to check + 1. */
  if(select(1, &read_fd,
            0, /*No writes*/
            0, /*No exceptions*/
            &tv)
     != -1) {

    /*	read_fd now holds a bit map of files that are
     * readable. We test the entry for the standard
     * input (file 0). */
    if(FD_ISSET(0,&read_fd))
      /* Character pending on stdin */
      hit = 1;
    
  }

  tcsetattr(0, TCSANOW, &initial);
  return hit;

}

#endif

inline bool keyBoardHit() {return (_kbhit() != 0); }

static int MkDir(const char *dir);

namespace upc {

int getCols(istream &is, vstring& cols)
{
	cols.clear();
	string line;
	if (getline(is, line) && line.size()>0) {
		string::size_type p;
		while ((p = line.find(ascii::TAB)) != string::npos) {
			cols.push_back(line.substr(0, p));
			line = line.substr(p+1);
		}
		cols.push_back(line);
	}
	return cols.size();
}

#ifdef _MSC_VER
bool keyStroke(char key)
{
	if (keyBoardHit() && ((key == 0) || (cin.get() == key)))
		return true;
	else
		return false;
}
#else
bool keyStroke(char key)
{
  if (!keyBoardHit())
    return false;

  if (key == 0)
    return true; 

  struct termios initial;
  struct termios current;

  tcgetattr(0, &initial);
  current = initial;

  /* One or more characters are sufficient
   * to cause a read to return. */
  current.c_cc[VMIN] = 1;
  /* No timeout; read waits forever
   * until ready. */
  current.c_cc[VTIME] = 0;
    
  /* Line-by-line mode off */
  current.c_lflag &= ~ICANON;
  tcsetattr(0, TCSANOW, &current);

  char ch = cin.get();
  tcsetattr(0, TCSANOW, &initial);


  if (ch == key)
    return true;
  else
    return false;
}
#endif



#ifndef _MSC_VER
#define _stati64 stat
#define _S_IFDIR S_IFDIR
#define _MAX_PATH PATH_MAX
#define MKDIR(A) mkdir((A), 0777)
#else
#define MKDIR mkdir
#endif

FileInfo fileInfo(const Path& fname) {
	struct _stati64 st;
	if (_stati64(fname.c_str(), &st) == 0) {
		FileInfo::ftype tp;
		if (st.st_mode & _S_IFDIR)
			tp = FileInfo::DIR;
		else
			tp = FileInfo::REG;
		return FileInfo(tp, st.st_size);
	}
	else {
		return FileInfo(FileInfo::ERR, -1);
	}
};

bool Filename::exist() const
{
	FileInfo st = fileInfo(*this);
	return (st.type() == FileInfo::REG);
}

long long Filename::size() const
{
	FileInfo st = fileInfo(*this);
	return st.size();
}


bool Filename::checkDir(bool create) const
{
	Directory d = path();
	if (create)
		return d.make();
	else
		return d.exist();
}

Directory Filename::path() const
{
	Filename::size_type p;
	if ((p = find_last_of("\\/:")) != npos)
		return substr(0,p+1);
	else
		return "";
}

bool Directory::exist() const
{
	FileInfo st = fileInfo(*this);
	return (st.type() == FileInfo::DIR);
}

bool Directory::make() const
{
	return (MkDir(c_str()) == 0);
}

} // namespace adl

static int _MkDir(const char* path)
{
	struct stat buf;
	if (stat(path, &buf) != 0)
		return MKDIR(path);
	else
		return 0;
}

static int MkDir(const char *dir)
{
	char tmp[_MAX_PATH+1];
	unsigned int i; 
	int retv = 0;
	size_t len;

	if (!dir || !dir[0])
		return 0;

	len = strlen(dir);
	if (len > _MAX_PATH)
		return -1;

	strcpy(tmp, dir);
	for (i=1; tmp[i]; i++) {
		if ((tmp[i] == '\\' || tmp[i] == '/') && (tmp[i-1] != ':')) {
			tmp[i] = 0;
			retv = _MkDir(tmp);
			if (i+1 == len) 
				return retv;
			strcpy(tmp, dir);
		}
	}
	return _MkDir(dir);
}
