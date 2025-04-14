#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "sort_absorb_last.h"
#define SAL_THREADS 10 // num parallel sections


long chunk_start(long len, long threads, long thread){
  long at_least = len / threads;  // every thread handles at least this many particles
  long remainder = len % threads; // and the first remainder threads handle one more
  return thread < remainder ? (at_least + 1) * thread : (at_least + 1) * remainder + at_least * (thread - remainder);
}


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

  long first_index[SAL_THREADS];
  long first_good[SAL_THREADS];
  long first_bad[SAL_THREADS];

  // identify per-thread starting points before sorting, since this method breaks once ->next is modified
  for (long thread = 0; thread < SAL_THREADS; ++thread) {;
//    first_index[thread] = (at_least + 1) * thread - (thread > remainder ? remainder : 0);
    first_index[thread] = chunk_start(len, SAL_THREADS, thread);
    first_good[thread] = len + 1;
    first_bad[thread] = len + 1;
    good[thread] = len + 1;
    bad[thread] = len + 1;
  }

  for (long thread = 0; thread < SAL_THREADS; ++thread) {
    long * cohort;
    long thread_end = chunk_start(len, SAL_THREADS, thread + 1);
    for (long i = first_index[thread]; i < thread_end && i < len; ++i) {
      int absorbed = particles[i]._absorbed;
      cohort = absorbed ? bad : good;
      if (cohort[thread] < len){
        nexts[cohort[thread]] = i; // from the head of the good/bad list next to the current node
      } else {
        if (absorbed){
          first_bad[thread] = i;
        } else {
          first_good[thread] = i;
        }
      }
      // update the good/bad list pointer to the current node
      cohort[thread] = i;
      // disconnect this node from the nexts list
      nexts[i] = len + 1;
    }
  }
  // move the good/bad pointers back to their first entries:
  for (long thread = 0; thread < SAL_THREADS; ++thread) {
    good[thread] = first_good[thread];
    bad[thread] = first_bad[thread];
  }
  long total_good = connect_particle_nodes(good, nexts, len, SAL_THREADS, 1);
  long total_bad = connect_particle_nodes(bad, nexts, len, SAL_THREADS, 0);


  if (!(total_good && total_bad)){
    return total_good;
  }
  if (multiplier){
    *multiplier = 1 + total_bad / total_good;
  }
  // with finite good and bad, we have work to do.
  // parallelize over the number of threads, each needs to fill-in total_bad / SAL_THREADS
  at_least = total_bad / SAL_THREADS;
  remainder = total_bad % SAL_THREADS;
  // move the pointers to their per-thread offsets
  for (long i=1; i<SAL_THREADS; i++) {
    long chunk = i - 1 < remainder ? at_least + 1 : at_least;
    bad[i] = particle_node_after(bad[i-1], nexts, len, chunk);
    good[i] = particle_node_after(good[i-1], nexts, len, chunk);
  }

  // overwrite the bad list with the good list
#pragma acc parallel loop present(particles[0:len], bad[0:SAL_THREADS], good[0:SAL_THREADS])
  for (long thread=0; thread < SAL_THREADS; thread++) {
    double randstate[6];
    long chunk = thread < remainder ? at_least + 1 : at_least;
    for (long i=0; i < chunk; ++i){
      // copy the good particle to the bad list but don't copy it's randstate
      memcpy(randstate, particles[bad[thread]].randstate, sizeof(double) * 6);
      particles[bad[thread]] = particles[good[thread]];
      memcpy(particles[bad[thread]].randstate, randstate, sizeof(double) * 6);
      // move the pointers
      bad[thread] = nexts[bad[thread]];
      good[thread] = nexts[good[thread]];
    }
  }

  // we overwrote any bad particles, so we return the total number, always.
  return len;
}



