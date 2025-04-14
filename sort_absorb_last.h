#pragma once

#include "particle.h"

long sort_absorb_offset(_class_particle * particles, long * nexts, long len, long * multiplier);


long chunk_start(long len, long threads, long thread);
long chunk_size(long len, long threads, long thread);
