
#include <vector>
#include <sndfile.h>
#include "wavfile_mono.h"

using namespace std;

int readwav_mono(const string &filename, unsigned int &sampling_freq, vector<float> &x) {
  /** 
  Read input filename and returns sampling_freq and samples
  Input file should be mono
  Return: 0 => OK; < 0 => Error
  **/
  SNDFILE *sndfile_in;
  SF_INFO sf_info;
  x.clear();
  sampling_freq = 0;

  sndfile_in = sf_open(filename.c_str(), SFM_READ, &sf_info);
  if (sndfile_in == 0) //Error opening input file
    return -1;

  if (sf_info.channels  != 1) //Only mono files supported!
    return -2;

  x.resize(sf_info.frames);

  if(sf_read_float(sndfile_in, x.data(), x.size()) != (signed) x.size()) {
    //Error reading data
    x.clear();
    return -3;
  }

  sampling_freq = sf_info.samplerate;
  sf_close(sndfile_in);
  return 0;
}


int writewav_mono(const string &filename, unsigned int sampling_freq, const vector<float> &x) {
  /** 
  Write output filename
  Output file is mono
  Return: 0 => OK; <0 => Error
  **/
  SNDFILE *sndfile_out;
  SF_INFO sf_info;

 sf_info.frames = x.size();
 sf_info.samplerate = sampling_freq;
 sf_info.channels = 1;
 sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
 

 sndfile_out = sf_open(filename.c_str(), SFM_WRITE, &sf_info);
 if (sndfile_out == 0) //Error opening input file
   return -1;


 if(sf_write_float(sndfile_out, x.data(), x.size()) != (signed) x.size()) {
   //Error writting data
   return -3;
  }

  sf_close(sndfile_out);
  return 0;
}

