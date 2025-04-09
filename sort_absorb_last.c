#include <math.h>
#include <memory.h>
#include "sort_absorb_last.h"
//
//long sort_absorb_last(_class_particle * particles, long p_len, _class_particle * buffer, long b_len, long split, long* multiplier) {
//#define SAL_THREADS 10 // num parallel sections
//
//  if (multiplier != NULL) *multiplier = -1; // set default out value for multiplier
//  long newlen = 0;
//  long los[SAL_THREADS]; // target array startidxs
//  long lens[SAL_THREADS]; // target array sublens
//  long l = floor(p_len/(SAL_THREADS-1)); // subproblem_len
//  long ll = p_len - l*(SAL_THREADS-1); // last_subproblem_len
//
//  // TODO: The l vs ll is too simplistic, since ll can become much larger
//  // than l, resulting in idling. We should distribute lengths more evenly.
//
//  // step 1: sort sub-arrays
//#pragma acc parallel loop present(particles[0:b_len], buffer[0:b_len])
//  for (long tidx=0; tidx<SAL_THREADS; tidx++) {
//    long lo = l*tidx;
//    long loclen = l;
//    if (tidx==(SAL_THREADS-1)) loclen = ll; // last sub-problem special case
//    long i = lo;
//    long j = lo + loclen - 1;
//
//    // write into buffer at i and j
//#pragma acc loop seq
//    while (i < j) {
//#pragma acc loop seq
//      while (!particles[i]._absorbed && i<j) {
//        buffer[i] = particles[i];
//        i++;
//      }
//#pragma acc loop seq
//      while (particles[j]._absorbed && i<j) {
//        buffer[j] = particles[j];
//        j--;
//      }
//      if (i < j) {
//        buffer[j] = particles[i];
//        buffer[i] = particles[j];
//        i++;
//        j--;
//      }
//    }
//    // transfer edge case
//    if (i==j)
//      buffer[i] = particles[i];
//
//    lens[tidx] = i - lo;
//    if (i==j && !particles[i]._absorbed) lens[tidx]++;
//  }
//
//  // determine lo's
//  long accumlen = 0;
//#pragma acc loop seq
//  for (long idx=0; idx<SAL_THREADS; idx++) {
//    los[idx] = accumlen;
//    accumlen = accumlen + lens[idx];
//  }
//
//  // step 2: write non-absorbed sub-arrays to psorted/output from the left
//#pragma acc parallel loop present(buffer[0:b_len])
//  for (long tidx=0; tidx<SAL_THREADS; tidx++) {
//    long j, k;
//#pragma acc loop seq
//    for (long i=0; i<lens[tidx]; i++) {
//      j = i + l*tidx;
//      k = i + los[tidx];
//      particles[k] = buffer[j];
//    }
//  }
//  //for (int ii=0;ii<accumlen;ii++) printf("%ld ", (psorted[ii]->_absorbed));
//
//  // return (no SPLIT)
//  if (split != 1)
//    return accumlen;
//
//  // SPLIT - repeat the non-absorbed block N-1 times, where len % accumlen = N + R
//  long mult = b_len / accumlen; // TODO: possibly use a new arg, bufferlen, rather than len
//
//  // not enough space for full-block split, return
//  if (mult <= 1)
//    return accumlen;
//
//  // copy non-absorbed block
//#pragma acc parallel loop present(particles[0:b_len])
//  for (long tidx = 0; tidx < accumlen; tidx++) { // tidx: thread index
//    _class_particle sourcebuffer;
//    _class_particle targetbuffer;
//    // assign reduced weight to all particles
//    particles[tidx].p = particles[tidx].p / (double) mult;
//#pragma acc loop seq
//    for (long bidx = 1; bidx < mult; bidx++) { // bidx: block index
//      // preserve absorbed particle (for randstate)
//      sourcebuffer = particles[bidx*accumlen + tidx];
//      // buffer full particle struct
//      targetbuffer = particles[tidx];
//      // reassign previous randstate
//      targetbuffer.randstate[0] = sourcebuffer.randstate[0];
//      targetbuffer.randstate[1] = sourcebuffer.randstate[1];
//      targetbuffer.randstate[2] = sourcebuffer.randstate[2];
//      targetbuffer.randstate[3] = sourcebuffer.randstate[3];
//      targetbuffer.randstate[4] = sourcebuffer.randstate[4];
//      targetbuffer.randstate[5] = sourcebuffer.randstate[5];
//      targetbuffer.randstate[6] = sourcebuffer.randstate[6];
//      // apply
//      particles[bidx*accumlen + tidx] = targetbuffer;
//    }
//  }
//
//  // set out split multiplier value
//  if (multiplier) *multiplier = mult;
//
//  // return expanded array size
//  return accumlen * mult;
//}


long sort_absorb_last(_class_particle * particles, long len, _class_particle * pbuffer, long buffer_len, long flag_split, long* multiplier) {
#define SAL_THREADS 10 // num parallel sections

  if (multiplier != NULL) *multiplier = -1; // set default out value for multiplier

  long target_start_index[SAL_THREADS]; // target array startidxs
  long target_sub_length[SAL_THREADS]; // target array sublens

  long at_least = len / SAL_THREADS;  // every thread handles at least this many particles
  long remainder = len % SAL_THREADS; // and the first remainder threads handle one more

  // step 1: sort sub-arrays
#pragma acc parallel loop present(particles[0:b_len], buffer[0:b_len])
  for (long thread=0; thread < SAL_THREADS; thread++) {
    long chunk = thread < remainder ? at_least + 1 : at_least;
    long first = (chunk * thread) + (thread < remainder ? 0 : remainder);
    long i = first, j = first + chunk - 1;

    // write into pbuffer at i and j
#pragma acc loop seq
    while (i < j) {
#pragma acc loop seq
      while (!particles[i]._absorbed && i<j) {
        pbuffer[i] = particles[i];
        i++;
      }
#pragma acc loop seq
      while (particles[j]._absorbed && i<j) {
        pbuffer[j] = particles[j];
        j--;
      }
      if (i < j) {
        pbuffer[j] = particles[i];
        pbuffer[i] = particles[j];
        i++;
        j--;
      }
    }
    target_sub_length[thread] = i - first;

    // transfer edge case
    if (i==j) {
      pbuffer[i] = particles[i];
      // and handle the case where the last particle is non-absorbed
      if (!particles[i]._absorbed) {
        target_sub_length[thread]++;
      }
    }
  }

  // determine lo's
  long moved = 0;
  // FIXME should this cumulative sum really be a seq loop? It only loops over the number of threads.
#pragma acc loop seq
  for (long idx=0; idx<SAL_THREADS; idx++) {
    target_start_index[idx] = moved;
    // TOOD can this not be moved += target_sub_length[idx]?
    moved = moved + target_sub_length[idx];
  }

  // step 2: write non-absorbed sub-arrays to psorted/output from the left
  // FIXME can this gather be parallelized efficiently?
#pragma acc parallel loop present(buffer[0:b_len])
  for (long thread=0; thread < SAL_THREADS; thread++) {
    long j = thread < remainder ? (at_least + 1) * thread : at_least * thread + remainder;
    long k = target_start_index[thread];
#pragma acc loop seq
    for (long i=0; i < target_sub_length[thread]; i++) {
      particles[k + i] = pbuffer[j + i];
    }
  }

  // SPLIT - repeat the non-absorbed block N-1 times, where len % moved = N + R
  long mult = buffer_len / moved; // TODO: possibly use a new arg, bufferlen, rather than len

  // no flag_split or not enough space for full-block flag_split, return
  if (flag_split != 1 || mult <= 1) {
    return moved;
  }

  // copy non-absorbed block
#pragma acc parallel loop present(particles[0:b_len])
  for (long thread = 0; thread < moved; thread++) { // thread: thread index
    // assign reduced weight to all particles
    particles[thread].p = particles[thread].p / (double) mult;
    // copy this particle to replicate (mult-1) times, preserving randstate
    _class_particle source = particles[thread];
#pragma acc loop seq
    for (long bidx = 1; bidx < mult; bidx++) { // bidx: block index
      long i = bidx * moved + thread; // this strided access can't be good for performance
      // reassign previous randstate
      source.randstate[0] = particles[i].randstate[0];
      source.randstate[1] = particles[i].randstate[1];
      source.randstate[2] = particles[i].randstate[2];
      source.randstate[3] = particles[i].randstate[3];
      source.randstate[4] = particles[i].randstate[4];
      source.randstate[5] = particles[i].randstate[5];
      source.randstate[6] = particles[i].randstate[6];
      // copy the not-absorbed particle with the absorbed-particle's random state
      // FIXME: Since no consideration is given to whether the destination was a moved non-absorbed particle,
      //        there will be particles with the same random state in the output.
      particles[i] = source;
    }
  }

  // set out flag_split multiplier value
  if (multiplier) *multiplier = mult;

  // return expanded array size
  return moved * mult;
}