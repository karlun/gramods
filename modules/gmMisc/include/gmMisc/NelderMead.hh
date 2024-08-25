/**
   Copyright 2010-2011, 2022, Karljohan Lundin Palmerius
*/

#ifndef GRAMODS_MISC_NELDERMEAD
#define GRAMODS_MISC_NELDERMEAD

#include <gmMisc/config.hh>
#include <gmCore/Console.hh>

#include <vector>
#include <limits>
#include <cmath>

BEGIN_NAMESPACE_GMMISC

template<class TYPE_OUT, class TYPE_IN>
class NelderMead {
public:
  NelderMead(std::function<TYPE_OUT(const TYPE_IN &X)> F) : function(F) {}

  TYPE_IN solve(const std::vector<TYPE_IN> &X0, size_t &iterations);

  std::function<TYPE_OUT(const TYPE_IN &X)> function;
  TYPE_OUT epsilon = std::numeric_limits<TYPE_OUT>::epsilon();

  /**
     Function to estimate midpoint, i.e. the mean of all points except
     the last in the list.
  */
  std::function<TYPE_IN(const std::vector<std::pair<TYPE_OUT, TYPE_IN>> &F_X)>
      func_midpoint =
          [](const std::vector<std::pair<TYPE_OUT, TYPE_IN>> &F_X) -> TYPE_IN {
    auto factor = 1.f / (F_X.size() - 1.f);
    TYPE_IN Xm = F_X.front().second * factor;
    for (size_t idx = 1; idx < F_X.size() - 1; idx++) {
      Xm = Xm + F_X[idx].second * factor;
    }
    return Xm;
  };

  /**
     Function to estimate reflection, i.e. Xm + (Xm - Xn).
  */
  std::function<TYPE_IN(const TYPE_IN &Xm, const TYPE_IN &Xn)> func_reflect =
      [](const TYPE_IN &Xm, const TYPE_IN Xn) -> TYPE_IN {
    return Xm * 2.f - Xn;
  };

  /**
     Function to estimate mean, i.e. 0.5 x XA + 0.5 x XB.
  */
  std::function<TYPE_IN(const TYPE_IN &XA, const TYPE_IN &XB)> func_mean =
      [](const TYPE_IN &XA, const TYPE_IN XB) -> TYPE_IN {
    return XA * 0.5f + XB * 0.5f;
  };
};

template<class TYPE_OUT, class TYPE_IN>
TYPE_IN NelderMead<TYPE_OUT, TYPE_IN>::solve(const std::vector<TYPE_IN> &X0,
                                             size_t &iterations) {

  const size_t N = X0.size();
  if (N < 2)
    throw gmCore::InvalidArgument("Too few values in solution simplex!");

  std::vector<std::pair<TYPE_OUT, TYPE_IN>> F_X;
  F_X.reserve(N);
  for (const auto &X : X0) F_X.push_back({function(X), X});

  size_t iteration = 0;
  size_t count_reflect = 0;
  size_t count_expand = 0;
  size_t count_contract = 0;
  size_t count_shrink = 0;

  while (true) {
    ++iteration;
    if (iterations > 0 && iteration > iterations) {
      GM_DBG1("NelderMead",
              "Termination by iteration limits ("
                  << iterations << ") after " << count_reflect << " reflect, "
                  << count_expand << " expand, " << count_contract
                  << " contract, and " << count_shrink << " shrink.");
      return F_X.front().second;
    }

    // Step 1 - sort
    std::sort(F_X.begin(),
              F_X.end(),
              [](const std::pair<TYPE_OUT, TYPE_IN> &a,
                 const std::pair<TYPE_OUT, TYPE_IN> &b) {
                return a.first < b.first;
              });

    // Step 2 - calculate centroid (mid)
    TYPE_IN Xm = func_midpoint(F_X);

    // Step 3 - reflect
    TYPE_IN Xr = func_reflect(Xm, F_X.back().second);
    TYPE_OUT Fr = function(Xr);

    if (F_X.front().first <= Fr && Fr < F_X[N - 2].first) {
      F_X.pop_back();
      F_X.push_back({Fr, Xr});

      GM_DBG3("NelderMead", "Reflect (" << Fr << ")");
      ++count_reflect;
      continue;
    }

    // Step 4 - expand
    if (Fr < F_X.front().first) {

      TYPE_IN Xe = func_reflect(Xr, Xm);
      TYPE_OUT Fe = function(Xe);

      TYPE_IN Xn = Fe < Fr ? Xe : Xr;
      TYPE_OUT Fn = Fe < Fr ? Fe : Fr;

      F_X.pop_back();
      F_X.push_back({Fn, Xn});

      GM_DBG3("NelderMead", "Expand (" << Fn << ")");
      ++count_expand;
      continue;
    }

    // Step 5 - contract
    // here (Fr < F[n-2])
    if (Fr < F_X.back().first) {
      TYPE_IN Xc = func_mean(Xr, Xm);
      TYPE_OUT Fc = function(Xc);
      if (Fc < Fr) {

        F_X.pop_back();
        F_X.push_back({Fc, Xc});

        GM_DBG3("NelderMead", "Contract (" << Fc << ")");
        ++count_contract;
        continue;
      }
    } else /* Fr >= F_X.back().first */ {
      TYPE_IN Xc = func_mean(Xm, F_X.back().second);
      TYPE_OUT Fc = function(Xc);
      if (Fc < F_X.back().first) {

        F_X.pop_back();
        F_X.push_back({Fc, Xc});

        GM_DBG3("NelderMead", "Contract (" << Fc << ")");
        ++count_contract;
        continue;
      }
    }

    // Step 6 - shrink
    for (size_t i = 1; i < N; i++) {
      F_X[i].second = func_mean(F_X.front().second, F_X[i].second);
      F_X[i].first = function(F_X[i].second);
    }
    GM_DBG3("NelderMead",
            "Shrink (" << F_X.front().first << "/" << F_X.back().first << ")");

    for (size_t idx = 1; idx < N; ++idx) {
      if (std::fabs(F_X[0].first - F_X[idx].first) <=
          (1 + std::fabs(F_X[0].first) + std::fabs(F_X[idx].first)) * epsilon) {
        iterations = iteration;
        GM_DBG1("NelderMead",
                "Termination by precision after "
                    << count_reflect << " reflect, " << count_expand
                    << " expand, " << count_contract << " contract, and "
                    << count_shrink << " shrink.");
        return F_X[0].second;
      }
    }

    ++count_shrink;
  }
}

END_NAMESPACE_GMMISC

#endif
