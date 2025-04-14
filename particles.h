#pragma once

/* Particle JUMP control logic */
struct particles_logic_struct {
  int dummy;
};

struct _struct_particle_soa {
  double *x, *y,*z; // position [m]
  double *vx,*vy,*vz; // velocity [m/s]
  double *sx,*sy,*sz; // spin [0-1] // // Still just single thing  void *mcMagnet;    // precession-state //
  int *allow_backprop; // allow backprop
  // Generic Temporaries:
  // May be used internally by components e.g. for special
  // return-values from functions used in trace, thus returned via
  // particle struct. (Example: Wolter Conics from McStas, silicon slabs.)
  double *_mctmp_a; // temp a
  double *_mctmp_b; // temp b
  double *_mctmp_c; // temp c
  unsigned long * randstate; // randstate needs 2D memory allocation
  double *t, *p;     // time, event weight //
  long *_uid;  // Unique event ID //
  int *_index;     // component index where to send this event //
//// these are needed for SCATTERED, ABSORB and RESTORE macros //
  int *_absorbed;  // flag set to TRUE when this event is to be removed/ignored //
  int *_scattered; // flag set to TRUE when this event has interacted with the last component instance //
  int *_restore;   // set to true if neutron event must be restored //
  int *flag_nocoordschange;   // set to true if particle is jumping //
  long *_next; // next particle in the list
// Include the struct defined earlier holding information on JUMP logic//
  struct particles_logic_struct *_logic;
  long n_particles; // number of particles
};
//typedef struct _struct_particle_soa _class_particle_soa;


struct struct_particle_soa_new {
  double * _doubles; // x, y, z, vx, vy, vz, sx, sy, sz, t, p, _mctmp_a, _mctmp_b, _mctmp_c
  unsigned long * _randstate; // seven random state variables per particle
  int * _ints; // _index, _absorbed, _scattered, _restore, flag_nocoordschange, allow_backprop
  long * _uid; // unique per-particle identifier
  long * _next; // next particle in the list
  struct particles_logic_struct * _logic;
  long n_particles; // number of particles
};
typedef struct struct_particle_soa_new _class_particle_soa;


_class_particle_soa * particle_soa_alloc(long n_particles);

void particle_soa_free(_class_particle_soa * particles);

// copy the contents of one particle to another, with or without its random state
void particle_soa_duplicate_one(_class_particle_soa *dest, long dest_index, _class_particle_soa *src, long src_index, int copy_rand_state, int copy_next);

// copy the contents of one particle to another, including its random state
void particle_soa_copy_one(_class_particle_soa *dest, long dest_index, _class_particle_soa *src, long src_index);

long per_particle_size_soa();
