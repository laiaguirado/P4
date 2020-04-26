#include <math.h>
#include <stdlib.h>
#include "gmm.h"

/* This file implements the initialization of GMM using VQ

   The mahalanobis distance is used:
   The variance is computed from all the data and copied to all centroids
*/

using namespace upc;
using namespace std;

int GMM::vq_lbg(const fmatrix &data, unsigned int final_nmix, unsigned int max_it, float inc_threshold, int verbose) {
  centroid(data);
  global_inv_sigma.resize(vector_size);
  for (unsigned int i=0; i<vector_size; ++i)
    global_inv_sigma[i] = inv_sigma[0][i];

  while (nmix <final_nmix) {
    //    cout << "nmix: " << nmix << endl; print(cout) << endl;
    vq_split(final_nmix);
    // print(cout) << endl;
    vq_lloyd(data, max_it, inc_threshold, verbose);
    //print(cout) << "-----------------" << endl << endl;
  }
  return 0;
}

int GMM::vq_split(unsigned int target_size) {
  unsigned int i, j, old_size;

  if (nmix >= target_size)
    return nmix;

  if (2*nmix <= target_size)
    target_size = 2*nmix;
  //  else sort centroids

  old_size = nmix;
  resize(target_size, vector_size);
  for (i=old_size, j=0; i<nmix; ++i, ++j)
    vq_split_centroid(j, i);

  return nmix;
}

void GMM::vq_split_centroid(unsigned int src, unsigned int dest) {
  unsigned int j;
  float r;

  w[dest] = 0.0F;
  for (j=0; j<vector_size; ++j) {
    r = (float) 2.0F *rand()/(float) RAND_MAX - 1.0F; /* r: (-1,1) */
    mu[dest][j] = mu[src][j] * (1 + 1e-2*r);
    //    mu[src][j] = mu[src][j] * (1 - 1e-2*r);
    inv_sigma[dest][j] = inv_sigma[src][j];
  }
}


#define MIN_MEMBERS 2.5

float GMM::vq_distance(const float *x, const float *y) const {
  float d, f;
  unsigned int j;

  for (j=0, d=0.0F; j<vector_size; ++j) {
    f = (x[j]-y[j]) * global_inv_sigma[j];
    d += f*f;
  }
  d /= vector_size;
  return d;
}


/*
  int sort_centroids(const void *_mix1, const void *_mix2) {
  const Mixture *mix1 = (const Mixture *) _mix1;
  const Mixture *mix2 = (const Mixture *) _mix2;
  if (mix1->w < mix2->w)
  return 1;
  else if (mix2->w < mix1->w)
  return -1;
  else
  return 0;
  }
*/


float GMM::vq(const float *x, int &index) const {

  unsigned int k;
  float d, dist_x;

  for (k=0, dist_x = 1e30F; k < nmix; ++k) {
    d = vq_distance(x, mu[k]);
    //    cout << '\t' << d;
    if (d <= dist_x) {
      index = k;
      dist_x = d;
    }
  }
  //  cout << "\t[" << index << "]\n";
  return dist_x;
}


float GMM::vq_expectation(const fmatrix &data, ivector &indexes) const {

  unsigned int n;
  float dist_total, dist_x;

  if (data.ncol() != vector_size)
    return -1.0;

  for (n=0, dist_total = 0.0F; n < data.nrow(); ++n) {
    dist_x = vq(data[n], indexes[n]);
    dist_total += dist_x;
    //    cout << n << '\t' << indexes[n] << '\t' << dist_x << endl;
  }
  dist_total /= data.nrow();
  return dist_total;
}

int GMM::vq_maximization(const fmatrix &data, const ivector &indexes, bool update_variances) {
  unsigned int n, j, k, index;

  w.reset();
  mu.reset();
  if (update_variances)
    inv_sigma.reset();

  for (n=0; n<data.nrow(); ++n) {
    index = indexes[n];
    w[index] += 1.0F;
    for (j=0; j < vector_size; ++j) {
      mu[index][j] += data[n][j]; /* sum{x} */
      if (update_variances)
        inv_sigma[index][j] += data[n][j] * data[n][j]; /* sum{x^2} */
    }
  }

  /* Delete empty cells */
  for (k=0; k < nmix;) {
    if (w[k] < MIN_MEMBERS)
      delete_mixture(k);
    else
      k++;
  }

  for (k=0; k < nmix; ++k) {
    for (j=0; j < vector_size; ++j) {
      mu[k][j] /= w[k]; /* sum{x}/N */
      if (update_variances) {
        inv_sigma[k][j] /= w[k]; /* sum{x^2}/N */
        inv_sigma[k][j] = sqrt(inv_sigma[k][j] - mu[k][j]*mu[k][j]); /* sigma */
        if (inv_sigma[k][j] < 1e-9)
          inv_sigma[k][j] = 1e-9;
        inv_sigma[k][j] =  1.0/inv_sigma[k][j];
      }
    }
    w[k] /=  data.nrow();
  }
  return 0;
}


int GMM::vq_lloyd(const fmatrix &data, unsigned int max_it, float inc_threshold, int verbose) {

  unsigned int iteration = 0;
  float old_dist, new_dist, inc=1.0F;
  ivector indexes(data.nrow());

  while (1) {
    new_dist = vq_expectation(data, indexes);
    //    cout << "MAXIM. \n";
    //    print(cout) << endl << endl;

    if (iteration > 0)
      inc = (old_dist-new_dist)/old_dist;

    if (verbose & 01)
      cout << "VQ  ncen=" << nmix << "\tite=" << iteration << "\tdist=" << new_dist << "\tinc=" << 100*inc << "%\n";

    if (((iteration > 0) and (inc < inc_threshold)) or (++iteration == max_it)) {
      vq_maximization(data, indexes, true);
      break;
    } else {
      vq_maximization(data, indexes, false);
      old_dist = new_dist;
    }
  }
  return 0;
}
