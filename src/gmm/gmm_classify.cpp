#include <unistd.h> //getopt function, to parse options
#include <iostream>
#include <fstream>
#include "filename.h"
#include "gmm.h"

using namespace std;
using namespace upc;

const string DEF_FEAT_EXT = "mcp";
const string DEF_GMM_EXT  = "gmc";

int usage(const char *progname, int err);

int read_options(int ArgC, const char *ArgV[], vector<Directory> &input_dirs, vector<Ext> &input_exts, 
		 vector<Directory> &gmm_dirs, vector<Ext> &gmm_exts, 
		 vector<string> &input_filenames, 
		 vector<string> &gmm_filenames); 

int read_gmms(const Directory &dir, const Ext &ext, const vector<string> &gmm_filenames, vector<GMM> &vgmm);


int classify(const vector<GMM> &vgmm, const fmatrix &dat, float &maxlprob) {
  float  lprob;
  int maxind  = -1;
  maxlprob = -1e38;

  //TODO .. assign maxind to the best index of vgmm
  //for each gmm, call logprob. Implement this function in gmm.cpp
  maxind = 0;


  return maxind;
}

int main(int argc, const char *argv[]) {

  vector<Directory> input_dirs, gmm_dirs;
  vector<Ext> input_exts, gmm_exts; 
  vector<string> input_filenames, gmm_filenames;

  int retv = read_options(argc, argv, input_dirs, input_exts, 
			  gmm_dirs, gmm_exts, input_filenames, gmm_filenames);

  if (retv != 0)
    return usage(argv[0], retv);
#if 0
  cout << "IDIR----------\n"; for (unsigned int i=0; i<input_dirs.size(); ++i) cout << input_dirs[i] << endl;
  cout << "GDIR----------\n"; for (unsigned int i=0; i<gmm_dirs.size(); ++i) cout << gmm_dirs[i] << endl;
  cout << "IEXT----------\n"; for (unsigned int i=0; i<input_exts.size(); ++i) cout << input_exts[i] << endl;
  cout << "GEXT----------\n"; for (unsigned int i=0; i<gmm_exts.size(); ++i) cout << gmm_exts[i] << endl;
  cout << "INAM----------\n"; for (unsigned int i=0; i<input_filenames.size(); ++i) cout << input_filenames[i] << endl;
  cout << "GNAM----------\n"; for (unsigned int i=0; i<gmm_filenames.size(); ++i) cout << gmm_filenames[i] << endl;
#endif



  /*
    Toni: I have implemented the reading of arguments for multiple GMM/Features. Read GMMs
    But here I will only use the first set of GMM/vectors.
    
    You can use data like this ...
    <vector<vector<GMM> > mgmm; mgmm.resize(3); mgmm[0] = vgmm; 
    <vector<fmatrix> vfmat; 
 
  */

  vector<GMM> vgmm;
  retv = read_gmms(gmm_dirs[0], gmm_exts[0], gmm_filenames, vgmm);
  if (retv != 0)
    return usage(argv[0], retv);

  ///Read and classify files
  for (unsigned int i=0; i<input_filenames.size(); ++i) {
    fmatrix dat;
    string path = input_dirs[0] + input_filenames[i] + input_exts[0];
    ifstream ifs(path.c_str(), ios::binary);
    if (ifs.good())
      ifs >> dat;

    if (!ifs.good()) {
      cerr << "Error reading data file: " << path << endl;
      return usage(argv[0],1);
    }

    int nclass;
    float lprob;
    nclass = classify(vgmm, dat, lprob);
    cout << input_filenames[i] << '\t' << gmm_filenames[nclass] 
	 << '\t' << lprob << endl; 
  }

  return 0;
}

