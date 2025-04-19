/**
   (c)2024 Karljohan Lundin Palmerius
*/

#ifndef GRAMODS_MISC_ACCUMULATIVEMEAN
#define GRAMODS_MISC_ACCUMULATIVEMEAN

#include <gmMisc/config.hh>

#include <cmath>

BEGIN_NAMESPACE_GMMISC

/**
   A means for accumulating values into a mean using two lists, one
   intermediate and one final, to keep as much precision as possible.
*/
template<class TYPE, class FLOAT = float> class AccumulativeMean {

public:
  AccumulativeMean(const TYPE &zero = 0)
    : intermediate_value(zero),
      mean_value(zero),
      intermediate_count(0),
      mean_count(0),
      zero(zero) {}

  AccumulativeMean(TYPE intermediate_value,
                   size_t intermediate_count,
                   TYPE mean_value,
                   size_t mean_count,
                   const TYPE &zero)
    : intermediate_value(intermediate_value),
      intermediate_count(intermediate_count),
      mean_value(mean_value),
      mean_count(mean_count),
      zero(zero) {}

  /**
     Function to estimate weighted mean, i.e. (1-r) * a + ( r ) * b.
  */
  std::function<TYPE(FLOAT r, const TYPE &a, const TYPE &b)> func_interp;

  /**
     Creates default interpolation function.
  */
  void setDefaultFunc() {
    func_interp = [](FLOAT r, const TYPE &a, const TYPE &b) -> TYPE {
      return (1 - r) * a + r * b;
    };
  }

  AccumulativeMean &operator+=(const TYPE &value) {
    if (intermediate_count == 0) {
      intermediate_value = value;
      intermediate_count = 1;
    } else {
      auto r = FLOAT(1) / FLOAT(1 + intermediate_count);
      intermediate_value = func_interp(r, intermediate_value, value);
      ++intermediate_count;
    }

    if (intermediate_count < next_accumulation_count)
      return *this;

    auto r = FLOAT(intermediate_count) / FLOAT(intermediate_count + mean_count);
    mean_value = func_interp(r, mean_value, intermediate_value);
    intermediate_value = zero;

    mean_count += intermediate_count;
    intermediate_count = 0;

    next_accumulation_count = size_t(std::sqrt(mean_count));

    return *this;
  }

  operator TYPE() const {
    if (!mean_count) return intermediate_value;
    auto r = FLOAT(intermediate_count) / FLOAT(intermediate_count + mean_count);
    return func_interp(r, mean_value, intermediate_value);
  }

private:
  TYPE intermediate_value;
  size_t intermediate_count;
  size_t next_accumulation_count = 1;

  TYPE mean_value;
  size_t mean_count;

  const TYPE zero;
};

END_NAMESPACE_GMMISC

#endif
