#include <string.h>
#include <fstream>
#include "gmm.h"

using namespace std;
using namespace upc;


int main(int argc, const char *argv[]) {
  if (argc == 1) {
    cout << "USAGE: " << argv[0] << "file1.gmm [file2.gmm ...]\n";
    return -1;
  }

  int i;
  for (i=1; i<argc; ++i) {
 
    ifstream is(argv[i], ios::binary);

    if (!is.good()) {
      cerr << "Error opening input file: " << argv[i] << "\t(" << strerror(errno) << ")\n";
      return -2;
    }

    GMM gmm;
    is >> gmm;
    if (!is.good()) {
      cerr << "Error reading file: " << argv[i] << "\t(" << strerror(errno) << ")\n";
      return -3;
    }

    cout << "GMM: " << argv[i] << endl;
    gmm.print(cout);
    cout << "----------------------------------------\n";
  }

  return 0;
}
