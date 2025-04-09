#include <stdio.h>
#include <stdlib.h>

#include "particle.h"
#include "sort_absorb_last.h"

int main() {
  long n_particles = 20;
  long n_buffer = n_particles;
  _class_particle * particles = calloc(n_particles, sizeof(_class_particle));
  _class_particle * buffer = calloc(n_buffer, sizeof(_class_particle));

  for (long i=0; i < n_particles; i++){
    particles[i].x = (double) i;
    particles[i].y = (double) i;
    particles[i].z = (double) i;
    particles[i].vx = (double) i;
    particles[i].vy = (double) i;
    particles[i].vz = (double) i;
    particles[i].sx = (double) i;
    particles[i].sy = (double) i;
    particles[i].sz = (double) i;
    particles[i].p = (double) i;
    particles[i].t = (double) i;
    particles[i]._absorbed = (int) (i % 3);
    particles[i].randstate[0] = (double) i;
    particles[i].randstate[1] = (double) i;
    particles[i].randstate[2] = (double) i;
    particles[i].randstate[3] = (double) i;
    particles[i].randstate[4] = (double) i;
    particles[i].randstate[5] = (double) i;
    particles[i].randstate[6] = (double) i;
  }
  long weight;
  printf("Pre sorted\n");
  for (long i=0; i < n_particles; i++){
    printf("Particle %ld: x=%f, _absorbed=%d, rand=%f\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0]);
  }

  long returned = sort_absorb_last(particles, n_particles, buffer, n_buffer, 1, &weight);

  printf("Returned %ld particles with weight %ld\n", returned, weight);
  for (long i=0; i < n_particles; i++){
    printf("Particle %ld: x=%f, _absorbed=%d, rand=%f\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0]);
  }

  free(particles);
  free(buffer);
  return 0;
}
