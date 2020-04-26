#include <math.h>
#include "digital_filter.h"
#include <iostream>

namespace upc {
  ///Allocate memory for state conditions
  void DigitalFilter::prepare_state() {
    unsigned int n = (a.size() > b.size() ? a.size() : b.size());
    if (u.size() != n) {
      u.clear();
      u.resize(n);
      index.resize(n);
    }
    /*
      if (a[0] != 1) {
      for (int i=0; i<b.size(); ++i) b[i] /= a[0];
      for (int i=1; i<a.size(); ++i) a[i] /= a[0];
      a[0] = 1.0F;
      }
    */
  }

  void DigitalFilter::set_resonator(float norm_central_freq,
                                    float norm_bandwidth) {

    std::vector<float> coef(3);
    coef[0] = 1;

    float r;
    r = exp(-M_PI*norm_bandwidth);
    coef[1] = -2*r*cos(2*M_PI*norm_central_freq);
    coef[2] = r*r;
    set_a(coef);
  }

  float DigitalFilter::operator()(float x) {    //Filter one sample
    unsigned int i;
    CircularIndex ci(index);
    float v = gain*x;
    float y = 0.0F;
    for (i=1, ++ci; i<a.size(); ++i, ++ci)
      v -= a[i]*u[ci];
    u[index] = v/a[0];
    for (i=0, ci=index; i<b.size(); ++i, ++ci)
      y += b[i]*u[ci];
    --index;
    return y;
  }

  std::vector<float> DigitalFilter::operator()(const std::vector<float> &x) {// Filtering a vector of samples
    std::vector<float> y;
    std::vector<float>::const_iterator ix;
    std::vector<float>::iterator iy;
    y.resize(x.size());
    for (ix=x.begin(), iy=y.begin(); ix != x.end() && iy != y.end(); ++ix, ++iy)
      *iy = (*this)(*ix);
    return y;
  }

  void DigitalFilter::operator()(std::vector<float>::const_iterator begSrc,
                                 std::vector<float>::const_iterator endSrc,
                                 std::vector<float>::iterator begDst) {
    while (begSrc != endSrc)
      *begDst++ = (*this)(*begSrc++);
  }

  float DigitalFilter::sfreqz(float freq, bool db) const {
    float m2;
    unsigned int i;
    float re = b[0], im = 0;
    for (i=1; i<b.size(); ++i) {
      re += b[i]*cos(-2*M_PI*freq*i);
      im += b[i]*sin(-2*M_PI*freq*i);
    }
    m2 = gain*gain*(re*re+im*im);

    re = a[0], im = 0;
    for (i=1; i<a.size(); ++i) {
      re += a[i]*cos(-2*M_PI*freq*i);
      im += a[i]*sin(-2*M_PI*freq*i);
    }
    m2 /= (re*re+im*im);

    if (db)
      m2 = 10.0*log10(m2);

    return m2;
  }

  std::vector<float> DigitalFilter::freqz(std::vector<float> const freq, bool db) const {
    std::vector<float>::const_iterator ifreq;
    std::vector<float> S;
    for (ifreq=freq.begin(); ifreq != freq.end(); ++ifreq) {
      S.push_back(sfreqz(*ifreq, db));
    }
    return S;
  }

  std::vector<float> DigitalFilter::freqz(unsigned int N, bool db) const {
    std::vector<float> S;
    if (N<2) return S;

    float freq=0.0F, delta_freq = 0.5F/(float)(N-1);
    unsigned int i;
    for (i=0; i<N; ++i, freq += delta_freq)
      S.push_back(sfreqz(freq, db));
    return S;
  }


}
