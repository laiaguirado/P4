#ifndef _KEY_VALUE
#define _KEY_VALUE

#include <string>
#include <map>
#include <vector>

namespace upc {
  /**
     Class to tranform parameters in a string format as a key/value table 

     string s = "A=3; B=hola; lista=3,2,5;"
     KeyValue kv(s);
     cout << kv("A") << '\n';
     cout << kv("B") << '\n';
     cout << kv("lista") << '\n';

     This will show
     "3"
     "hola"
     "3,2,5"
     
     Furthermore, it can transform the result to int, float, vector.

     E.g.:
     int i;
     kv.to_int("A",i);
     vector<float> x;
     kv.to_vector("lista", x);

  **/

  class KeyValue {
    typedef std::map<std::string, std::string> S2SMAP; 
    typedef S2SMAP::iterator iterator;
    typedef S2SMAP::const_iterator const_iterator;
    /// Map to store key/value
    S2SMAP kv;    
  public: 
    KeyValue(const std::string & raw_parameters = "");
    ///Initialize table from string, e.g. "city=Barcelona; year=2015"
    void set(const std::string & raw_parameters);

    ///Give the value (as a string) of a given key
    const std::string &operator()(const std::string & key) const; 

    ///Get the value as a float
    bool  to_float(const std::string & key, float &) const; 
    ///Get the value as an int
    bool  to_int(const std::string & key, int &) const; 
    ///Get the value as a vector of floats
    bool  to_vector(const std::string & key, std::vector<float> &) const; 
  };
}

#endif
