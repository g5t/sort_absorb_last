#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "sort_absorb_last.h"


long sort_absorb_last(_class_particle * particles, long len, _class_particle * pbuffer, long buffer_len, long flag_split, long* multiplier) {
#define SAL_THREADS 3 // num parallel sections

  if (multiplier != NULL) *multiplier = -1; // set default out value for multiplier

  long at_least = len / SAL_THREADS;  // every thread handles at least this many particles
  long remainder = len % SAL_THREADS; // and the first remainder threads handle one more

  particle_node * good[SAL_THREADS];
  particle_node * bad[SAL_THREADS];

  for (long i=0; i<SAL_THREADS; i++) {
    good[i] = calloc(1, sizeof(particle_node));
    good[i]->this = NULL;
    good[i]->prev = NULL;
    good[i]->next = NULL;
    bad[i] = calloc(1, sizeof(particle_node));
    bad[i]->this = NULL;
    bad[i]->prev = NULL;
    bad[i]->next = NULL;
  }

  // divide the particles into two linked lists: absorbed and not-absorbed
#pragma acc parallel loop present(particles[0:b_len], buffer[0:b_len])
  for (long thread=0; thread < SAL_THREADS; thread++) {

    long chunk = thread <= remainder ? at_least + 1 : at_least;
    long first = (chunk * thread) + (thread <= remainder ? 0 : remainder);
    long i = first, j = first + chunk;

    for (long k=i; k<j; k++) {
      if (particles[k]._absorbed) {
        // add to absorbed list
        bad[thread]->this = particles + k;
        bad[thread]->next = calloc(1, sizeof(particle_node *));
        bad[thread]->next->prev = bad[thread];
        bad[thread] = bad[thread]->next;
      } else {
        // add to absorbed list
        good[thread]->this = particles + k;
        good[thread]->next = calloc(1, sizeof(particle_node *));
        good[thread]->next->prev = good[thread];
        good[thread] = good[thread]->next;
      }
    }
    // go back to the start of this thread's list
    good[thread] = particle_list_rewind(good[thread]);
    bad[thread] = particle_list_rewind(bad[thread]);
  }
  // identify the first thread to have found a good particle
  long total_good = connect_particle_lists(good, SAL_THREADS, 1); // loop the good list
  long total_bad = connect_particle_lists(bad, SAL_THREADS, 0); // don't loop the bad list

  printf("Total good: %ld, Total bad: %ld\n(bad ones:)\n", total_good, total_bad);
  print_particle_list(bad[0]);
  printf("\n");

  if (total_good && total_bad){
    // with finite good and bad, we have work to do.
    // parallelize over the number of threads, each needs to fill-in total_bad / SAL_THREADS
    at_least = total_bad / SAL_THREADS;
    remainder = total_bad % SAL_THREADS;
    // move the pointers to their per-thread offsets
    for (long i=1; i<SAL_THREADS; i++) {
      bad[i] = particle_list_after(bad[i-1], i < remainder ? at_least + 1 : at_least);
      // good might loop-around, this is fine.
      good[i] = particle_list_after(good[i-1], i < remainder ? at_least + 1 : at_least);
    }
    // overwrite the bad list with the good list
#pragma acc parallel loop present(bad[0:SAL_THREADS, good[0:SAL_THREADS])
    for (long thread=0; thread < SAL_THREADS; thread++) {
      for (long i=0; i < (thread <= remainder ? at_least + 1 : at_least); ++i){
        particle_node_copy(bad[thread], good[thread], 0); // don't copy the rand state from good to bad
        bad[thread] = bad[thread]->next;
        good[thread] = good[thread]->next;
      }
    }
  }
  // the first entry in good and bad point to a node which is connected to all nodes of that type,
  // so freeing only that node's list will free the whole thing.
  particle_list_free(good[0], total_good, 0); // don't warn due to looped list
  particle_list_free(bad[0], total_bad, 1);

  if (!total_good || !total_bad) {
    // no particles are good _or_ none are bad, return since we can't split
    return total_bad ? 0 : len;
  }
  return len;
}


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
  }

  for (long thread = 0; thread < SAL_THREADS; ++thread){
    long chunk = thread <= remainder ? at_least + 1 : at_least;
    particle_node * node = start[thread];
    // ensure the ->prev link in removed
    node->prev = NULL;
    good[thread] = NULL;
    bad[thread] = NULL;
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