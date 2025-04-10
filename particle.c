#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "particle.h"


long particle_list_length(particle_node * nodes) {
  long count = 0;
  while (nodes != NULL) {
    count++;
    nodes = nodes->next;
  }
  return count;
}


particle_node * particle_list_end(particle_node * nodes) {
  while (nodes != NULL && nodes->this != NULL && nodes->next != NULL) {
    nodes = nodes->next;
  }
  return nodes;
}


particle_node * particle_list_after(particle_node * nodes, long index) {
  long count = 0;
  while (nodes != NULL && count < index) {
    count++;
    nodes = nodes->next;
  }
  return nodes;
}


long connect_particle_lists(particle_node ** nodes, long node_count, int loop) {
  long first;
  for (first = 0; first < node_count && nodes[first]->this == NULL; first++);
  if (first == node_count) {
    // no particles to connect
    return 0;
  }
  particle_node * end = particle_list_end(nodes[first]);
  for (long next=first + 1; next < node_count; next++) {
    if (nodes[next]->this){
      // found a particle, connect it to the list
      end->next = nodes[next];
      nodes[next]->prev = end;
      // move the end pointer to the new end
      end = particle_list_end(nodes[next]);
    }
  }
  // nodes[first] now connects all particles in each list, in order,
  // but we want the _first_ node to go through all particles
  if (first){
    nodes[0] = nodes[first];
  }
  // count the number of particles in the list
  long count = particle_list_length(nodes[0]);
  if (loop){
    // connect the last node to the first
    end->next = nodes[0];
    nodes[0]->prev = end;
  }
  return count;
}


void particle_list_free(particle_node * nodes, long count, int warn){
  particle_node * next;
  while (nodes != NULL && count > 0) {
    next = nodes->next;
    free(nodes);
    nodes = next;
    count--;
  }
}


void particle_node_copy(particle_node * dest, particle_node * src, int copy_rand_state){
  _class_particle p = *src->this;
  if (!copy_rand_state) {
    p.randstate[0] = dest->this->randstate[0];
    p.randstate[1] = dest->this->randstate[1];
    p.randstate[2] = dest->this->randstate[2];
    p.randstate[3] = dest->this->randstate[3];
    p.randstate[4] = dest->this->randstate[4];
    p.randstate[5] = dest->this->randstate[5];
    p.randstate[6] = dest->this->randstate[6];
  }
  memcpy(dest->this, &p, sizeof(_class_particle));
}

void print_particle(_class_particle * p){
  printf("Particle: x=%f, y=%f, z=%f, vx=%f, vy=%f, vz=%f, sx=%f, sy=%f, sz=%f, p=%f, t=%f, _absorbed=%d\n",
         p->x, p->y, p->z,
         p->vx, p->vy, p->vz,
         p->sx, p->sy, p->sz,
         p->p, p->t,
         p->_absorbed);
}
void print_particle_list(particle_node * nodes){
  while (nodes != NULL && nodes->this != NULL) {
    print_particle(nodes->this);
    nodes = nodes->next;
  }
}

particle_node * particle_list_rewind(particle_node * nodes){
  if (nodes->this == NULL && nodes->prev != NULL) {
    particle_node * remove = nodes;
    nodes = nodes->prev;
    free(remove);
    nodes->next = NULL;;
  }
  while (nodes != NULL && nodes->prev != NULL) {
    nodes = nodes->prev;
  }
  return nodes;
}