long separate_soa(_class_particle_soa * particles, long * left_start, long * left_total, long * right_start, long * right_total, int connect_left, int connect_right){

  long left[SAL_THREADS];
  long right[SAL_THREADS];
  long len = particles->n_particles;
  long * next = particles->_next;

//#pragma acc parallel loop present(next[0:len], is_right[0:len], left[0:SAL_THREADS], right[0:SAL_THREADS])
  for (long thread=0; thread < SAL_THREADS; ++thread){
    left[thread] = len + 1;
    right[thread] = len + 1;
    long left_head = len + 1;
    long right_head = len + 1;
    long start = chunk_start(len, SAL_THREADS, thread);
    long end = chunk_start(len, SAL_THREADS, thread + 1);
    for (long i=start; i < end && i < len; ++i){
      int isr = particles->_ints[6*i+1];
      if ((isr ? right_head : left_head) < len) {
        next[isr ? right_head : left_head] = i; // from the head of the left|right list next points to the current index
      } else {
        if (isr){ right[thread] = i; } else { left[thread] = i; }
      }
      // move the left or right head to be this particle
      if (isr){ right_head = i; } else { left_head = i; }
      next[i] = len + 1; // disconnect this particle from the next list
    }
  }

  long tl = connect_particle_nodes(left, next, len, SAL_THREADS, connect_left);
  long tr = connect_particle_nodes(right, next, len, SAL_THREADS, connect_right);

  if (left_total) *left_total = tl;
  if (right_total) *right_total = tr;
  if (left_start) *left_start = left[0];
  if (right_start) *right_start = right[0];
  return tl + tr;
}


long sort_absorb_soa(_class_particle_soa * particles, long * multiplier){
  if (multiplier != NULL) *multiplier = -1; // set default out value for multiplier
  long left_start, right_start, left_total, right_total;
  long total = separate_soa(particles, &left_start, &left_total, &right_start, &right_total, 1, 0);
  //long total = separate_soa_single(particles, &left_start, &left_total, &right_start, &right_total, 1, 0);
  if (!total){
    return right_total;
  }
  if (multiplier){
    *multiplier = 1 + right_total / left_total;
  }
  long len = particles->n_particles;
  // with finite good and bad, we have work to do.
  // parallelize over the number of threads, each needs to fill-in total_bad / SAL_THREADS
  long at_least = right_total / SAL_THREADS;  // every thread handles at least this many particles
  long remainder = right_total % SAL_THREADS; // and the first remainder threads handle one more
  long left[SAL_THREADS];
  long right[SAL_THREADS];
  left[0] = left_start;
  right[0] = right_start;
  // move the pointers to their per-thread offsets
  for (long i=1; i<SAL_THREADS; i++) {
    long chunk = i - 1 < remainder ? at_least + 1 : at_least;
    left[i] = particle_node_after(left[i-1], particles->_next, len, chunk);
    right[i] = particle_node_after(right[i-1], particles->_next, len, chunk);
  }

  // overwrite the right list with the left list
//#pragma acc parallel loop present(particles[0:len], left[0:SAL_THREADS], right[0:SAL_THREADS])
  for (long thread=0; thread < SAL_THREADS; ++thread){
    long chunk = thread < remainder ? at_least + 1 : at_least;
    for (long i=0; i < chunk; ++i){
      // copy the good particle to the bad list but don't copy it's randstate
      particle_soa_duplicate_one(particles, right[thread], particles, left[thread], 0, 0);
      // move the pointers
      right[thread] = particles->_next[right[thread]];
      left[thread] = particles->_next[left[thread]];
    }
  }
  // we overwrote any bad particles, so we return the total number, always.
  return len;
}





long separate_soa_single(_class_particle_soa * particles, long * left_start, long * left_total, long * right_start, long * right_total, int connect_left, int connect_right){
  long len = particles->n_particles;
  long * next = particles->_next;

  long left = len + 1;
  long right = len + 1;
  long left_head = len + 1;
  long right_head = len + 1;
  for (long i=0; i < len; ++i){
    int isr = particles->_ints[6*i+1];
    if ((isr ? right_head : left_head) < len) {
      next[isr ? right_head : left_head] = i; // from the head of the left|right list next points to the current index
    } else {
      if (isr){ right = i; } else { left = i; }
    }
    // move the left or right head to be this particle
    if (isr){ right_head = i; } else { left_head = i; }
    next[i] = len + 1; // disconnect this particle from the next list
  }

  // FIXME since there is only one worker and we don't really care about tl vs tr, this can be simplified.
  long tl = connect_particle_nodes(&left, next, len, 1, connect_left);
  long tr = connect_particle_nodes(&right, next, len, 1, connect_right);

  if (left_total) *left_total = tl;
  if (right_total) *right_total = tr;
  if (left_start) *left_start = left;
  if (right_start) *right_start = right;
  return tl + tr;
}
































