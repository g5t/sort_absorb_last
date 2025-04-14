#include <stdio.h>
#include <stdlib.h>

#include "particle.h"
#include "particles.h"
#include "sort_absorb_last.h"

int particle_stats();
int particles_stats();

int main(int argc, char *argv[]) {
  if (argc != 1) {
    fprintf(stderr, "Usage: %s <min_n_particles> <max_n_particles> <1 ? array of struct : struct of array>\n", argv[0]);
    return 1;
  }

  particle_stats();
  particles_stats();
  return 0;
}

int particle_stats() {
  long container_size = sizeof(_class_particle*);
  long particle_size = sizeof(_class_particle);
  long nexts_size = sizeof(long);
  printf("array of structs\n");
  printf("  container size = %ld\n", container_size);
  printf("  per particle size = %ld\n", particle_size);
  printf("  next size = %ld\n", nexts_size);
  printf("  memory for 1024 particles = %ld\n", container_size + 1024 *(particle_size + nexts_size));
  return 0;
}

int particles_stats() {
  long class_size = sizeof(_class_particle_soa);
  long ppsize = per_particle_size_soa();
  printf("struct of arrays\n");
  printf("  sizeof(_class_particle_soa) container = %ld\n", class_size);
  printf("  required memory per particle = %ld\n", ppsize);
  printf("  memory for 1024 particles = %ld\n", class_size + ppsize * 1024);
  return 0;
}
