// OpenCL evaluation kernel for the GPU-consumer function demo.
//
// Loaded and compiled at run time by the GGPUConsumer's OpenCL backend (clBuildProgram); its path
// comes from the config file. Same evaluation ABI as the CUDA kernel: flat row-major parameters,
// item count + dimension, an opaque problem-constant blob (here a single int function id), one
// fitness written per item. function id 0 = parabola (sum of squares), 1 = Rosenbrock.

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void evaluate(
    __global const double *params, int n_items, int dim,
    __global const uchar *pconst, int pconst_size,
    __global double *fitness)
{
    const int i = get_global_id(0);
    if (i >= n_items) {
        return;
    }

    int fid = 0;
    if (pconst_size >= (int)sizeof(int)) {
        fid = *((__global const int *)pconst);
    }

    __global const double *x = params + (long)i * dim;
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
