#include <stdio.h>
#include <stdlib.h>

#include "particle.h"
#include "particles.h"
#include "sort_absorb_last.h"

int run_particle_test(long n_particles);
int run_particles_test(long n_particles);

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <min_n_particles> <max_n_particles> <1 ? array of struct : struct of array>\n", argv[0]);
    return 1;
  }
  long first = strtol(argv[1], NULL, 10);
  long last = strtol(argv[2], NULL, 10) + 1;
  long soa = strtol(argv[3], NULL, 10);

  for (long n=first; n < last; ++n){
//    printf("Testing %ld particles\n", n);
    int result = soa ? run_particle_test(n) : run_particles_test(n);
    if (result != 0) {
      fprintf(stderr, "Test failed for %ld particles\n", n);
      return 1;
    }
  }
  return 0;
};

int run_particle_test(long n_particles) {
  _class_particle * particles = calloc(n_particles, sizeof(_class_particle));
  long * nexts = calloc(n_particles, sizeof(long));
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
    particles[i].randstate[0] = (unsigned long) i;
    particles[i].randstate[1] = (unsigned long) i;
    particles[i].randstate[2] = (unsigned long) i;
    particles[i].randstate[3] = (unsigned long) i;
    particles[i].randstate[4] = (unsigned long) i;
    particles[i].randstate[5] = (unsigned long) i;
    particles[i].randstate[6] = (unsigned long) i;
  }
  // Set up the offsets for the good list
  for (long i=0; i < n_particles; i++){
    nexts[i] = i + 1; // i < n_particles - 1 ? i + 1 : n_particles + 1;
  }

//  printf("Pre sorted\n");
//  for (long i=0; i < 1 + 0 *n_particles; i++){
//    printf("Particle %ld: x=%f, _absorbed=%d, rand=%ld\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0]);
//  }

  long weight;
  long returned;
  returned = sort_absorb_offset(particles, nexts, n_particles, &weight);

  // the expected result is that all particles with _absorbed == 1 are unmoved, but overwritten by one of the good
  // particles in the list. Since every third particle is good, the pattern should be
  // 0, 0, 3, 3, 6, 9, 6, 12, 15, 9, 18, 21, 12, 24, 27, 15, 30, 33, 18, 36, 39 ...
  int correct_count = 0;
  int counter = -3;
  for (long i=0; i<n_particles; i++){
    if (particles[i]._absorbed == 0 && particles[i].randstate[0] == (unsigned long)i){
      long expected = (i % 3) == 0 ? i : (counter+=3) ;
      if (particles[i].x == (double)expected){
        ++correct_count;
      } else {
        printf("Particle %ld: x=%f, _absorbed=%d, rand=%ld, expected %ld\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0], expected);
      }
    } else {
      printf("Particle %ld: x=%f, _absorbed=%d, rand=%ld matching%ld\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0], particles[i].randstate[0] - i);
    }
    if (counter >= n_particles - 3) counter = -3;
  }


//  printf("Correct count: %d of %ld\n", correct_count, n_particles);
//  printf("\nReturned %ld particles with weight %ld\n", returned, weight);
//  for (long i=0; i < n_particles; i++){
//    printf("Particle %ld: x=%f, _absorbed=%d, rand=%f thread=%f\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0], particles[i].randstate[5]);
//  }
  free(particles);
  free(nexts);
  return correct_count == n_particles && returned == n_particles ? 0 : 1;
}


int run_particles_test(long n_particles) {
  _class_particle_soa * particles = particle_soa_alloc(n_particles);
  for (long i=0; i<n_particles; ++i){
    particles->n_particles = n_particles;
    particles->x[i] = (double)i;
    particles->y[i] = (double)i;
    particles->z[i] = (double)i;
    particles->vx[i] = (double)i;
    particles->vy[i] = (double)i;
    particles->vz[i] = (double)i;
    particles->sx[i] = (double)i;
    particles->sy[i] = (double)i;
    particles->sz[i] = (double)i;
    particles->p[i] = (double)i;
    particles->t[i] = (double)i;
    particles->_absorbed[i] = (int)(i % 3);
    particles->randstate[i][0] = i;
    particles->randstate[i][1] = i;
    particles->randstate[i][2] = i;
    particles->randstate[i][3] = i;
    particles->randstate[i][4] = i;
    particles->randstate[i][5] = i;
    particles->randstate[i][6] = i;
    particles->next[i] = i + 1;
  }

  long weight;
  long returned;
  returned = sort_absorb_soa(particles, &weight);

  // the expected result is that all particles with _absorbed == 1 are unmoved, but overwritten by one of the good
  // particles in the list. Since every third particle is good, the pattern should be
  // 0, 0, 3, 3, 6, 9, 6, 12, 15, 9, 18, 21, 12, 24, 27, 15, 30, 33, 18, 36, 39 ...
  int correct_count = 0;
  int counter = -3;
  for (long i=0; i<n_particles; i++){
    if (particles->_absorbed[i] == 0 && particles->randstate[i][0] == (unsigned long)i){
      long expected = (i % 3) == 0 ? i : (counter+=3) ;
      if (particles->x[i] == (double)expected){
        ++correct_count;
      } else {
        printf("Particle %ld: x=%f, _absorbed=%d, rand=%ld, expected %ld\n", i, particles->x[i], particles->_absorbed[i], particles->randstate[i][0], expected);
      }
    } else {
      printf("Particle %ld: x=%f, _absorbed=%d, rand=%ld matching%ld\n", i, particles->x[i], particles->_absorbed[i], particles->randstate[i][0], particles->randstate[i][0] - i);
    }
    if (counter >= n_particles - 3) counter = -3;
  }
//  printf("Correct count: %d of %ld\n", correct_count, n_particles);
//  printf("\nReturned %ld particles with weight %ld\n", returned, weight);
//  for (long i=0; i < n_particles; i++){
//    printf("Particle %ld: x=%f, _absorbed=%d, rand=%f thread=%f\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0], particles[i].randstate[5]);
//  }

  // pretend we're going on with this linked list scheme, so de-tangle the particle list:
//  for (long i=0; i < n_particles; i++){
//    particles->next[i] = i + 1;
//  }

  particle_soa_free(particles);
  return correct_count == n_particles && returned == n_particles ? 0 : 1;
}
