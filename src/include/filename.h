/* Copyright (C) Jose Adrian Rodriguez Fonollosa 2004.
 *               Universitat Politecnica de Catalunya, Barcelona, Spain.
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */
#ifndef _PV_FILENAME_H_
#define _PV_FILENAME_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

namespace upc {

// Useful types
typedef vector<string> vstring;

namespace ascii {
	const char CR   = 0x0d;   // Carriage return character
	const char NL   = 0x0a;   // Newline or Linefeed character
	const char LF   = 0x0a;   // Newline or Linefeed character
	const char FF   = 0x0c;   // Formfeed character
	const char TAB  = 0x09;   // Tab character
	const char ESC  = 0x1b;   // Escape character
}

// Utilities for formatted files with TAB separated colums
int getCols(istream& is, vstring& cols);

// Check key stroke
bool keyStroke(char key = 0);

// FileInfo
class FileInfo {
public:
	enum ftype {ERR, DIR, REG};
	FileInfo(ftype tp, long long size) : _type(tp), _size(size) {}
	ftype type() const {return _type; }
	long long size() const {return _size; }
private:
	ftype _type;
	long long _size;
};

// Ext
class Ext : public string {
public:
	Ext(const string& str) : string(str) {check_dot(); };
	Ext(const char* str) : string(str) {check_dot(); };
private:
	void check_dot() {
		if (empty() || substr(0,1) != ".") *this = "." + *this;
	}
};

// Path
class Path : public string {
public:
	Path() {};
	Path(const string& str) : string(str) {};
	Path(const char* str) : string(str) {};
	virtual ~Path() {}
};

// directory
class Directory : public Path {
public:
	Directory() {};
	Directory(const Path& p) : Path(p) {};
	Directory(const string& p) : Path(p) {};
	Directory(const char* p) : Path(p) {};
	bool make() const;
	bool exist() const;
	virtual ~Directory() {}
};

// FileName
class Filename : public Path {
public:
	Filename() {};
	Filename(const Path& p) : Path(p) {};
	Filename(const string& p) : Path(p) {};
	Filename(const char* p) : Path(p) {};
	bool checkDir(bool create = true) const;
	bool exist() const;
	Directory path() const;
	long long size() const;
	virtual ~Filename() {}
};

} // namespace upc

#endif
