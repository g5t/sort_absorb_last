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