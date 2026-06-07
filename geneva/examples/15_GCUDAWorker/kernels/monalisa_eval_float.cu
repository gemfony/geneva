// CUDA Mona-Lisa fitness kernel for example 15 -- PIXEL-PARALLEL, FP32 version.
//
// Runtime-compiled by the GGPUConsumer's CUDA backend (NVRTC). The batch is evaluated in ONE launch
// of n_items * threads_per_item threads: each item is handled by `threads_per_item` cooperating
// threads, each rendering a STRIPE of the image's pixels and atomic-accumulating its partial deviation
// into the item's fitness. The backend zeroes the fitness buffer before launch. This gives strong GPU
// utilisation even for SMALL populations on a large canvas (where one-thread-per-individual would
// leave the GPU mostly idle); for large populations threads_per_item can be 1 and per-individual
// parallelism already saturates the device.
//
// This is the FP32 path: params/fitness/target are all float (the genome and the target image are
// float on the host too), so the whole device pipeline runs in single precision -- the actual FP32
// speedup. The math mirrors the host float score() in GMonaLisaProblem.hpp, so a CPU run and a GPU run
// agree closely (not bit-identical: the parallel-stripe atomic sum reassociates the per-pixel terms).
//
// problem-constant blob (floats): [W, H, target(W*H*3)].
// per-item parameters (dim = 10*NT + 3): per triangle [cx cy radius a1 a2 a3 r g b alpha], then bg[3].

#define MONALISA_MAXTRI 64   // example 15 uses 64 triangles; corners are cached per thread

extern "C" __global__ void evaluate(
    const float *params, int n_items, int dim,
    const unsigned char *pconst, int pconst_size,
    float *fitness,
    int threads_per_item)
{
    if (threads_per_item < 1) {
        threads_per_item = 1;
    }
    const long long gid = (long long)blockIdx.x * blockDim.x + threadIdx.x;
    const int item = (int)(gid / threads_per_item);
    const int stripe = (int)(gid % threads_per_item);
    if (item >= n_items) {
        return;
    }

    const float *pc = reinterpret_cast<const float *>(pconst);
    const int W = (int)pc[0];
    const int H = (int)pc[1];
    const float *target = pc + 2;

    const float *p = params + (long long)item * dim;
    int NT = (dim - 3) / 10;
    if (NT > MONALISA_MAXTRI) {
        NT = MONALISA_MAXTRI;
    }
    const float bgR = p[10 * ((dim - 3) / 10) + 0];
    const float bgG = p[10 * ((dim - 3) / 10) + 1];
    const float bgB = p[10 * ((dim - 3) / 10) + 2];

    const float scale = (float)(W < H ? W : H);
    const float twoPi = 2.0f * 3.14159265358979323846f;

    // Precompute the three corners of every triangle once (image space).
    float corners[MONALISA_MAXTRI * 6];
    for (int t = 0; t < NT; ++t) {
        const float *tri = p + t * 10;
        const float cx = tri[0] * W;
        const float cy = tri[1] * H;
        const float radius = tri[2];
        for (int k = 0; k < 3; ++k) {
            const float ang = tri[3 + k] * twoPi;
            corners[t * 6 + k * 2 + 0] = cx + radius * cosf(ang) * scale;
            corners[t * 6 + k * 2 + 1] = cy + radius * sinf(ang) * scale;
        }
    }

    // This thread renders the pixels p where (p % threads_per_item == stripe).
    const long long nPixels = (long long)W * H;
    float partial = 0.0f;
    for (long long pix = stripe; pix < nPixels; pix += threads_per_item) {
        const int x = (int)(pix % W);
        const int y = (int)(pix / W);
        const float px = x + 0.5f;
        const float py = y + 0.5f;
        float r = bgR, g = bgG, b = bgB;
        for (int t = 0; t < NT; ++t) {
            const float *c = corners + t * 6;
            const float x1 = c[0], y1 = c[1], x2 = c[2], y2 = c[3], x3 = c[4], y3 = c[5];
            const float minx = fminf(x1, fminf(x2, x3));
            const float maxx = fmaxf(x1, fmaxf(x2, x3));
            const float miny = fminf(y1, fminf(y2, y3));
            const float maxy = fmaxf(y1, fmaxf(y2, y3));
            if (px < minx || px > maxx || py < miny || py > maxy) {
                continue;
            }
            const float d1 = (px - x2) * (y1 - y2) - (py - y2) * (x1 - x2);
            const float d2 = (px - x3) * (y2 - y3) - (py - y3) * (x2 - x3);
            const float d3 = (px - x1) * (y3 - y1) - (py - y1) * (x3 - x1);
            const bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
            const bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
            if (!(hasNeg && hasPos)) {
                const float *tri = p + t * 10;
                const float a = tri[9];
                r = (1.0f - a) * r + a * tri[6];
                g = (1.0f - a) * g + a * tri[7];
                b = (1.0f - a) * b + a * tri[8];
            }
        }
        const long long idx = pix * 3;
        const float dr = target[idx] - r;
        const float dg = target[idx + 1] - g;
        const float db = target[idx + 2] - b;
        const float f = 0.04f;
        partial += dr * dr / (dr * dr + f);
        partial += dg * dg / (dg * dg + f);
        partial += db * db / (db * db + f);
    }

    atomicAdd(&fitness[item], partial);
}
