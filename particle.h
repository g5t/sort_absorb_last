#pragma once

struct particle_struct {
  double x;
  double y;
  double z;
  double vx;
  double vy;
  double vz;
  double sx;
  double sy;
  double sz;
  double p;
  double t;
  int _absorbed;
  double randstate[7];
};

typedef struct particle_struct _class_particle;

struct particle_node_struct {
  _class_particle * this;
  struct particle_node_struct * prev;
  struct particle_node_struct * next;
};

typedef struct particle_node_struct particle_node;

long particle_list_length(particle_node * nodes);
particle_node * particle_list_end(particle_node * nodes);
particle_node * particle_list_after(particle_node * nodes, long index);

long connect_particle_lists(particle_node ** nodes, long node_count, int loop);

void particle_list_free(particle_node * nodes, long count, int warn);

void particle_node_copy(particle_node * dest, particle_node * src, int copy_rand_state);

void print_particle(_class_particle * p);
void print_particle_list(particle_node * nodes);

particle_node * particle_list_rewind(particle_node * nodes);