int read_gmms(const Directory &dir, const Ext &ext, const vector<string> &filenames, vector<GMM> &vgmm) {
  vgmm.clear();
  GMM gmm;
  
  for (unsigned int i=0; i<filenames.size(); ++i) {
    string path = dir + filenames[i] + ext;
    ifstream ifs(path.c_str(), ios::binary);
    if (ifs.good())
      ifs >> gmm;

    if (!ifs.good()) {
      cerr << "Error reading GMM file: " << path << endl;
      return -1;
    }
    vgmm.push_back(gmm);
    //    gmm.print(cout) << "----------------------\n";
  }
  return 0;
}

int usage(const char *progname, int err)  {
  cerr << "Usage: " << progname << " [options] list_gmm list_of_test_files\n\n";
  
  cerr << "Options can be: \n"
       << "  -d dir\tDirectory of the feature files (def. \".\")\n"
       << "  -e ext\tExtension of the feature files (def. \"" << DEF_FEAT_EXT << "\")\n"
       << "  -D dir\tDirectory of the gmm files (def. \".\")\n"
       << "  -E ext\tExtension of the gmm files (def. \"" << DEF_GMM_EXT << "\")\n\n";

  cerr << "For each input sentence, different feature files (and different GMMs)\n"
       << "can be provided using several times the options -d -e -D and -E\n";

  return err;
}

int read_options(int ArgC, const char *ArgV[], vector<Directory> &input_dirs, vector<Ext> &input_exts, 
		 vector<Directory> &gmm_dirs, vector<Ext> &gmm_exts, 
		 vector<string> &input_filenames, 
		 vector<string> &gmm_filenames) { 
  char option;
  //optarg and optind are global variables declared and set by the getopt() function

  while ((option = getopt(ArgC, (char **)ArgV, "d:e:D:E:")) != -1) {
    switch (option) {
    case 'd': input_dirs.push_back(optarg); break;
    case 'e': input_exts.push_back(optarg); break;
    case 'D': gmm_dirs.push_back(optarg); break;
    case 'E': gmm_exts.push_back(optarg); break;
    case '?': return -1;
    }
  }
  if (input_dirs.empty()) input_dirs.push_back("./");
  if (gmm_dirs.empty()) gmm_dirs.push_back("./");
  if (input_exts.empty()) input_exts.push_back(DEF_FEAT_EXT);
  if (gmm_exts.empty()) gmm_exts.push_back(DEF_GMM_EXT);


  if (input_dirs.size() != input_exts.size() or
      input_dirs.size() != gmm_dirs.size() or
      input_dirs.size() != gmm_exts.size()) {
    cerr << ArgV[0] << ": ERROR - Same number of feature/gmm directories/extensions need to be provided." << endl;
    return -2;    
  }

  //Add ending '/' to directories, and leading '.' to extensions
  for (unsigned int i=0; i<input_dirs.size(); ++i) {
    if (!input_dirs[i].empty() and *(input_dirs[i].rbegin()) != '/') input_dirs[i] += '/';
    if (!gmm_dirs[i].empty() and *(gmm_dirs[i].rbegin()) != '/') gmm_dirs[i] += '/';
    if (!input_exts[i].empty() and input_exts[i][0] != '.') input_exts[i] = '.' + input_exts[i];
    if (!gmm_exts[i].empty() and gmm_exts[i][0] != '.') gmm_exts[i] = '.' + gmm_exts[i];
  }

  //advance argc and argv to skip read options
  ArgC -= optind;
  ArgV += optind;
  
  if (ArgC != 2)
    return -3;

  //Save name of gmm files in vector 'gmm_filenames'
  ifstream is(ArgV[0]);
  if (!is.good()) {
    cerr << "ERROR opening list of gmm files: " << ArgV[0] << endl;
    return -4;
  }
  string s;
  while (is >> s)
    gmm_filenames.push_back(s);
  is.close();

  //Save name of files in vector 'input_filenames'
  is.open(ArgV[1]);
  if (!is.good()) {
    cerr << "ERROR opening list of test files: " << ArgV[1] << endl;
    return -5;
  }
  while (is >> s)
    input_filenames.push_back(s);
  is.close();
  
  return 0;      
}
