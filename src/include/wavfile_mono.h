#ifndef WAVFILE_MONO_H
#define WAVFILE_MONO_H

#include <string>
#include <vector>

/** 
    Read input sound filename and set the value of sampling_freq and samples
    This is just a wrapper of the sndfile lib, for the particular case that the input is mono.
    Return: 0 => OK; < 0 => Error.
*/
int readwav_mono(const std::string &filename, unsigned int &sampling_rate, std::vector<float> &x);
int writewav_mono(const std::string &filename, unsigned int sampling_freq, const std::vector<float> &x);


#endif
