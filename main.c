#include <stdio.h>
#include <stdlib.h>

#include "particle.h"
#include "sort_absorb_last.h"

int run_particle_tests(long min, long max);
int run_particle_test(long n_particles, _class_particle * particles, long * nexts);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <min_n_particles> <max_n_particles>\n", argv[0]);
    return 1;
  }
  long first = strtol(argv[1], NULL, 10);
  long last = strtol(argv[2], NULL, 10);

  int result = run_particle_tests(first, last);
  return result;
}

int run_particle_tests(long min, long max){
  if (min > max){
    long a = min;
    min = max;
    max = a;
  }
  _class_particle * particles = calloc(max, sizeof(_class_particle));
  long * nexts = calloc(max, sizeof(long));

  int result;
#pragma acc data copy(particles[0:max], nexts[0:max])
  for (long n=min; n<=max; ++n){
    result = run_particle_test(n, particles, nexts);
    if (result != 0) {
      fprintf(stderr, "Error: run_particle_test: failed for n_particles = %ld\n", n);
//      break;
    }
  }
  free(particles);
  free(nexts);
  return result;
}

#pragma acc routine seq
int run_particle_test(long n_particles, _class_particle * particles, long * nexts) {
  if (n_particles <= 0) {
    fprintf(stderr, "Error: run_particle_test: n_particles must be > 0\n");
    return 1;
  }
  if (particles == NULL || nexts == NULL) {
    fprintf(stderr, "Error: run_particle_test: particles or nexts is NULL\n");
    return 1;
  }
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

  return correct_count == n_particles && returned == n_particles ? 0 : 1;
}
