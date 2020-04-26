#include <unistd.h> //getopt function, to parse options
#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <stdlib.h>
#include <math.h>
#include "filename.h"
#include "matrix.h"

using namespace std;
using namespace upc;


//Add to fmatrix, the function to dump it in a text file
class MyFMatrix : public fmatrix {
  bool bBinary;

public:
  MyFMatrix(bool _bin=false) :
    bBinary(_bin) {}

  ostream & print(ostream &os) const {
    cout << "Ep!\n";
    if (bBinary) {
      os << (fmatrix) (*this);
    } else {
      for (unsigned int i=0; i<nrow(); ++i) {
        os << (*this)[i][0];
        for (unsigned int j=1; j<ncol(); ++j) {
          os << '\t' << (*this)[i][j];
        }
        os << '\n';
      }
    }
    return os;
  }
};

ostream &operator<<(ostream &os, const MyFMatrix &dat) {
  return dat.print(os);
}


int read_data(const vector<string> &filenames, fmatrix &dat);

int usage(const char *progname, int err);

int read_options(int ArgC, const char *ArgV[], vector<string> &filenames,
                 Filename &cov_filename, Filename &covnorm_filename, 
		 bool &binary);


bool compare_corr(pair<int, float> a, pair<int, float> b) { 
  return fabs(a.second) > fabs(b.second);
} 

void report_pearson(ostream &os, const fmatrix &sigma) {
  if (sigma.ncol() == 2) {
    os << sigma[0][1] << '\n';
    return;
  }

  for (unsigned int i = 0; i < sigma.ncol(); i++) {
    os << "Feature " << i << '\n';
    vector<pair<int, float> > corr;
    for (unsigned int j = 0; j < sigma.ncol(); j++) {
      if (j!= i)
	corr.push_back(make_pair(j, sigma[i][j]));
    }
    sort(corr.begin(), corr.end(), compare_corr);   
    for (unsigned int j = 0; j < corr.size(); j++)
      os << corr[j].second << "\trho[" << i << "][" << corr[j].first << "]\n";
    os << '\n';    
  }
}


void normalize_covarianze(fmatrix &cov) {

  unsigned int ncof = cov.ncol();
  if (ncof != cov.nrow())
    return; //error

  for (unsigned int i=0; i<ncof; ++i) {
    for (unsigned int j=0; j<ncof; ++j) {
      if (i == j)
        continue;
      cov[i][j] = cov[i][j]/(sqrt(cov[i][i] * cov[j][j]));
    }
  }
  for (unsigned int i=0; i<ncof; ++i)
    cov[i][i] = 1.0F;
}


void covarianze(const fmatrix &dat, fmatrix &cov) {
  unsigned int ncof = dat.ncol(), i, j, n;
  cov.resize(ncof, ncof);
  fvector mean(ncof);

  for (n=0; n<dat.nrow(); ++n) {
    for (i=0; i<ncof; ++i) {
      mean[i] += dat[n][i];
      for (j=0; j<ncof; ++j)
        cov[i][j] += dat[n][i] * dat[n][j];
    }
  }
  for (i=0; i<ncof; ++i)
    mean[i] /= n;

  for (i=0; i<ncof; ++i) {
    for (j=0; j<ncof; ++j)
      cov[i][j] = cov[i][j]/n - mean[i] * mean[j];
  }
}


int main(int argc, const char *argv[]) {

  vector<string> filenames;
  Filename cov_filename, covnorm_filename;
  bool binary = false;

  ///Read command line options
  int retv = read_options(argc, argv, filenames, cov_filename, covnorm_filename, binary);

  if (retv != 0)
    return usage(argv[0], retv);

  //Read data from filenames
  fmatrix data;
  read_data(filenames, data);
  cout << "DATA: " << data.nrow() << " x " << data.ncol() << endl;

  MyFMatrix sigma;

  covarianze(data, sigma);

  if (!cov_filename.empty()) {
    //Create directory, if it is needed
    cov_filename.checkDir();

    //Save covarianze
    ofstream ofs(cov_filename.c_str());
    ofs <<sigma;
  }

  normalize_covarianze(sigma);

  if (!covnorm_filename.empty()) {
    covnorm_filename.checkDir();
    ofstream ofs(covnorm_filename.c_str());
    ofs <<sigma;
  }


  report_pearson(cout, sigma);

  return 0;
}

int usage(const char *progname, int err)  {
  cerr << "Usage: " << progname << " [options] file1.dat [file2.dat ...]\n\n";


  cerr << "Options can be: \n"
       << "  -C file\tWrite covarianze file (def. do not write)\n"
       << "  -N file\tWrite norm. covarianze file (def. do not write)\n"
       << "  -b     \tWrite cov. files as a binary fmatrix (def. false)\n\n";

  cerr << "This program computes the pearson correlation between\n" 
    "the different vector features (normalized covarianze matrix)\n\n";

  return err;
}

int read_options(int ArgC, const char *ArgV[], vector<string> &filenames,
                 Filename &cov_filename, Filename &covnorm_filename, 
		 bool &binary) {

  char option;
  filenames.clear();

  //optarg and optind are global variables declared and set by the getopt() function

  while ((option = getopt(ArgC, (char **)ArgV, "bC:N:")) != -1) {
    switch (option) {
    case 'C': cov_filename = optarg; break;
    case 'N': covnorm_filename = optarg; break;
    case 'b': binary = true; break;
    case '?': return -1;
    }
  }

  //advance argc and argv to skip read options
  ArgC -= optind;
  ArgV += optind;

  if (ArgC == 0)
    return -1;

  //Save name of files in vector 'filenames'
  for (int i=0; i<ArgC; ++i)
    filenames.push_back(ArgV[i]);

  return 0;
}


int read_data(const vector<string> &filenames, fmatrix &dat) {
  fmatrix dat1;
  for (unsigned int i=0; i<filenames.size(); ++i) {
    ifstream is(filenames[i].c_str(), ios::binary);
    if (!is.good()) {
      cerr << "Error reading file: " << filenames[i] << endl;
      dat.reset();
      return -1;
    }
    if (i==0) {
      is >> dat;
    } else {
      is >> dat1;
      if (dat1.ncol() != dat.ncol()) {
        cerr << "Error in vector dimension: " << filenames[i] << dat1.ncol()
             << " (expected: " << dat.ncol() << ")\n";
        dat.reset();
        return -1;
      }
      int row = dat.nrow();
      dat.resize(dat.nrow()+dat1.nrow(), dat.ncol());
      for (unsigned int i=0; i<dat1.nrow(); ++i, ++row)
        for (unsigned int j=0; j<dat1.ncol(); ++j)
          dat[row][j] = dat1[i][j];
    }
  }
  return 0;
}
