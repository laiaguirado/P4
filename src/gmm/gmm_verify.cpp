
#include <unistd.h> //getopt function, to parse options
#include <iostream>
#include <fstream>
#include <map>
#include "filename.h"
#include "gmm.h"

using namespace std;
using namespace upc;

const string DEF_FEAT_EXT = "mcp";
const string DEF_GMM_EXT  = "gmc";

int usage(const char *progname, int err);

int read_options(int ArgC, const char *ArgV[], vector<Directory> &input_dirs, vector<Ext> &input_exts, 
		 vector<Directory> &gmm_dirs, vector<Ext> &gmm_exts, 
		 vector<string> &input_filenames, vector<string> &candidates, 
		 vector<string> &gmm_filenames, string &world_name); 

int read_gmms(const Directory &dir, const Ext &ext, const vector<string> &gmm_filenames, map<string, GMM> &mgmm);



float verify(const GMM &gmm_candidate, const fmatrix &dat) {

  //TODO: implement verification score based on gmm of the candidate
  /*
    lprobcand is an informative values to be printed as debug information.
    The decision is based on the returned value
   */

  float score = 0.0F;
  return score;
}



float verify(const GMM &gmm_candidate, const GMM & gmm_world, const fmatrix &dat,
	     float &lprobcand, float &lprobbackground) {

  //TODO: implement verification score based on gmm of the candidate and 'world' model
  float score = 0.0F;
  lprobcand = 0.0F;
  lprobbackground = 0.0F;


  return score;

}



int main(int argc, const char *argv[]) {

  vector<Directory> input_dirs, gmm_dirs;
  vector<Ext> input_exts, gmm_exts; 
  vector<string> input_filenames, gmm_filenames, candidates;
  string  world_name;

  int retv = read_options(argc, argv, input_dirs, input_exts, 
			  gmm_dirs, gmm_exts, input_filenames, candidates, gmm_filenames, world_name);

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



  map<string,GMM> mgmm;
  retv = read_gmms(gmm_dirs[0], gmm_exts[0], gmm_filenames, mgmm);
  if (retv != 0)
    return usage(argv[0], retv);

  map<string,GMM>::const_iterator igmm_world = mgmm.end();
  if (!world_name.empty()) {
    igmm_world = mgmm.find(world_name);
    if (igmm_world == mgmm.end()) {
      cerr << "ERROR: GMM model for background model \"" << world_name << "\" not found." << endl;
      return -1;
    }
  }
    

  /* In this implementation, we assume that the world model is a gmm (gmm_world)
     and that each candidate has its onw gmm */

  ///Read and verify files
  for (unsigned int i=0; i<input_filenames.size(); ++i) {

    map<string,GMM>::const_iterator igmm = mgmm.find(candidates[i]);
    if (igmm == mgmm.end()) {
      cerr << "ERROR: GMM model for candidate \"" << candidates[i] << "\" not found." << endl;
      return -2;
    }
    const GMM &gmm_candidate = igmm->second;


    fmatrix dat;
    string path = input_dirs[0] + input_filenames[i] + input_exts[0];
    ifstream ifs(path.c_str(), ios::binary);
    if (ifs.good())
      ifs >> dat;

    if (!ifs.good()) {
      cerr << "Error reading data file: " << path << endl;
      return usage(argv[0],1);
    }


    if (world_name.empty()) {
      float score = verify(gmm_candidate, dat);
      cout << input_filenames[i] << '\t' << candidates[i] << '\t' << score << endl;
    } else {
      float score, probCandidate, probWorld;
      const GMM &gmm_world = igmm_world->second;
      score = verify(gmm_candidate, gmm_world, dat, probCandidate, probWorld);
      cout << input_filenames[i] << '\t' << candidates[i] << '\t' << score 
	   << '\t' << probCandidate <<'\t' << probWorld << endl; 
    }
  }
  return 0;
}

int read_gmms(const Directory &dir, const Ext &ext, const vector<string> &filenames, map<string, GMM> &vgmm) {
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
    vgmm[filenames[i]] = gmm;
    //    gmm.print(cout) << "----------------------\n";
  }
  return 0;
}

int usage(const char *progname, int err)  {
  cerr << "Usage: " << progname << " [options] list_gmm list_of_test_files list_of_candidate\n\n";
  
  cerr << "Options can be: \n"
       << "  -d dir\tDirectory of the feature files (def. \".\")\n"
       << "  -e ext\tExtension of the feature files (def. \"" << DEF_FEAT_EXT << "\")\n"
       << "  -D dir\tDirectory of the gmm files (def. \".\")\n"
       << "  -E ext\tExtension of the gmm files (def. \"" << DEF_GMM_EXT << "\")\n"
       << "  -w name\tName of the \"background\" GMM (def. do not use world model\")\n"
       << "         \tnname does not include directory and extension:\n"
       << "         \tthe dir option (-D) and ext (-e) will be added\n\n";

  cerr << "Each \"trial\" is defined by the speech files and the candidate (pretended user)\n"
       << "The number of items in both files has to be the same.\n\n";
  cerr << "For each input sentence, different feature files (and different GMMs)\n"
       << "can be provided using several times the options -d -e -D and -E\n";

  return err;
}

int read_options(int ArgC, const char *ArgV[], vector<Directory> &input_dirs, vector<Ext> &input_exts, 
		 vector<Directory> &gmm_dirs, vector<Ext> &gmm_exts, 
		 vector<string> &input_filenames, 
		 vector<string> &candidates, 
		 vector<string> &gmm_filenames, string &world_name) { 
  char option;
  //optarg and optind are global variables declared and set by the getopt() function

  while ((option = getopt(ArgC, (char **)ArgV, "d:e:D:E:w:")) != -1) {
    switch (option) {
    case 'd': input_dirs.push_back(optarg); break;
    case 'e': input_exts.push_back(optarg); break;
    case 'D': gmm_dirs.push_back(optarg); break;
    case 'E': gmm_exts.push_back(optarg); break;
    case 'w': world_name = optarg; break;
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
  
  if (ArgC != 3)
    return -3;

  //Save name of gmm files in vector 'gmm_filenames'
  ifstream is(ArgV[0]);
  if (!is.good()) {
    cerr << "ERROR opening list of gmm files: " << ArgV[0] << endl;
    return -4;
  }
  string s;
  bool bFound = false;
  while (is >> s) {
    gmm_filenames.push_back(s);
    if (s == world_name)
      bFound = true;
  }
  is.close();
  
  if (!bFound and not world_name.empty())
    gmm_filenames.push_back(world_name);


  //Save name of files in vector 'input_filenames'
  is.open(ArgV[1]);
  if (!is.good()) {
    cerr << "ERROR opening list of test files: " << ArgV[1] << endl;
    return -5;
  }
  while (is >> s)
    input_filenames.push_back(s);
  is.close();

  //Save name of files in vector 'input_filenames'
  is.open(ArgV[2]);
  if (!is.good()) {
    cerr << "ERROR opening list of candidates: " << ArgV[2] << endl;
    return -6;
  }
  while (is >> s)
    candidates.push_back(s);
  is.close();

  if (candidates.size() != input_filenames.size()) {
    cerr << "ERROR: num. of filenames (" << input_filenames.size() << ")"
	 << "does not match with number of candidates ( " << candidates.size() << ")\n";
    return -7;
  }
  return 0;      
}
