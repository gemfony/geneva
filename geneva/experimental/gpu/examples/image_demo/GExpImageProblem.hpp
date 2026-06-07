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
#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * Shared math for the EXPERIMENTAL image-fitness problem. These are pure host functions (no Geneva
 * dependencies) used by BOTH the individual's fitnessCalculation (CPU) and the marshaller's
 * hostEvaluate (CPU reference). The CUDA/OpenCL kernels in kernels/ replicate the SAME formulas
 * (renderPixel / edgeSign / pointInTriangle) verbatim in device code, so a GPU run and a CPU run
 * produce identical fitness. Everything is double precision for tight parity.
 *
 * Problem: a W x H canvas; each individual encodes T triangles, 10 parameters each, all in [0,1]:
 *   [x0,y0, x1,y1, x2,y2, r,g,b, alpha]. The image is rendered by painter's-algorithm alpha blending
 *   of the triangles (in order) over a constant background. Fitness = sum over pixels of the squared
 *   RGB difference to a TARGET image, which is generated once by rendering a fixed ground-truth
 *   individual (so the global optimum is ~0).
 */
namespace Gem::Courtier::GPU::ImageDemo {

constexpr int IMG_W = 48;                 ///< canvas width
constexpr int IMG_H = 48;                 ///< canvas height
constexpr int IMG_T = 12;                 ///< triangles per individual
constexpr int PARAMS_PER_TRIANGLE = 10;   ///< x0 y0 x1 y1 x2 y2 r g b a
constexpr int IMG_DIM = IMG_T * PARAMS_PER_TRIANGLE; ///< parameters per individual (120)
constexpr double BG_R = 1.0;              ///< background colour (white)
constexpr double BG_G = 1.0;
constexpr double BG_B = 1.0;

/** @brief Signed area of the triangle (a,b,c)*2 -- used for the point-in-triangle test. */
inline double edgeSign(double ax, double ay, double bx, double by, double cx, double cy) {
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

/** @brief Whether point (px,py) is inside the triangle whose 3 vertices are tri[0..5]. */
inline bool pointInTriangle(double px, double py, const double *tri) {
    const double d1 = edgeSign(tri[0], tri[1], tri[2], tri[3], px, py);
    const double d2 = edgeSign(tri[2], tri[3], tri[4], tri[5], px, py);
    const double d3 = edgeSign(tri[4], tri[5], tri[0], tri[1], px, py);
    const bool hasNeg = (d1 < 0.0) || (d2 < 0.0) || (d3 < 0.0);
    const bool hasPos = (d1 > 0.0) || (d2 > 0.0) || (d3 > 0.0);
    return not(hasNeg && hasPos);
}

/** @brief Renders the colour at normalised pixel centre (cx,cy) for one individual's parameters. */
inline void renderPixel(const double *params, double cx, double cy,
                        double &outR, double &outG, double &outB) {
    double r = BG_R;
    double g = BG_G;
    double b = BG_B;
    for(int t = 0; t < IMG_T; ++t) {
        const double *tri = params + t * PARAMS_PER_TRIANGLE;
        if(pointInTriangle(cx, cy, tri)) {
            const double a = tri[9];
            r = a * tri[6] + (1.0 - a) * r;
            g = a * tri[7] + (1.0 - a) * g;
            b = a * tri[8] + (1.0 - a) * b;
        }
    }
    outR = r;
    outG = g;
    outB = b;
}

/** @brief Renders an individual's full image into outRGB (IMG_W*IMG_H*3 doubles). */
inline void renderImage(const double *params, double *outRGB) {
    for(int py = 0; py < IMG_H; ++py) {
        for(int px = 0; px < IMG_W; ++px) {
            const double cx = (px + 0.5) / static_cast<double>(IMG_W);
            const double cy = (py + 0.5) / static_cast<double>(IMG_H);
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;
            renderPixel(params, cx, cy, r, g, b);
            const int idx = (py * IMG_W + px) * 3;
            outRGB[idx] = r;
            outRGB[idx + 1] = g;
            outRGB[idx + 2] = b;
        }
    }
}

/** @brief Sum of squared per-pixel RGB differences between the individual's render and the target. */
inline double renderScore(const double *params, const double *targetRGB) {
    double sum = 0.0;
    for(int py = 0; py < IMG_H; ++py) {
        for(int px = 0; px < IMG_W; ++px) {
            const double cx = (px + 0.5) / static_cast<double>(IMG_W);
            const double cy = (py + 0.5) / static_cast<double>(IMG_H);
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;
            renderPixel(params, cx, cy, r, g, b);
            const int idx = (py * IMG_W + px) * 3;
            const double dr = r - targetRGB[idx];
            const double dg = g - targetRGB[idx + 1];
            const double db = b - targetRGB[idx + 2];
            sum += dr * dr + dg * dg + db * db;
        }
    }
    return sum;
}

/** @brief A fixed ground-truth parameter set (deterministic), whose render IS the target image. */
inline const std::vector<double> &groundTruthParams() {
    static const std::vector<double> gt = [] {
        std::vector<double> p(static_cast<std::size_t>(IMG_DIM));
        // Deterministic pseudo-random fill in [0,1] via a small LCG (no <random>, fully reproducible).
        std::uint64_t s = 0x9E3779B97F4A7C15ULL;
        for(auto &v : p) {
            s = s * 6364136223846793005ULL + 1442695040888963407ULL;
            v = static_cast<double>((s >> 11) & 0xFFFFFFFFFFFFFULL) / 9007199254740992.0; // [0,1)
        }
        return p;
    }();
    return gt;
}

/** @brief The target image (IMG_W*IMG_H*3 doubles), generated once from the ground-truth individual. */
inline const std::vector<double> &targetImage() {
    static const std::vector<double> target = [] {
        std::vector<double> img(static_cast<std::size_t>(IMG_W) * IMG_H * 3);
        renderImage(groundTruthParams().data(), img.data());
        return img;
    }();
    return target;
}

/** @brief Convenience: score an individual's parameters against the shared target image. */
inline double scoreParams(const double *params) {
    return renderScore(params, targetImage().data());
}

} /* namespace Gem::Courtier::GPU::ImageDemo */
