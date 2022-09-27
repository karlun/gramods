
#ifndef GRAMODS_GRAPHICS_EYE
#define GRAMODS_GRAPHICS_EYE

#include <gmGraphics/config.hh>

#include <stdexcept>
#include <cstddef>
#include <sstream>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   An indexed eye that can be rendered.

   @see StereoscopicView
   @see StereoscopicMultiplexer
   @see Camera
*/
struct Eye {

  /**
     The index of the specific eye, starting at zero (0).
  */
  size_t idx;

  /**
     The total count of eyes of which the specified eye is one.
  */
  size_t count;

  /**
     Check if the internal states are valid and throw exception if idx
     is not smaller than count. This may be called by any method that
     takes an Eye as an argument.
  */
  void validate() {
    if (valid()) return;

    std::stringstream ss;
    ss << "Eye index " << idx << " is larger or equal to count " << count <<".";
    throw std::out_of_range(ss.str());
  }

  /**
     Returns true iff idx is smaller than count.
   */
  bool valid() { return idx < count; }

  /**
     Less than operator, returns true if this has lower count or lower
     idx than the other.
  */
  bool operator<(const gramods::gmGraphics::Eye &e2) const {
    return count < e2.count || (count == e2.count && idx < e2.idx);
  }

  /**
     Equals operator, returns true if this is identical to the other.
  */
  bool operator==(const gramods::gmGraphics::Eye &e2) const {
    return count == e2.count && idx == e2.idx;
  }

  /**
     Pre-defined Eye for monoscopic rendering.
  */
  static const Eye MONO;

  /**
     Pre-defined left Eye for stereoscopic rendering.
  */
  static const Eye LEFT;

  /**
     Pre-defined right Eye for stereoscopic rendering.
  */
  static const Eye RIGHT;
};

END_NAMESPACE_GMGRAPHICS;

BEGIN_NAMESPACE_GRAMODS;

/**
   Stream operator reading into an Eye. This is typically used to read
   XML attributes. This will recognize "MONO", "LEFT" and "RIGHT".
*/
std::istream& operator>> (std::istream &in, gmGraphics::Eye &e);

END_NAMESPACE_GRAMODS;

#endif
