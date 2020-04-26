/* Copyright (C) Antonio Bonafonte, 2012
 *               Universitat Politecnica de Catalunya, Barcelona, Spain.
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */

#ifndef UPC_FILTER
#define UPC_FILTER
#include <vector>

namespace upc {
///Circular Index used to go through state circular buffer (u)

/**
  The circular index is a simple class to address a circular buffer. 
  Once the buffer size is defined, you can add or substract and the index
  remains inside the buffer. A casting operator to int is provided.

    Example:
~~~~~~~~~~~~~~~{.c}
    float x[8]={0,1,2,4,5,6,7};
    CircularIndex ci(8);
    int n=10; float z=0.0F;
    for (n=0; n< 10; ++n, ++ci)
        z += x[ci+1] - x[ci-1];
~~~~~~~~~~~~~~~
  Operations +, +=, ++, -, -=, -- are defined
**/
  class CircularIndex {
    int index, ///the index (value) 
      size; ///buffer size; index goes from 0 ... size-1
  public:
  CircularIndex(int s=1): index(0), size(s) {} ///< default: size=1; not very useful (index can only be 0)
  CircularIndex(CircularIndex &ci): index(ci.index), size(ci.size) {}///< constructor defining size based on other CircularIndex
    void resize(int s) {size=s; index=0;} ///<Set size of circular index
    CircularIndex & operator++() {if (++index == size) index = 0; return *this;}///<Increment
    CircularIndex & operator+=(int i) {index += i; while (index >= size) index -= size; return *this;}///<Increment 'i' positions
    CircularIndex & operator+(int i) {return CircularIndex(*this) += i;}///<Increment 'i' positions
    CircularIndex & operator--() {if (index == 0) index = size; --index; return *this;}///<Decrement
    CircularIndex & operator-=(int i) {index -= i; while (index < 0) index += size; return *this;}///<Decrement 'i' positions
    CircularIndex & operator-(int i) {return CircularIndex(*this) -= i;}///<Decrement 'i' positions
    operator int() {return index;}///!Casting from CircularIndex to int, returning int.
    CircularIndex & operator= (const CircularIndex &ci) {index = ci.index; size = ci.size; return *this;}///!Assignment operator.
  };


  /**
     This class implements digital filtering.
     The filter is stored as rational funcion H(z) = g B(z)/A(z)
     Being 
     g: gain
     B(z) = b0 +b1 z^-1 + b2 z^-2 ... bm z^-m 
     A(z) = a0 +a1 z^-1 + a2 z^-2 ... an z^-n 

     The state conditions are saved after each call to the filter.
     The filter is implemented using the direct form.
     Y(z) = B(z)/A(z) X(z) => U(z) = X(z)/A(z); Y(z) = B(z) U(z)
     State conditions: u[n]

     The state conditions are only cleared either:
         - explicitely calling clean() 
         - by a change in filter coeficients that imply an order filter

     The filters coefficients are set using set_a(), set_b(), set_gain().
     An alternative convinient method exist for 2nd order resonators, 
     using the central frequency and the resonator bandwidth: set_resonator()     
   **/

  ///Digital filter implemented using direct form.
  class DigitalFilter {
    std::vector<float> a; ///<Denominator A(z) = a0+a1 z^-1 + a2 z^-2 ... an z^-n)
    std::vector<float> b; ///<Numerator B(z) = b0+b1 z^-1 + b2 z^-2 ... bm z^-m)
    std::vector<float> u; ///<State conditions, implemented as circular buffer; call "clear()" to reset them.
    float gain; ///additional gain factor
    CircularIndex index;  ///<Used to avoid simplify access to circular buffer 'u' for state conditions
    void prepare_state(); ///<Allocate memory for stated conditions.

  public:
  DigitalFilter(const std::vector<float> &_a, const std::vector<float> &_b, float g=1.0F) :
    a(_a), b(_b), gain(g) {prepare_state();} ///<Create filter
    DigitalFilter() {a.push_back(1.0); b.push_back(1.0); gain = 1.0F;  prepare_state();}///<create void filter H(z) = 1
    void set_a(std::vector<float> const &A) {a = A; prepare_state();}///<Change denominator (state conditions only cleaned if order changes)
    void set_b(const std::vector<float> &B) {b = B; prepare_state();}///<Change denominator (state conditions only cleaned if order changes)
  DigitalFilter(const DigitalFilter &f): a(f.a), b(f.b), gain(f.gain) {prepare_state();} ///<Create filter
    ///Asign operator (copy a filter from other)
    DigitalFilter & operator=(const DigitalFilter &f) {
      a = f.a; b = f.b; gain =f.gain; prepare_state(); return *this;
    }
    ///Second order AR band-pass filter defined by central frequency and bandwidth
    void set_resonator(float norm_central_freq, float norm_bandwidth);
    void set_gain(float g) {gain=g;}
    void clear() {u.clear();} ///<clean state conditions
    float operator()(float x); ///<Filter one sample
    std::vector<float> operator()(const std::vector<float> &x); ///<Filter a vector of samples
    void operator()(std::vector<float>::const_iterator begSrc, 
		    std::vector<float>::const_iterator endSrc, 
		    std::vector<float>::iterator begDst); ///<Filter several samples, from begSrc to endSrc, and save at begDst and following positions
    /** Get freq. response of filter in the given frequencies (freq). sfreqz() is called for each given frequency 
     **/
    std::vector<float> freqz(std::vector<float> const freq, bool db=true) const; 
    /** Get freq. response of filter in N discrite frequencies, from 0 to 0.5. sfreqz() is called for each given frequency **/
    std::vector<float> freqz(unsigned int N, bool db=true) const;

    /** Get freq. response of filter in the given frequency (freq). freq is the digital frequency, f/fsampling, with range in [0,0.5]
        If db is true, provide the result in dB: 20log(|H(e^j2pif)|; Otherwise, the results is the squared module, |H|^2 **/ 
    float sfreqz(float freq, bool db=true) const;
  };
}
#endif
