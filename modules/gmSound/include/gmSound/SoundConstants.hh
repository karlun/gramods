
#ifndef GRAMODS_CORE_SOUNDCONSTANTS
#define GRAMODS_CORE_SOUNDCONSTANTS

#include <gmMisc/PolyFit.hh>

namespace gramods {
/**
   The speed of sound in standard air at 20°C.
*/
constexpr float SPEED_OF_SOUND_T20 = 343.f;

/**
   The speed of sound in standard air at 0°C.
*/
constexpr float SPEED_OF_SOUND_T0 = 331.f;

/**
   Second order approximation of the speed of sound in standard air,
   based on table at
   [Wikipedia](https://en.wikipedia.org/wiki/Speed_of_sound).
*/
const gmMisc::PolyFit SPEED_OF_SOUND({{35},
                                      {30},
                                      {25},
                                      {20},
                                      {15},
                                      {10},
                                      {5},
                                      {0},
                                      {-5},
                                      {-10},
                                      {-15},
                                      {-20},
                                      {-25}},
                                     {{351.88},
                                      {349.02},
                                      {346.13},
                                      {343.21},
                                      {340.27},
                                      {337.31},
                                      {334.32},
                                      {331.30},
                                      {328.25},
                                      {325.18},
                                      {322.07},
                                      {318.94},
                                      {315.77}},
                                     2);
}

#endif
