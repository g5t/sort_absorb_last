#pragma once

#include "particle.h"
#include "particles.h"

long sort_absorb_list(particle_node * input, long len, long * multiplier);

long sort_absorb_offset(_class_particle * particles, long * nexts, long len, long * multiplier);

long sort_absorb_soa(_class_particle_soa * particles, long * multiplier);

long chunk_start(long len, long threads, long thread);
long chunk_size(long len, long threads, long thread);


long separate_soa_single(_class_particle_soa * particles, long * left_start, long * left_total, long * right_start, long * right_total, int connect_left, int connect_right);