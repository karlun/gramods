

#ifndef GRAMODS_GRAPHICS_MATHS
#define GRAMODS_GRAPHICS_MATHS

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Quickly calculates the next power of two.
*/
int nextPowerOfTwo(int v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

END_NAMESPACE_GMGRAPHICS;

#endif
