#include <fstream>
#include <unistd.h>
#include "matrix.h"

using namespace std;
using namespace upc;

int usage(const char *progname) {
  cerr << progname << "[options] fmatrix1 [....]\n";
  cerr << "where options are:\n";
  cerr << " -H: do not print filename and frame numbers\n";
  cerr << " -b: print binary values (float values, without header)\n";
  return -1;
}


int getOptions(int argc, const char *argv[], bool &bBinary, bool &bHeader) {
  char option;
  bBinary = false;
  bHeader = true;

  while ((option = getopt(argc, (char **) argv, "bHh")) != -1) {
    switch (option) {
    case 'b': bBinary = true;  break;
    case 'H': bHeader = false; break;
    case 'h':
    case '?': 
    default:
      return -1;
    }
  }
  if (bBinary and bHeader)
    bHeader = false;

  if (optind == argc) return -1;
  else return optind;
}

class MyFMatrix : public fmatrix {
  bool bFrames, bBinary;
 public: 
  MyFMatrix(bool _frames=true, bool _bin=false) : 
    bFrames(_frames), bBinary(_bin) {}

  void print(ostream &os) const {
    if (bBinary) {
      os.write((const char*) (*this)[0], nrow()*ncol()*sizeof(float));
    } else {
      for (unsigned int i=0; i<nrow(); ++i) {
	if (bFrames)
	  os << '[' << i << "]\t";
	for (unsigned int j=0; j<ncol(); ++j) {
	  if (j>0) os << '\t';
	  os << (*this)[i][j];
	}
	os << '\n';
      }
    }
  }
};

int main(int argc, const char *argv[]) {

  bool bHeader, bBinary;

  int posArg = getOptions(argc, argv, bBinary, bHeader);
  if (posArg < 0) {
    usage(argv[0]);
    return posArg;
  }

  for (int i=posArg; i<argc; ++i) {
 
    ifstream is(argv[i], ios::binary);
    if (!is.good()) {
      cerr << "Error opening input file: " << argv[i] << endl;
      return -2;
    }

    MyFMatrix m(bHeader, bBinary);
    is >> m;
    if (!is.good()) {
      cerr << "Error reading file: " << argv[i] << endl;
      return -3;
    }

    if (bHeader)
      cout << "FMATRIX: " << argv[i] << '\n';
    m.print(cout);    
    if (bHeader)
      cout << "----------------------------------------\n\n";
  }
  return 0;
}
