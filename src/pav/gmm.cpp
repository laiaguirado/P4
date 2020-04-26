/* Copyright (C) Universitat Politècnica de Catalunya, Barcelona, Spain.
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 * antonio.bonafonte@upc.edu
 * Barcelona, November 2011
 */

#include <fstream>
#include "gmm.h"

using namespace std;
using namespace upc;


namespace upc {
  void GMM::resize(size_t _nmix, size_t dim) {    

    unsigned int n = (nmix < _nmix ? nmix : _nmix); 

    vector_size=dim; nmix=_nmix;

    fvector w2 = w;
    w.resize(nmix);
    for (unsigned int i=0; i<n; ++i)
      w[i] = w2[i];

    mu.resize(nmix, vector_size);
    inv_sigma.resize(nmix, vector_size);
  }


#define CTTE_GAUSSIAN -0.9189385332046727 /* log(1 / sqrt(2 * PI)) */

  /*
   Use this function to compute log(x+y) from logx and logy:

   The basic expression,
       log(e^logx + e^logy),
   may result in 'nan', if e^logx or e^logy is too large

   Solution:
   => if logx is larger,
   log(e^logx + e^logy)=log(e^logx * (1+e^logy/e^logx)) = logx +log(1+e^(logy-logx)) 

   (same idea if logy is larger)
  */

  float add_logs(float logx, float logy) {
    if (logx > logy)
      return logx + log(1.0 + exp(logy-logx));
    else
      return logy + log(1.0 + exp(logx-logy));
  }

  float gaussian_logprob(unsigned int vector_size, const float *mu, const float *inv_sigma, const float *x) {
    float e=0.0F, c=0.0F, logp;
    unsigned int j;
    for (j=0; j<vector_size; ++j) {
      float f = (x[j]-mu[j]) * inv_sigma[j];
      e += (f*f);      
      c += log(inv_sigma[j]);
    }
    e /= 2;
    logp = vector_size * CTTE_GAUSSIAN + c - e;
    return logp;
  }

  void GMM::delete_mixture(unsigned int k) {
    if (k >= nmix)
      return;

    unsigned int last = nmix-1;

    if (k != last) {    
      //save last mixture in position k 
      w[k] = w[last];
      unsigned int i;
      for (i=0; i<vector_size; ++i) {
	mu[k][i] = mu[last][i];
	inv_sigma[k][i] = inv_sigma[last][i];
      }
    }
    resize(nmix-1, vector_size);     
  }
  

  /// Computes the logprob of a single frame of the input data.
  float GMM::gmm_logprob(const float *x) const {
    float log_prob_x,f;
    unsigned int k;
    log_prob_x = log(w[0]) + gaussian_logprob(vector_size, mu[0], inv_sigma[0], x); 

    for (k=1; k<nmix; ++k) {
      f = log(w[k]) +  gaussian_logprob(vector_size, mu[k], inv_sigma[k], x);
      log_prob_x = add_logs(log_prob_x, f);
    }
    return log_prob_x;
  }

  /// \TODO Compute the logprob for the whole input data.
  float GMM::logprob(const fmatrix &data) const {    

    if (nmix == 0 or vector_size == 0 or vector_size != data.ncol())
      return -1e38F;
    
    float lprob = 0.0;
    unsigned int n;

    for (n=0; n<data.nrow(); ++n) {
      /// \TODO Compute the logprob of a single frame of the input data; you can use gmm_logprob() above.
    }    
    return lprob/n;
  }


  int GMM::centroid(const upc::fmatrix &data) {
    if (data.nrow() == 0 or data.ncol() == 0)
      return -1;

    resize(1, data.ncol());

    unsigned int n;
    fmatrix weights(data.nrow(), 1);

    for (n=0; n < weights.nrow(); ++n)
      weights[n][0] = 1.0F;
    em_maximization(data, weights);
    return 0;
  }

  
  ///Compute the best mixtures (weights, means, variances) given
  /// -the input data
  /// -the weights that the input data is generated for each gaussian

  int GMM::em_maximization(const upc::fmatrix &data, const upc::fmatrix &weights) {
    unsigned int n, j, k;

    w.reset();
    mu.reset();
    inv_sigma.reset();

    for (n=0; n<data.nrow(); ++n) {
      for (k=0; k < nmix; ++k) {
	w[k] +=  weights[n][k];
	for (j=0; j < vector_size; ++j) {
	  mu[k][j] += weights[n][k] * data[n][j]; /* sum{x w_i} */
	  inv_sigma[k][j] += weights[n][k] * data[n][j] * data[n][j]; /* sum{x^2 w_i} */
	}
      }
    }
    for (k=0; k < nmix; ++k) {
      for (j=0; j < vector_size; ++j) {
	mu[k][j] /= w[k]; /* sum{x w_i}/sum{w_i} */
	inv_sigma[k][j] /= w[k]; /* sum{x^2 w_i}/sum{w_i} */
	inv_sigma[k][j] = 1.0F/sqrt(inv_sigma[k][j] - mu[k][j]*mu[k][j]); /* 1/sigma */
      }
      w[k] /=  data.nrow();
    }
    return 0;
  }

  ///For each input data compute the probability that the data is generated from each mixture
  ///We work with log of probabilities to avoid numerical problems on intermediate results

