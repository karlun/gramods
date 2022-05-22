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

class NelderMead {
public:
  template<class TYPE_OUT, class TYPE_IN, class Function>
  static TYPE_IN
  solve(const std::vector<TYPE_IN> &X0,
        Function function,
        size_t &iterations,
        TYPE_OUT epsilon = std::numeric_limits<TYPE_OUT>::epsilon());
};

template<class TYPE_OUT, class TYPE_IN, class Function>
TYPE_IN NelderMead::solve(const std::vector<TYPE_IN> &X0,
                          Function function,
                          size_t &iterations,
                          TYPE_OUT epsilon) {

  const size_t N = X0.size();
  std::vector<std::pair<TYPE_OUT, TYPE_IN>> F_X;
  F_X.reserve(X0.size());
  for (const auto &X : X0) F_X.push_back({function(X), X});

  // Step 1 - sort
  std::sort(
      F_X.begin(),
      F_X.end(),
      [](const std::pair<TYPE_OUT, TYPE_IN> &a,
         const std::pair<TYPE_OUT, TYPE_IN> &b) { return a.first < b.first; });

  size_t iteration = 0;

  while (true) {
    ++iteration;
    if (iterations > 0 && iteration > iterations) { return F_X[0].second; }

    TYPE_IN Xm = F_X[0].second;
    for (size_t idx = 1; idx < N - 1; idx++) { Xm = Xm + F_X[idx].second; }
    Xm = Xm * (1.f / (N - 1));

    // (Step 2 - reflect)
    TYPE_IN Xr = Xm * 2.f - F_X[N - 1].second;
    TYPE_OUT Fr = function(Xr);

    // Step 2 - reflect
    if (F_X[0].first <= Fr && Fr < F_X[N - 2].first) {

      size_t idx;
      for (idx = 0; Fr >= F_X[idx].first; idx++)
        ;

      F_X.insert(F_X.begin() + idx, {Fr, Xr});
      F_X.pop_back();

      GM_DBG3("NelderMead", "Reflect (" << Fr << ")");
      continue;
    }

    // Step 3 - expand
    if (Fr < F_X[0].first) {

      TYPE_IN Xe = Xm * 2.f - Xr;
      TYPE_OUT Fe = function(Xe);

      TYPE_IN Xn = Fe < Fr ? Xe : Xr;
      TYPE_OUT Fn = Fe < Fr ? Fe : Fr;

      size_t idx;
      for (idx = 0; Fr >= F_X[idx].first; idx++)
        ;

      F_X.insert(F_X.begin() + idx, {Fn, Xn});
      F_X.pop_back();

      GM_DBG3("NelderMead", "Expand (" << Fe << ")");
      continue;
    }

    // Step 4 - contract
    TYPE_IN Xc = Xm * 0.5f + F_X[N - 1].second * 0.5f;
    TYPE_OUT Fc = function(Xc);
    if (Fc < F_X[N - 1].first) {

      size_t idx;
      for (idx = 0; Fc >= F_X[idx].first; idx++)
        ;

      F_X.insert(F_X.begin() + idx, {Fc, Xc});
      F_X.pop_back();

      GM_DBG3("NelderMead", "Contract (" << Fc << ")");
      continue;
    }

    // Step 5 - shrink
    for (size_t i = 1; i < N; i++) {
      F_X[i].second = F_X[0].second + (F_X[i].second - F_X[0].second) * 0.5f;
      F_X[i].first = function(F_X[i].second);
    }
    GM_DBG3("NelderMead",
            "Shrink (" << F_X[0].first << "/" << F_X[N - 1].first << ")");

    for (size_t idx = 1; idx < N; ++idx) {
      if (std::fabs(F_X[0].first - F_X[idx].first) <=
          (1 + std::fabs(F_X[0].first) + std::fabs(F_X[idx].first)) * epsilon) {
        iterations = iteration;
        return F_X[0].second;
      }
    }

    // Sort
    std::sort(F_X.begin(),
              F_X.end(),
              [](const std::pair<TYPE_OUT, TYPE_IN> &a,
                 const std::pair<TYPE_OUT, TYPE_IN> &b) {
                return a.first < b.first;
              });
  }
}

END_NAMESPACE_GMMISC

#endif
