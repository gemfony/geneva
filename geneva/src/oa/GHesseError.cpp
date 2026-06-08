/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/oa/GHesseError.hpp"

#include <cmath>
#include <utility>

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
namespace {

/** @brief Returns x with x[idx] += delta (a single-coordinate perturbation). */
std::vector<double> perturb1(std::vector<double> x, std::size_t idx, double delta) {
    x[idx] += delta;
    return x;
}

/** @brief Returns x with x[i] += di and x[j] += dj (a two-coordinate perturbation). */
std::vector<double> perturb2(std::vector<double> x, std::size_t i, double di, std::size_t j, double dj) {
    x[i] += di;
    x[j] += dj;
    return x;
}

/**
 * @brief Inverts a symmetric matrix by Gauss-Jordan elimination with partial pivoting. Returns false
 * (leaving @p inv untouched) if the matrix is singular / too ill-conditioned to invert. Original,
 * self-contained implementation -- no external linear-algebra code.
 */
bool invertMatrix(std::vector<std::vector<double>> const &m, std::vector<std::vector<double>> &inv) {
    const std::size_t n = m.size();
    // Work on an augmented [m | I] system.
    std::vector<std::vector<double>> a(n, std::vector<double>(2 * n, 0.));
    for(std::size_t i = 0; i < n; ++i) {
        for(std::size_t j = 0; j < n; ++j) {
            a[i][j] = m[i][j];
        }
        a[i][n + i] = 1.;
    }

    for(std::size_t col = 0; col < n; ++col) {
        // Partial pivot: pick the row with the largest magnitude in this column.
        std::size_t pivot = col;
        double best = std::fabs(a[col][col]);
        for(std::size_t r = col + 1; r < n; ++r) {
            if(std::fabs(a[r][col]) > best) {
                best = std::fabs(a[r][col]);
                pivot = r;
            }
        }
        if(best < 1.e-300) {
            return false; // singular
        }
        if(pivot != col) {
            std::swap(a[pivot], a[col]);
        }

        // Normalise the pivot row.
        const double pivot_val = a[col][col];
        for(std::size_t j = 0; j < 2 * n; ++j) {
            a[col][j] /= pivot_val;
        }

        // Eliminate this column from every other row.
        for(std::size_t r = 0; r < n; ++r) {
            if(r == col) {
                continue;
            }
            const double factor = a[r][col];
            if(factor != 0.) {
                for(std::size_t j = 0; j < 2 * n; ++j) {
                    a[r][j] -= factor * a[col][j];
                }
            }
        }
    }

    inv.assign(n, std::vector<double>(n, 0.));
    for(std::size_t i = 0; i < n; ++i) {
        for(std::size_t j = 0; j < n; ++j) {
            inv[i][j] = a[i][n + j];
        }
    }
    return true;
}

} /* anonymous namespace */

/******************************************************************************/