  float GMM::em_expectation(const fmatrix &data, fmatrix &weights) const {
    unsigned int n, k;
    float log_prob_total, log_prob_x;

    if (data.ncol() != vector_size)
      return -1.0;

    if (weights.nrow() != data.nrow() or
	weights.ncol() != nmix)
      weights.resize(data.nrow(), nmix);

    //use log(prob) for intermediate computation, to avoid underflow

    //For each input data ...
    for (n=0, log_prob_total = 0.0F; n<data.nrow(); ++n) {
      //For each mixture ...
      for (k=0, log_prob_x = -1e20F; k < nmix; ++k) {
      	weights[n][k] = log(w[k]) +  gaussian_logprob(vector_size, mu[k], inv_sigma[k], data[n]);
      	log_prob_x = add_logs(log_prob_x, weights[n][k]);
      }

      for (k=0; k < nmix; ++k)
	weights[n][k] = exp(weights[n][k]-log_prob_x);
      log_prob_total += log_prob_x;
    }

    log_prob_total /= data.nrow();
    return log_prob_total;
  }

  int GMM::em(const fmatrix &data, unsigned int max_it, float inc_threshold, int verbose) {
    unsigned int iteration;
    float old_prob=-1e34, new_prob=-1e34, inc_prob=-1e34;
    
    fmatrix weights(data.nrow(), nmix);
    for (iteration=0; iteration<max_it; ++iteration) {
      /// \TODO
	  // Complete the loop in order to perform EM, and implement the stopping criterion.
	  //
	  // EM loop: em_expectation + em_maximization.
	  //
      // Update old_prob, new_prob and inc_prob in order to stop the loop if logprob does not
      // increase more than inc_threshold.
      if (verbose & 01)
	cout << "GMM nmix=" << nmix << "\tite=" << iteration << "\tlog(prob)=" << new_prob << "\tinc=" << inc_prob << endl;
    }
    return 0;
  }

  int GMM::em_split(const fmatrix &data, unsigned int final_nmix, unsigned int max_it, float inc_threshold, int verbose) {
    centroid(data);
    while (nmix <final_nmix) {
      split(final_nmix);
      em(data, max_it, inc_threshold, verbose);
    }
    return 0;
  }

  void GMM::split_mixture(unsigned int src, unsigned int dest) {
    unsigned int j;
    int sign;
    float r;    
    for (j=0; j<vector_size; ++j) {
      r = (float) 2.0F *rand()/(float) RAND_MAX - 1.0F; /* r: (-1,1) */
      sign = (r > 0 ? 1 : -1);
      
      mu[dest][j] = mu[src][j] + sign * 0.5/inv_sigma[src][j];
      mu[src][j] = mu[src][j] - sign * 0.5/inv_sigma[src][j];
      
      inv_sigma[src][j] *= sqrt(2);
      inv_sigma[dest][j] = inv_sigma[src][j];      
    }
    w[src] /= 2.0F;
    w[dest] = w[src];
  }

  int GMM::split(unsigned int target_size) {
    unsigned int i, j, old_size;

    if (nmix >= target_size)
      return nmix;
    
    if (2*nmix <= target_size) {
      target_size = 2*nmix;
      old_size = nmix;
      resize(target_size, vector_size);
      for (i=old_size, j=0; i<nmix; ++i, ++j)
	split_mixture(j, i);
    } else {
      old_size = nmix;
      resize(target_size, vector_size);
      /* best way: select mixtures with larger variance (now, the first ones) */
      for (i=old_size, j=0; i<nmix; ++i, ++j)
	split_mixture(j, i);
    }
    return nmix;
  }

  int GMM::random_init(const upc::fmatrix &data, unsigned int nmix) {
    if (data.nrow() == 0 or data.ncol() == 0)
      return -1;
    resize(nmix, data.ncol());

    unsigned int n, k;
    fmatrix weights(data.nrow(), nmix);
    weights.reset();
    for (n=0; n < data.nrow(); ++n) {
      float r = (float) rand()/(float) RAND_MAX; /* r: [0,1] */
      k = (int) (nmix * r);
      if (k == nmix) k = nmix-1;    
      weights[n][k] = 1.0F;
    }
    em_maximization(data, weights);
    return 0;
  }

#define HEADER_SIZE 15
  static char header[HEADER_SIZE] = "UPC: GMM V 2.0";

  std::istream& GMM::read(std::istream &is) {
    char s[HEADER_SIZE];
    is.read(s, HEADER_SIZE);
    if (string(s) != string(header))
      is.setstate(ios::failbit);
    else
      is >> w >> mu >> inv_sigma;
    nmix = mu.nrow();
    vector_size = mu.ncol();
    return is;
  }
  std::ostream& GMM::write(std::ostream &os) const {
    os.write(header, HEADER_SIZE);
    os << w << mu << inv_sigma;
    return os;
  }

  std::ostream& GMM::print(std::ostream &os) const {
    unsigned int k, i;
    os << "GMM: nmix=" << nmix << "; vector_size=" << vector_size << endl;
    for (k=0; k<nmix; ++k) {
      os << "w[" << k << "]=\t" << w[k] << '\n';

      os << "mu[" << k << "]=" << mu[k][0];
      for (i=1; i<vector_size; ++i) 
	os << "\t" << mu[k][i]; 
      os << '\n';

      os << "sig[" << k << "]=" << 1/inv_sigma[k][0];
      for (i=1; i<vector_size; ++i) 
	os << "\t" << 1/inv_sigma[k][i];
      os << '\n' << endl;
      
    }
    return os;
  }
}

