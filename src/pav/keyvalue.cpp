#include <string>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <cctype>
#include "keyvalue.h"


#include <iostream>

using namespace std;
using namespace upc;

static string kv_null;
static float fNotFound = -1e30;
static int   iNotFound = -(0x8FFFFFFF);

KeyValue::KeyValue(const std::string & s) {
  if (s.empty())
    return;
  else
    set(s);
}

void KeyValue::set(const std::string & s) {
  //Remove possible blanks
  string str(s);
  str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());

  istringstream istr(str), istr2;
  string token, key, value;

  while (getline(istr, token, ';')) {
    string::size_type pos = token.find('=');
    key = token.substr(0,pos);
    value = token.substr(pos+1);
    kv[key] = value;
  }
}

const std::string & KeyValue::operator()(const std::string & key) const {
  const_iterator i = kv.find(key);
  if (i == kv.end())
    return kv_null;
  else
    return i->second;
}

bool KeyValue::to_float(const std::string & key, float &f) const {
  f = fNotFound;
  string s = (*this)(key);
  if (s.empty()) return false;
  f = atof(s.c_str());
  return true;
}

bool KeyValue::to_int(const std::string & key, int &i) const {
  i = iNotFound;
  string s = (*this)(key);
  if (s.empty()) return false;
  i = atoi(s.c_str());
  return true;
}

bool KeyValue::to_vector(const std::string & key, vector<float> &v) const {
  v.clear();
  string s = (*this)(key);
  if (s.empty()) return false;

  string str(s);
  replace(s.begin(), s.end(), ',', ' ');
  istringstream istr(s);
  while (istr >> s) 
    v.push_back(atof(s.c_str()));
  return true;
}

#if 0
//deprecated ...
float KeyValue::to_float(const std::string & key) const {
  float f;
  to_float(key, f);
  return f;
}

float KeyValue::to_int(const std::string & key) const {
  int i;
  to_int(key, i);
  return i;
}
#endif
