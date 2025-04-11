#include <stdio.h>
#include <stdlib.h>

#include "particle.h"
#include "sort_absorb_last.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <n_particles> <1 ? pointers : offsets>\n", argv[0]);
    return 1;
  }
  long n_particles = strtol(argv[1], NULL, 10);
  long pointers_or_offsets = strtol(argv[2], NULL, 10);

  _class_particle * particles = calloc(n_particles, sizeof(_class_particle));
  particle_node * good = calloc(n_particles, sizeof(particle_node));
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
    particles[i].randstate[0] = (double) i;
    particles[i].randstate[1] = (double) i;
    particles[i].randstate[2] = (double) i;
    particles[i].randstate[3] = (double) i;
    particles[i].randstate[4] = (double) i;
    particles[i].randstate[5] = (double) i;
    particles[i].randstate[6] = (double) i;
  }

  // Start with all particles in 'good' list because they just passed through a component
  for (long i=0; i < n_particles; i++){
    good[i].this = particles + i;
    good[i].prev = i ? NULL : good + i - 1;
    good[i].next = i < n_particles - 1 ? good + i + 1 : NULL;
  }
  // Set up the offsets for the good list
  for (long i=0; i < n_particles; i++){
    nexts[i] = i + 1; // i < n_particles - 1 ? i + 1 : n_particles + 1;
  }

  printf("Pre sorted\n");
  for (long i=0; i < 1 + 0 *n_particles; i++){
    printf("Particle %ld: x=%f, _absorbed=%d, rand=%f\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0]);
  }

//  long returned = sort_absorb_last(particles, n_particles, buffer, n_buffer, 1, &weight);

  long weight;
  long returned;
  if (pointers_or_offsets){
    printf("Sorting pointers\n");
    returned = sort_absorb_list(good, n_particles, &weight);
  } else {
    printf("Sorting offsets\n");
    returned = sort_absorb_offset(particles, nexts, n_particles, &weight);
  }

  // the expected result is that all particles with _absorbed == 1 are unmoved, but overwritten by one of the good
  // particles in the list. Since every third particle is good, the pattern should be
  // 0, 0, 3, 3, 6, 9, 6, 12, 15, 9, 18, 21, 12, 24, 27, 15, 30, 33, 18, 36, 39 ...
  int correct_count = 0;
  int counter = -3;
  for (long i=0; i<n_particles; i++){
    if (particles[i]._absorbed == 0 && particles[i].randstate[0] == (double)i){
      long expected = (i % 3) == 0 ? i : (counter+=3) ;
      if (particles[i].x == (double)expected){
        ++correct_count;
      } else {
        printf("Particle %ld: x=%f, _absorbed=%d, rand=%f, expected %ld\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0], expected);
      }
    } else {
      printf("Particle %ld: x=%f, _absorbed=%d, rand=%f matching%f\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0], particles[i].randstate[0] - (double) i);
    }
    if (counter >= n_particles - 3) counter = -3;
  }
  printf("Correct count: %d of %ld\n", correct_count, n_particles);

//
//
//  printf("\nReturned %ld particles with weight %ld\n", returned, weight);
//  for (long i=0; i < n_particles; i++){
//    printf("Particle %ld: x=%f, _absorbed=%d, rand=%f thread=%f\n", i, particles[i].x, particles[i]._absorbed, particles[i].randstate[0], particles[i].randstate[5]);
//  }

  // pretend we're going on with this linked list scheme, so de-tangle the particle list:
  for (long i=0; i < n_particles; i++){
    good[i].prev = i ? NULL : good + i - 1;
    good[i].next = i < n_particles - 1 ? good + i + 1 : NULL;
  }

  free(particles);
  free(good);
  free(nexts);
  return correct_count == n_particles && returned == n_particles ? 0 : 1;
}
