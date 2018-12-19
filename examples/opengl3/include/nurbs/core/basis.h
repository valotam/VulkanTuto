/**
@file
@brief Low-level functions for evaluating B-spline basis functions and 
      their derivatives
*/

#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include "../util/util.h"
#include "../util/array2.h"

namespace nurbs {

/**
Find the span of the given parameter in the knot vector.
@param[in] degree Degree of the curve.
@param[in] knots Knot vector of the curve.
@param[in] parameter_u Parameter value.
@return Span index into the knot vector 
        such that (span - 1) < parameter_u <= span
*/
template <typename T>
int FindSpan(
    const unsigned int degree, 
    const std::vector<T> &knots, 
    const T parameter_u) {
  int num_control_points = knots.size() - degree - 1;
  int last_internal_knot_idx = num_control_points - 1;
  /*
    // For parameter_u that is equal to last knot value
    if (util::CloseTo(parameter_u, knots[last_internal_knot_idx + 1])) {
        return last_internal_knot_idx;
    }
  */
  // For values of parameter_u that lies outside the domain
  if (parameter_u >= (knots[last_internal_knot_idx + 1]
                      - std::numeric_limits<T>::epsilon())) {
    return last_internal_knot_idx;
  }
  if (parameter_u <= (knots[degree] + std::numeric_limits<T>::epsilon())) {
    return degree;
  }

  // Binary search
  auto iter = std::upper_bound(knots.begin(), knots.end(), parameter_u) - 1;
  int span = iter - knots.begin();

  return span;
}

/**
Compute a single B-spline basis function
@param[in] ith_idx The ith basis function to compute.
@param[in] degree Degree of the basis function.
@param[in] knots Knot vector corresponding to the basis functions.
@param[in] parameter_u Parameter to evaluate the basis functions at.
@return The value of the ith basis function at parameter_u.
*/
template <typename T>
T BsplineOneBasis(
    int ith_idx,
    unsigned int degree,
    const std::vector<T> &knots,
    T parameter_u) {
  int num_control_points = knots.size() - degree - 1;
  int last_internal_knot_idx = num_control_points - 1;

  // Special case: endpoints of curve
  static auto IsEndPoint = [](
      const int idx, const int idx_open_knot,
      const T value, const T value_open_knot) -> bool {
    return ((idx == idx_open_knot) && util::CloseTo(value, value_open_knot));
  };
  if (IsEndPoint(ith_idx, 0, parameter_u, knots[0]) ||
      IsEndPoint(ith_idx, last_internal_knot_idx,
                 parameter_u, knots[last_internal_knot_idx + 1])) {
    return 1.0;
  }
  // Local property ensures that basis function is zero outside span
  static auto IsOutOfSpan = [](
      const T value, const T start, const T end) -> bool {
    return (value < start || value >= end);
  };
  if (IsOutOfSpan(parameter_u, knots[ith_idx], knots[ith_idx + degree + 1]))
    return 0.0;
  // Initialize zeroth-degree functions
  std::vector<double> basis(degree + 1, 0);
  for (int j = 0; j <= degree; ++j) {
    if (!IsOutOfSpan(parameter_u, knots[ith_idx + j], knots[ith_idx + j + 1]))
      basis[j] = 1.0;
  }
  // Compute triangular table
  for (int k = 1; k <= degree; ++k) {
    T saved = (util::CloseTo(basis[0], 0.0)) ? 0.0
              : ((parameter_u - knots[ith_idx])*basis[0]) /
                (knots[ith_idx + k] - knots[ith_idx]);
    for (int j = 0; j < degree - k + 1; ++j) {
      T left = knots[ith_idx + j + 1];
      T right = knots[ith_idx + j + k + 1];
      if (util::CloseTo(basis[j + 1], 0.0)) {
        basis[j] = saved;
        saved = 0.0;
      }
      else {
        T temp = basis[j + 1] / (right - left);
        basis[j] = saved + (right - parameter_u)*temp;
        saved = (parameter_u - left)*temp;
      }
    }
  }
  return basis[0];
}

/**
Compute all non-zero B-spline basis functions
@param[in] degree Degree of the basis function.
@param[in] span Index obtained from FindSpan() corresponding the parameter_u and knots.
@param[in] knots Knot vector corresponding to the basis functions.
@param[in] parameter_u Parameter to evaluate the basis functions at.
@return basis Values of (degree+1) non-zero basis functions.
*/
template <typename T>
std::vector<T> BsplineBasis(
    unsigned int degree,
    int span,
    const std::vector<T> &knots,
    T parameter_u) {
  std::vector<T> basis(degree + 1, static_cast<T>(0));
  std::vector<T> left (degree + 1, static_cast<T>(0)),
                 right(degree + 1, static_cast<T>(0));
  T saved = 0.0, temp = 0.0;

  basis[0] = 1.0;

  for (int j = 1; j <= degree; j++) {
    left[j] = (parameter_u - knots[span + 1 - j]);
    right[j] = knots[span + j] - parameter_u;
    saved = 0.0;
    for (int r = 0; r < j; r++) {
      temp = basis[r] / (right[r + 1] + left[j - r]);
      basis[r] = saved + right[r + 1] * temp;
      saved = left[j - r] * temp;
    }
    basis[j] = saved;
  }
  return basis;
}

/**
// Compute all non-zero derivatives of B-spline basis functions
@param[in] degree Degree of the basis function.
@param[in] span Index obtained from FindSpan() corresponding the parameter_u and knots.
@param[in] knots Knot vector corresponding to the basis functions.
@param[in] parameter_u Parameter to evaluate the basis functions at.
@param[in] num_ders Number of derivatives to compute (num_ders <= degree)
@return ders Values of non-zero derivatives of basis functions.
*/
template <typename T>
array2<T> BsplineDerBasis(
    unsigned int deg,
    int span,
    const std::vector<T> &knots,
    T u,
    int num_ders) {
  std::vector<T> left, right;
  left.resize(deg + 1, 0.0);
  right.resize(deg + 1, 0.0);
  T saved = 0.0, temp = 0.0;

  array2<T> ndu(deg + 1, deg + 1);
  ndu(0, 0) = 1.0;

  for (int j = 1; j <= deg; j++) {
    left[j] = u - knots[span + 1 - j];
    right[j] = knots[span + j] - u;
    saved = 0.0;

    for (int r = 0; r < j; r++) {
      // Lower triangle
      ndu(j, r) = right[r + 1] + left[j - r];
      temp = ndu(r, j - 1) / ndu(j, r);
      // Upper triangle
      ndu(r, j) = saved + right[r + 1] * temp;
      saved = left[j - r] * temp;
    }

    ndu(j, j) = saved;
  }

  array2<T> ders(num_ders + 1, deg + 1, T(0));

  for (int j = 0; j <= deg; j++) {
    ders(0, j) = ndu(j, deg);
  }

  array2<T> a(2, deg + 1);

  for (int r = 0; r <= deg; r++) {
    int s1 = 0;
    int s2 = 1;
    a(0, 0) = 1.0;

    for (int k = 1; k <= num_ders; k++) {
      T d = 0.0;
      int rk = r - k;
      int pk = deg - k;
      int j1 = 0;
      int j2 = 0;

      if (r >= k) {
        a(s2, 0) = a(s1, 0) / ndu(pk + 1, rk);
        d = a(s2, 0) * ndu(rk, pk);
      }

      if (rk >= -1) {
        j1 = 1;
      }
      else {
        j1 = -rk;
      }

      if (r - 1 <= pk) {
        j2 = k - 1;
      }
      else {
        j2 = deg - r;
      }

      for (int j = j1; j <= j2; j++) {
        a(s2, j) = (a(s1, j) - a(s1, j - 1)) / ndu(pk + 1, rk + j);
        d += a(s2, j) * ndu(rk + j, pk);
      }

      if (r <= pk) {
        a(s2, k) = -a(s1, k - 1) / ndu(pk + 1, r);
        d += a(s2, k) * ndu(r, pk);
      }

      ders(k, r) = d;

      int temp = s1;
      s1 = s2;
      s2 = temp;
    }
  }

  T fac = static_cast<T>(deg);
  for (int k = 1; k <= num_ders; k++) {
    for (int j = 0; j <= deg; j++) {
      ders(k, j) *= fac;
    }
    fac *= static_cast<T>(deg - k);
  }

  return ders;
}

} // namespace nurbs
