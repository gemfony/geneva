// CUDA image-fitness kernel for the experimental GPU image demo.
//
// Loaded and NVRTC-compiled at run time (path from the config). MINIMAL kernels / MAXIMAL bulk: ONE
// launch evaluates the whole batch, one thread per individual -- each thread renders its T triangles
// over the full W x H canvas and accumulates its scalar fitness (no atomics, no intermediate buffers).
// The math is identical to the host renderScore()/pointInTriangle() in GExpImageProblem.hpp, so the
// CPU fitnessCalculation and this kernel agree.
//
// problem-constant blob (doubles): [W, H, T, bgR, bgG, bgB, target(W*H*3)].
// per-item parameters (dim = T*10): [x0 y0 x1 y1 x2 y2 r g b a] per triangle.

extern "C" __global__ void evaluate(
    const double *params, int n_items, int dim,
    const unsigned char *pconst, int pconst_size,
    double *fitness)
{
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n_items) {
        return;
    }

    const double *pc = reinterpret_cast<const double *>(pconst);
    const int W = (int)pc[0];
    const int H = (int)pc[1];
    const int T = (int)pc[2];
    const double bgR = pc[3];
    const double bgG = pc[4];
    const double bgB = pc[5];
    const double *target = pc + 6;

    const double *p = params + (long long)i * dim;
    double sum = 0.0;

    for (int py = 0; py < H; ++py) {
        for (int px = 0; px < W; ++px) {
            const double cx = (px + 0.5) / (double)W;
            const double cy = (py + 0.5) / (double)H;
            double r = bgR, g = bgG, b = bgB;
            for (int t = 0; t < T; ++t) {
                const double *tri = p + t * 10;
                const double d1 = (tri[2] - tri[0]) * (cy - tri[1]) - (tri[3] - tri[1]) * (cx - tri[0]);
                const double d2 = (tri[4] - tri[2]) * (cy - tri[3]) - (tri[5] - tri[3]) * (cx - tri[2]);
                const double d3 = (tri[0] - tri[4]) * (cy - tri[5]) - (tri[1] - tri[5]) * (cx - tri[4]);
                const bool hasNeg = (d1 < 0.0) || (d2 < 0.0) || (d3 < 0.0);
                const bool hasPos = (d1 > 0.0) || (d2 > 0.0) || (d3 > 0.0);
                if (!(hasNeg && hasPos)) {
                    const double a = tri[9];
                    r = a * tri[6] + (1.0 - a) * r;
                    g = a * tri[7] + (1.0 - a) * g;
                    b = a * tri[8] + (1.0 - a) * b;
                }
            }
            const int idx = (py * W + px) * 3;
            const double dr = r - target[idx];
            const double dg = g - target[idx + 1];
            const double db = b - target[idx + 2];
            sum += dr * dr + dg * dg + db * db;
        }
    }
    fitness[i] = sum;
}
