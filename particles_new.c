#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "particles.h"

_class_particle_soa * particle_soa_alloc(long n_particles){
  if (n_particles <= 0) {
    fprintf(stderr, "Error: particle_soa_alloc: n_particles must be > 0\n");
    return NULL;
  }
  _class_particle_soa * particles = calloc(1, sizeof(_class_particle_soa));
  if (particles == NULL) {
    fprintf(stderr, "Error: particle_soa_alloc: could not allocate memory for particles\n");
    exit( -1);
  }
  particles->n_particles = n_particles;
  particles->_doubles = calloc(14 * n_particles, sizeof(double));
  particles->_ints = calloc(6 * n_particles, sizeof(int));
  particles->_randstate = calloc(7 * n_particles, sizeof(unsigned long));
  particles->_uid = calloc(n_particles, sizeof(unsigned long));
  particles->_next = calloc(n_particles, sizeof(long));
  if (particles->_doubles == NULL || particles->_ints == NULL || particles->_randstate == NULL ||
      particles->_uid == NULL || particles->_next == NULL) {
    fprintf(stderr, "Error: particle_soa_alloc: could not allocate memory for particle arrays\n");
    exit( -1);
  }
  return particles;
}

void particle_soa_free(_class_particle_soa * particles){
  if (particles == NULL) return;
  free(particles->_doubles);
  free(particles->_ints);
  free(particles->_uid);
  free(particles->_randstate);
  free(particles->_next);
  free(particles);
}

// copy the contents of one particle to another, with or without its random state
void particle_soa_duplicate_one(_class_particle_soa *dest, long dest_index, _class_particle_soa *src, long src_index, int copy_rand_state, int copy_next){
  if (dest == NULL || src == NULL) {
    fprintf(stderr, "Error: particle_soa_duplicate_one: dest or src is NULL\n");
    return;
  }
  if (dest->n_particles <= dest_index || src->n_particles <= src_index) {
    fprintf(stderr, "Error: particle_soa_duplicate_one: dest or src index out of bounds\n");
    return;
  }
  long d_stride=14;
  long i_stride=6;
  long r_stride=7;
  memcpy(dest->_doubles + d_stride * dest_index, src->_doubles + d_stride * src_index, sizeof(double) * d_stride);
  memcpy(dest->_ints + i_stride * dest_index, src->_ints + i_stride * src_index, sizeof(int) * i_stride);
  if (copy_rand_state) {
    memcpy(dest->_randstate + r_stride * dest_index, src->_randstate + r_stride * src_index, sizeof(unsigned long) * r_stride);
  }
  if (copy_next) {
    dest->_next[dest_index] = src->_next[src_index];
  }
  dest->_uid[dest_index] = src->_uid[src_index];
}

// copy the contents of one particle to another, including its random state
void particle_soa_copy_one(_class_particle_soa *dest, long dest_index, _class_particle_soa *src, long src_index){
  particle_soa_duplicate_one(dest, dest_index, src, src_index, 1, 0);
}


long per_particle_size_soa(){
  return sizeof(double) * 14 + sizeof(int) * 6 + sizeof(unsigned long) * 7 + sizeof(long) * 3 + sizeof(struct particles_logic_struct);
}