#pragma once

#include "particle.h"

long sort_absorb_last(_class_particle * particles, long len, _class_particle * pbuffer, long buffer_len, long flag_split, long* multiplier);

long sort_absorb_list(particle_node * input, long len, long * multiplier);