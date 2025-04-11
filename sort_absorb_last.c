#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "sort_absorb_last.h"


long sort_absorb_list(particle_node * input, long len, long * multiplier){
#define SAL_THREADS 3 // num parallel sections

  if (multiplier != NULL) *multiplier = -1; // set default out value for multiplier

  long at_least = len / SAL_THREADS;  // every thread handles at least this many particles
  long remainder = len % SAL_THREADS; // and the first remainder threads handle one more

  particle_node * good[SAL_THREADS];
  particle_node * bad[SAL_THREADS];
  particle_node * start[SAL_THREADS];

  // identify per-thread starting points before sorting, since this method breaks once ->next is modified
  for (long thread = 0; thread < SAL_THREADS; ++thread) {
    long chunk = thread <= remainder ? at_least + 1 : at_least;
    long first = (chunk * thread) + (thread <= remainder ? 0 : remainder);
    start[thread] = particle_list_after(input, first); // input + first
    good[thread] = NULL;
    bad[thread] = NULL;
  }

  for (long thread = 0; thread < SAL_THREADS; ++thread){
    long chunk = thread <= remainder ? at_least + 1 : at_least;
    particle_node * node = start[thread];
    if (node == NULL || node->this == NULL) continue;
    // ensure the ->prev link in removed
    node->prev = NULL;
    particle_node * move;
    for (long i = 0; i < chunk; ++i){
      if (node == NULL || node->this == NULL) break;
      int absorbed = node->this->_absorbed;
      move = absorbed ? bad[thread] : good[thread];
      if (move != NULL){
        // add to the end of the list
        move->next = node;
        node->prev = move;
      } else {
        // this is the first node in the good/bad list
        node->prev = NULL;
      }
      if (absorbed){
        bad[thread] = node;
      } else {
        good[thread] = node;
      }
      // store the last node in the good/bad list
      move = node;
      // move the pointer for the input list
      node = node->next;
      // unlink the good/bad list end node
      move->next = NULL;
    }
    // go back to the start of this thread's list
    good[thread] = particle_list_rewind(good[thread]);
    bad[thread] = particle_list_rewind(bad[thread]);
  }
  // identify the first thread to have found a good particle
  // TODO Since we're modifying the pointers in the input list, we should not create a loop
  //      otherwise we may have trouble 'straightening' the list later.
  long total_good = connect_particle_lists(good, SAL_THREADS, 1); // *do not* loop the good list
  long total_bad = connect_particle_lists(bad, SAL_THREADS, 0); // don't loop the bad list

  if (total_good && total_bad){
    // with finite good and bad, we have work to do.
    // parallelize over the number of threads, each needs to fill-in total_bad / SAL_THREADS
    at_least = total_bad / SAL_THREADS;
    remainder = total_bad % SAL_THREADS;
    // move the pointers to their per-thread offsets
    for (long i=1; i<SAL_THREADS; i++) {
      bad[i] = particle_list_after(bad[i-1], i <= remainder ? at_least + 1 : at_least);
      // good might loop-around, this is fine.
      good[i] = particle_list_after(good[i-1], i <= remainder ? at_least + 1 : at_least);
    }
    // overwrite the bad list with the good list
#pragma acc parallel loop present(input[0:len], bad[0:SAL_THREADS], good[0:SAL_THREADS])
    for (long thread=0; thread < SAL_THREADS; thread++) {
      for (long i=0; i < (thread <= remainder ? at_least + 1 : at_least); ++i){
        particle_node_copy(bad[thread], good[thread], 0); // don't copy the rand state from good to bad
        bad[thread] = bad[thread]->next;
        good[thread] = good[thread]->next;
      }
    }
  }

  if (total_good && multiplier){
    // if we have good particles, we need to return the multiplier
    // which is how much we've scaled up the good particles
    *multiplier = 1 + total_bad / total_good;
  }

  // if there were any good particles, we have ensured there are len good ones output
  return total_good ? len : 0;
}

long chunk_size(long len, long threads, long thread){
  long at_least = len / threads;  // every thread handles at least this many particles
  long remainder = len % threads; // and the first remainder threads handle one more
  return thread < remainder ? at_least + 1 : at_least;
}

long chunk_start(long len, long threads, long thread){
  long at_least = len / threads;  // every thread handles at least this many particles
  long remainder = len % threads; // and the first remainder threads handle one more
  return thread < remainder ? (at_least + 1) * thread : (at_least + 1) * remainder + at_least * (thread - remainder);
}


long sort_absorb_offset(_class_particle * particles, long * nexts, long len, long * multiplier){
#define SAL_THREADS 3 // num parallel sections

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
    printf("thread=%ld, first_index=%ld\n", thread, first_index[thread]);
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

  printf("total_good=%ld, total_bad=%ld\n", total_good, total_bad);

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
#pragma acc parallel loop present(particles[0:len], offsets[0:len], bad[0:SAL_THREADS], good[0:SAL_THREADS])
  for (long thread=0; thread < SAL_THREADS; thread++) {
    long chunk = thread < remainder ? at_least + 1 : at_least;
    for (long i=0; i < chunk; ++i){
      // copy the good particle to the bad list but don't copy it's randstate
      long g = good[thread];
      long b = bad[thread];
      double randstate[6] = {
          particles[b].randstate[0],
          particles[b].randstate[1],
          particles[b].randstate[2],
          particles[b].randstate[3],
          particles[b].randstate[4],
          particles[b].randstate[5]
      };

      particles[b] = particles[g];
      particles[b].randstate[0] = randstate[0];
      particles[b].randstate[1] = randstate[1];
      particles[b].randstate[2] = randstate[2];
      particles[b].randstate[3] = randstate[3];
      particles[b].randstate[4] = randstate[4];
      particles[b].randstate[5] = -(double)thread; // randstate[5];

      // move the pointers
      bad[thread] = nexts[b];
      good[thread] = nexts[g];
    }
  }

  // we overwrote any bad particles, so we return the total number, always.
  return len;
}















