GHesseErrorResult GHesseError::estimate(
    eval_fn_t const &eval_fn,
    std::vector<double> const &x_min,
    double f_min,
    std::vector<double> const &step_sizes,
    GHesseErrorOptions const &opts
) const {
    GHesseErrorResult result;
    result.up = opts.up;

    const std::size_t n = x_min.size();
    if(n == 0 || step_sizes.size() != n) {
        return result; // invalid input
    }

    // --- diagonal of the Hessian (always; O(n) evaluations) -----------------------------------------
    // H_jj = (f(x + h_j e_j) - 2 f(x) + f(x - h_j e_j)) / h_j^2.
    std::vector<std::vector<double>> diag_points;
    diag_points.reserve(2 * n);
    for(std::size_t j = 0; j < n; ++j) {
        diag_points.push_back(perturb1(x_min, j, step_sizes[j]));
        diag_points.push_back(perturb1(x_min, j, -step_sizes[j]));
    }
    const std::vector<double> diag_values = eval_fn(diag_points);
    result.n_evaluations += diag_points.size();

    std::vector<double> hessian_diag(n, 0.);
    result.parameter_errors.assign(n, 0.);
    double min_curv = 0.;
    double max_curv = 0.;
    bool any_positive = false;
    for(std::size_t j = 0; j < n; ++j) {
        const double h = step_sizes[j];
        const double f_plus = diag_values[2 * j];
        const double f_minus = diag_values[2 * j + 1];
        const double curv = (h > 0.) ? (f_plus - 2. * f_min + f_minus) / (h * h) : 0.;
        hessian_diag[j] = curv;
        if(curv > 0.) {
            // Parameter-fixed parabolic error: 1/2 * H_jj * sigma_j^2 = UP -> sigma_j = sqrt(2 UP / H_jj).
            result.parameter_errors[j] = std::sqrt(2. * opts.up / curv);
            if(not any_positive) {
                min_curv = max_curv = curv;
                any_positive = true;
            }
            else {
                min_curv = (curv < min_curv) ? curv : min_curv;
                max_curv = (curv > max_curv) ? curv : max_curv;
            }
        }
    }
    result.valid = any_positive;
    result.condition_number = (min_curv > 0.) ? (max_curv / min_curv) : 0.;

    // --- optional full Hessian -> covariance (small dimension only) ---------------------------------
    if(opts.full_covariance && n <= opts.max_full_dim && result.valid) {
        // Off-diagonal mixed second differences:
        // H_ij = (f(x+hi+hj) - f(x+hi-hj) - f(x-hi+hj) + f(x-hi-hj)) / (4 hi hj).
        std::vector<std::vector<double>> off_points;
        std::vector<std::pair<std::size_t, std::size_t>> off_index;
        for(std::size_t i = 0; i < n; ++i) {
            for(std::size_t j = i + 1; j < n; ++j) {
                const double hi = step_sizes[i];
                const double hj = step_sizes[j];
                off_index.emplace_back(i, j);
                off_points.push_back(perturb2(x_min, i, hi, j, hj));
                off_points.push_back(perturb2(x_min, i, hi, j, -hj));
                off_points.push_back(perturb2(x_min, i, -hi, j, hj));
                off_points.push_back(perturb2(x_min, i, -hi, j, -hj));
            }
        }

        std::vector<std::vector<double>> hessian(n, std::vector<double>(n, 0.));
        for(std::size_t j = 0; j < n; ++j) {
            hessian[j][j] = hessian_diag[j];
        }

        if(not off_points.empty()) {
            const std::vector<double> off_values = eval_fn(off_points);
            result.n_evaluations += off_points.size();
            for(std::size_t k = 0; k < off_index.size(); ++k) {
                const std::size_t i = off_index[k].first;
                const std::size_t j = off_index[k].second;
                const double hi = step_sizes[i];
                const double hj = step_sizes[j];
                const double v =
                    (off_values[4 * k] - off_values[4 * k + 1] - off_values[4 * k + 2] +
                     off_values[4 * k + 3]) /
                    (4. * hi * hj);
                hessian[i][j] = v;
                hessian[j][i] = v;
            }
        }

        // Covariance V = 2 * UP * H^-1 (the factor 2 follows from the 1/2 in the quadratic expansion
        // F ~ F_min + 1/2 dx^T H dx; with UP it matches the MINUIT chi^2 convention).
        std::vector<std::vector<double>> inv;
        if(invertMatrix(hessian, inv)) {
            result.covariance.assign(n, std::vector<double>(n, 0.));
            for(std::size_t i = 0; i < n; ++i) {
                for(std::size_t j = 0; j < n; ++j) {
                    result.covariance[i][j] = 2. * opts.up * inv[i][j];
                }
            }
            // Profiled (correlation-aware) errors from the covariance diagonal.
            bool all_positive = true;
            for(std::size_t j = 0; j < n; ++j) {
                const double var = result.covariance[j][j];
                if(var > 0.) {
                    result.parameter_errors[j] = std::sqrt(var);
                }
                else {
                    all_positive = false;
                }
            }
            result.covariance_valid = all_positive;
        }
    }

    return result;
}

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */
