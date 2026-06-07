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

#pragma once

// Standard headers
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>

// Geneva headers
#include "GImageHelperFunctions.hpp"

/**
 * Shared CPU math for the Mona-Lisa problem of example 15. The fitness of a candidate is the
 * deviation of the alpha-blended triangle superimposition from a TARGET image, summed per pixel and
 * channel through a rational saturation function -- exactly the metric example 15's GPU kernel uses.
 *
 * These pure host functions are shared by the individual's fitnessCalculation (CPU) and the
 * marshaller's hostEvaluate (CPU reference); the runtime CUDA kernel (kernels/monalisa_eval.cu)
 * replicates the SAME formulas, so a CPU run and a GPU run give the same fitness and the GPU can be
 * cross-checked against the CPU.
 *
 * NOTE: double precision is used throughout (the original example-15 kernel used float, for speed).
 * That choice is deliberate here: it makes the CPU/GPU cross-check tight (parity to ~1e-9), which is
 * the point of having a CPU equivalent. Speed comparisons remain meaningful.
 *
 * Genome layout (flat, == GParameterSet::streamline order; alpha-sort disabled so order is canonical):
 *   per triangle (10 doubles): cx, cy, radius, angle1, angle2, angle3, r, g, b, alpha   (all in [0,1])
 *   then the background colour (3 doubles): bgR, bgG, bgB.   dim = 10*nTriangles + 3.
 */
namespace Gem::Geneva::MonaLisa {

constexpr double RSF_FACTOR = 0.04; ///< rational-saturation-function factor (matches example 15)

/** @brief The target image, loaded once from a PNG: width/height and W*H*3 RGB doubles in [0,1]. */
struct Target {
    int width = 0;
    int height = 0;
    std::vector<double> rgb; ///< size width*height*3
};

/** @brief The process-wide target (set once before any evaluation). */
inline Target &mutableTarget() {
    static Target t;
    return t;
}
inline const Target &target() { return mutableTarget(); }

/** @brief Loads @p path (PNG) into the process-wide target. Throws on failure. */
inline void loadTarget(const std::string &path) {
    std::vector<float> rgbf;
    int w = 0;
    int h = 0;
    if(not Gem::Common::loadImageToFloat(path, rgbf, w, h)) {
        throw std::runtime_error("GMonaLisaProblem: could not load target image '" + path + "'");
    }
    Target &t = mutableTarget();
    t.width = w;
    t.height = h;
    t.rgb.assign(rgbf.begin(), rgbf.end());
}

/** @brief The three corner coordinates of a circle-triangle, scaled to image space. */
inline void calculateCorners(const double *tri, int width, int height, double *out6) {
    const double cx = tri[0] * width;
    const double cy = tri[1] * height;
    const double radius = tri[2];
    const double scale = static_cast<double>(width < height ? width : height);
    const double twoPi = 2.0 * std::numbers::pi_v<double>;
    for(int k = 0; k < 3; ++k) {
        const double ang = tri[3 + k] * twoPi;
        out6[k * 2 + 0] = cx + radius * std::cos(ang) * scale;
        out6[k * 2 + 1] = cy + radius * std::sin(ang) * scale;
    }
}

/** @brief Point-in-triangle test (bounding-box reject + three edge signs), matching the kernel. */
inline bool pointInTriangle(double px, double py, const double *c) {
    const double x1 = c[0], y1 = c[1], x2 = c[2], y2 = c[3], x3 = c[4], y3 = c[5];
    const double minx = std::min(x1, std::min(x2, x3));
    const double maxx = std::max(x1, std::max(x2, x3));
    const double miny = std::min(y1, std::min(y2, y3));
    const double maxy = std::max(y1, std::max(y2, y3));
    if(px < minx || px > maxx || py < miny || py > maxy) {
        return false;
    }
    const double d1 = (px - x2) * (y1 - y2) - (py - y2) * (x1 - x2);
    const double d2 = (px - x3) * (y2 - y3) - (py - y3) * (x2 - x3);
    const double d3 = (px - x1) * (y3 - y1) - (py - y1) * (x3 - x1);
    const bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    const bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return not(hasNeg && hasPos);
}

/**
 * @brief Renders the flat genome @p params (dim = 10*NT+3) over a @p W x @p H canvas and returns the
 * rational-saturation deviation from @p targetRGB (W*H*3). Corners are precomputed once per triangle.
 */
inline double score(const double *params, int dim, int W, int H, const double *targetRGB,
                    std::vector<double> &cornerScratch) {
    const int NT = (dim - 3) / 10;
    const double bgR = params[10 * NT + 0];
    const double bgG = params[10 * NT + 1];
    const double bgB = params[10 * NT + 2];

    cornerScratch.resize(static_cast<std::size_t>(NT) * 6);
    for(int t = 0; t < NT; ++t) {
        calculateCorners(params + t * 10, W, H, cornerScratch.data() + t * 6);
    }

    double sum = 0.0;
    for(int y = 0; y < H; ++y) {
        for(int x = 0; x < W; ++x) {
            const double px = x + 0.5;
            const double py = y + 0.5;
            double r = bgR, g = bgG, b = bgB;
            for(int t = 0; t < NT; ++t) {
                if(pointInTriangle(px, py, cornerScratch.data() + t * 6)) {
                    const double *tri = params + t * 10;
                    const double a = tri[9];
                    r = (1.0 - a) * r + a * tri[6];
                    g = (1.0 - a) * g + a * tri[7];
                    b = (1.0 - a) * b + a * tri[8];
                }
            }
            const std::size_t idx = (static_cast<std::size_t>(y) * W + x) * 3;
            const double dr = targetRGB[idx] - r;
            const double dg = targetRGB[idx + 1] - g;
            const double db = targetRGB[idx + 2] - b;
            sum += dr * dr / (dr * dr + RSF_FACTOR);
            sum += dg * dg / (dg * dg + RSF_FACTOR);
            sum += db * db / (db * db + RSF_FACTOR);
        }
    }
    return sum;
}

/**
 * @brief Renders the flat genome @p params (dim = 10*NT+3) into an 8-bit RGB buffer (W*H*3 bytes,
 * row-major, 3 bytes per pixel) using the SAME alpha-blend rasterisation as score(). This is what the
 * image-output monitor saves to disk so the evolving candidate can be watched as a picture.
 */
inline void renderToRGB(const double *params, int dim, int W, int H,
                        std::vector<unsigned char> &rgb_out) {
    const int NT = (dim - 3) / 10;
    const double bgR = params[10 * NT + 0];
    const double bgG = params[10 * NT + 1];
    const double bgB = params[10 * NT + 2];

    std::vector<double> corners(static_cast<std::size_t>(NT) * 6);
    for(int t = 0; t < NT; ++t) {
        calculateCorners(params + t * 10, W, H, corners.data() + t * 6);
    }

    rgb_out.resize(static_cast<std::size_t>(W) * H * 3);
    auto toByte = [](double v) -> unsigned char {
        return static_cast<unsigned char>(std::clamp(v, 0.0, 1.0) * 255.0 + 0.5);
    };
    for(int y = 0; y < H; ++y) {
        for(int x = 0; x < W; ++x) {
            const double px = x + 0.5;
            const double py = y + 0.5;
            double r = bgR, g = bgG, b = bgB;
            for(int t = 0; t < NT; ++t) {
                if(pointInTriangle(px, py, corners.data() + t * 6)) {
                    const double *tri = params + t * 10;
                    const double a = tri[9];
                    r = (1.0 - a) * r + a * tri[6];
                    g = (1.0 - a) * g + a * tri[7];
                    b = (1.0 - a) * b + a * tri[8];
                }
            }
            const std::size_t idx = (static_cast<std::size_t>(y) * W + x) * 3;
            rgb_out[idx + 0] = toByte(r);
            rgb_out[idx + 1] = toByte(g);
            rgb_out[idx + 2] = toByte(b);
        }
    }
}

/** @brief Convenience: score a genome against the process-wide target. */
inline double scoreAgainstTarget(const double *params, int dim) {
    const Target &t = target();
    static thread_local std::vector<double> scratch;
    return score(params, dim, t.width, t.height, t.rgb.data(), scratch);
}

} /* namespace Gem::Geneva::MonaLisa */
