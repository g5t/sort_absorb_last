#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "sort_absorb_last.h"
#define SAL_THREADS 10 // num parallel sections

long sort_absorb_offset(_class_particle * particles, long * nexts, long len, long * multiplier){

  if (multiplier != NULL) *multiplier = -1; // set default out value for multiplier

  long at_least = len / SAL_THREADS;  // every thread handles at least this many particles
  long remainder = len % SAL_THREADS; // and the first remainder threads handle one more
  if (at_least < 1 && remainder){
    // fewer than SAL_THREADS particles, ideally pass this off to a single thread algorithm
    at_least = 1;
    remainder = 0;
  }

  long good[SAL_THREADS];
  long bad[SAL_THREADS];
  long last_good[SAL_THREADS];
  long last_bad[SAL_THREADS];

#pragma acc parallel loop independent pcopyin(particles[0:len]) pcopy(nexts[0:len]) copyout(bad, good, last_bad, last_good)
  for (long thread = 0; thread < SAL_THREADS; ++thread) {
    long * cohort;
    // initialize list "pointers"
    good[thread] = bad[thread] = last_good[thread] = last_bad[thread] = len + 1;
    // identify this thread's range of particles
    long thread_start = at_least * thread + (thread < remainder ? thread : remainder);
    long thread_end = thread_start + at_least + (thread < remainder ? 1 : 0);
    for (long i = thread_start; i < thread_end && i < len; ++i) {
      int absorbed = particles[i]._absorbed;
      cohort = absorbed ? last_bad : last_good;
      if (cohort[thread] < len){
        nexts[cohort[thread]] = i; // from the head of the good/bad list next to the current node
      } else {
        // this is the first node in the list, so we need to set the head
        if (absorbed){
          bad[thread] = i;
        } else {
          good[thread] = i;
        }
      }
      // update the good/bad list pointer to the current node
      cohort[thread] = i;
      // disconnect this node from the nexts list
      nexts[i] = len + 1;
    }
  }
  // combine the segments of each list, looping the good list
  // overwrites the [0] entry of good and bad to point to a valid head node for their respective lists
  long total_good = connect_particle_nodes(good, last_good, nexts, len, SAL_THREADS, 1);
  long total_bad = connect_particle_nodes(bad, last_bad, nexts, len, SAL_THREADS, 0);

  if (!(total_good && total_bad)){
    return total_good;
  }
  if (multiplier){
    *multiplier = 1 + total_bad / total_good;
  }
  // with finite good and bad, we have work to do.
  // we will parallelize over the number of threads, each needs to fill-in total_bad / SAL_THREADS
  at_least = total_bad / SAL_THREADS;
  remainder = total_bad % SAL_THREADS;
  // move the pointers to their per-thread offsets
  for (long i=1; i<SAL_THREADS; i++) {
    long chunk = i - 1 < remainder ? at_least + 1 : at_least;
    bad[i] = particle_node_after(bad[i-1], nexts, len, chunk);
    good[i] = particle_node_after(good[i-1], nexts, len, chunk);
  }

  // overwrite the bad list with the good list
  long g;
  long b;
#pragma acc parallel loop independent pcopy(particles[0:len]) pcopyin(nexts[0:len], bad, good), private(g, b)
  for (long thread=0; thread < SAL_THREADS; thread++) {
    double randstate[6];
    long chunk = thread < remainder ? at_least + 1 : at_least;
    g=good[thread];
    b=bad[thread];
    for (long i=0; i < chunk; ++i){
      // copy the good particle to the bad list but don't copy it's randstate
      memcpy(randstate, particles[b].randstate, sizeof(double) * 6);
      particles[b] = particles[g];
      memcpy(particles[b].randstate, randstate, sizeof(double) * 6);
      // move the pointers
      b = nexts[b];
      g = nexts[g];
    }
  }

  // we overwrote any bad particles, so we return the total number, always.
  return len;
}





























