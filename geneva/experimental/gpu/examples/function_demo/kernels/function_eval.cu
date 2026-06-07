// CUDA evaluation kernel for the GPU-consumer function demo.
//
// This file is NOT compiled at build time: the GGPUConsumer's CUDA backend reads it at run time
// (its path comes from the config file) and compiles it with NVRTC. It must follow the framework's
// evaluation ABI: an extern "C" (un-mangled) entry that takes the flat row-major parameter buffer,
// the item count and dimension, an opaque problem-constant blob, and writes one fitness per item.
//
// Problem constant: a single int "function id" -- 0 = parabola (sum of squares), 1 = Rosenbrock.

extern "C" __global__ void evaluate(
    const double *params, int n_items, int dim,
    const unsigned char *pconst, int pconst_size,
    double *fitness,
    int threads_per_item)
{
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n_items) {
        return;
    }

    int fid = 0;
    if (pconst_size >= (int)sizeof(int)) {
        fid = *reinterpret_cast<const int *>(pconst);
    }

    const double *x = params + (long long)i * dim;
    double f = 0.0;
    if (fid == 1) { // Rosenbrock
        for (int j = 0; j < dim - 1; ++j) {
            const double a = x[j + 1] - x[j] * x[j];
            const double b = 1.0 - x[j];
            f += 100.0 * a * a + b * b;
        }
    } else { // parabola (sum of squares)
        for (int j = 0; j < dim; ++j) {
            f += x[j] * x[j];
        }
    }
    fitness[i] = f;
}
