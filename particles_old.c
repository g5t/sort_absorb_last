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
  particles->x = calloc(n_particles, sizeof(double));
  particles->y = calloc(n_particles, sizeof(double));
  particles->z = calloc(n_particles, sizeof(double));
  particles->vx = calloc(n_particles, sizeof(double));
  particles->vy = calloc(n_particles, sizeof(double));
  particles->vz = calloc(n_particles, sizeof(double));
  particles->sx = calloc(n_particles, sizeof(double));
  particles->sy = calloc(n_particles, sizeof(double));
  particles->sz = calloc(n_particles, sizeof(double));
  particles->allow_backprop = calloc(n_particles, sizeof(int));
  particles->randstate = calloc(7 * n_particles, sizeof(unsigned long));
  particles->_mctmp_a = calloc(n_particles, sizeof(double));
  particles->_mctmp_b = calloc(n_particles, sizeof(double));
  particles->_mctmp_c = calloc(n_particles, sizeof(double));
  particles->t = calloc(n_particles, sizeof(double));
  particles->p = calloc(n_particles, sizeof(double));
  particles->_uid = calloc(n_particles, sizeof(unsigned long));
  particles->_index = calloc(n_particles, sizeof(int));
  particles->_absorbed = calloc(n_particles, sizeof(int));
  particles->_scattered = calloc(n_particles, sizeof(int));
  particles->_restore = calloc(n_particles, sizeof(int));
  particles->flag_nocoordschange = calloc(n_particles, sizeof(int));
  particles->_next = calloc(n_particles, sizeof(long));
  if (particles->x == NULL || particles->y == NULL || particles->z == NULL ||
      particles->vx == NULL || particles->vy == NULL || particles->vz == NULL ||
      particles->sx == NULL || particles->sy == NULL || particles->sz == NULL ||
      particles->allow_backprop == NULL || particles->randstate == NULL ||
      particles->_mctmp_a == NULL || particles->_mctmp_b == NULL ||
      particles->_mctmp_c == NULL || particles->t == NULL || particles->p == NULL ||
      particles->_uid == NULL || particles->_index == NULL ||
      particles->_absorbed == NULL
      || particles->_scattered == NULL || particles->_restore == NULL || particles->flag_nocoordschange == NULL
      || particles->_next == NULL) {
    fprintf(stderr, "Error: particle_soa_alloc: could not allocate memory for particle arrays\n");
    exit( -1);
  }
  return particles;
}

void particle_soa_free(_class_particle_soa * particles){
  if (particles == NULL) return;
  free(particles->x);
  free(particles->y);
  free(particles->z);
  free(particles->vx);
  free(particles->vy);
  free(particles->vz);
  free(particles->sx);
  free(particles->sy);
  free(particles->sz);
  free(particles->allow_backprop);
  free(particles->_mctmp_a);
  free(particles->_mctmp_b);
  free(particles->_mctmp_c);
  free(particles->randstate);
  free(particles->t);
  free(particles->p);
  free(particles->_uid);
  free(particles->_index);
  free(particles->_absorbed);
  free(particles->_scattered);
  free(particles->_restore);
  free(particles->flag_nocoordschange);
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
  dest->x[dest_index] = src->x[src_index];
  dest->y[dest_index] = src->y[src_index];
  dest->z[dest_index] = src->z[src_index];
  dest->vx[dest_index] = src->vx[src_index];
  dest->vy[dest_index] = src->vy[src_index];
  dest->vz[dest_index] = src->vz[src_index];
  dest->sx[dest_index] = src->sx[src_index];
  dest->sy[dest_index] = src->sy[src_index];
  dest->sz[dest_index] = src->sz[src_index];
  dest->_absorbed[dest_index] = src->_absorbed[src_index];
  dest->t[dest_index] = src->t[src_index];
  dest->p[dest_index] = src->p[src_index];
  if (copy_rand_state) {
    long stride = 7;
    long dest_offset = stride * dest_index;
    long src_offset = stride * src_index;
    memcpy(dest->randstate + dest_offset, src->randstate + src_offset, sizeof(unsigned long) * stride);
  }
  if (copy_next){
    dest->_next[dest_index] = src->_next[src_index];
  }
  dest->_mctmp_a[dest_index] = src->_mctmp_a[src_index];
  dest->_mctmp_b[dest_index] = src->_mctmp_b[src_index];
  dest->_mctmp_c[dest_index] = src->_mctmp_c[src_index];
  dest->flag_nocoordschange[dest_index] = src->flag_nocoordschange[src_index];
  dest->_scattered[dest_index] = src->_scattered[src_index];
  dest->_restore[dest_index] = src->_restore[src_index];
  dest->_uid[dest_index] = src->_uid[src_index];
  dest->_index[dest_index] = src->_index[src_index];
  dest->allow_backprop[dest_index] = src->allow_backprop[src_index];
}

// copy the contents of one particle to another, including its random state
void particle_soa_copy_one(_class_particle_soa *dest, long dest_index, _class_particle_soa *src, long src_index){
  particle_soa_duplicate_one(dest, dest_index, src, src_index, 1, 0);
}


long per_particle_size_soa(){
  return sizeof(double) * 14 + sizeof(int) * 6 + sizeof(unsigned long) * 7 + sizeof(long) * 3 + sizeof(struct particles_logic_struct);
}