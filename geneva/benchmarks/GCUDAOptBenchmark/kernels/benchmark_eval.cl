// OpenCL evaluation kernel for the GPU optimisation benchmark.
//
// Runtime-compiled by the courtier GPU consumer's OpenCL backend (set backend=opencl and point
// kernel_path at this file in config/GGPUConsumer.json). It mirrors kernels/benchmark_eval.cu and the
// shared CPU math in geneva/individuals/GBenchmarkFunctions.hpp; keep all three in sync.
//
// ABI: __kernel evaluate(params, n_items, dim, pconst, pconst_size, fitness, threads_per_item).
// Problem constant: a single int "function id" (Gem::Geneva::Benchmarks::FUNC_*, 0..14).

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define BM_PI 3.14159265358979323846
#define BM_E  2.71828182845904523536

double bm_parabola(__global const double *x, int n) {
    double r = 0.;
    for(int i = 0; i < n; ++i) { r += x[i] * x[i]; }
    return r;
}

double bm_noisyParabola(__global const double *x, int n) {
    double sq = 0.;
    for(int i = 0; i < n; ++i) { sq += x[i] * x[i]; }
    return (cos(sq) + 2.) * sq;
}

double bm_rosenbrock(__global const double *x, int n) {
    double r = 0.;
    for(int i = 0; i < n - 1; ++i) {
        double t = x[i + 1] - x[i] * x[i];
        double u = 1. - x[i];
        r += 100. * t * t + u * u;
    }
    return r;
}

double bm_ackley(__global const double *x, int n) {
    double r = 0.;
    for(int i = 0; i < n - 1; ++i) {
        double s = x[i] * x[i] + x[i + 1] * x[i + 1];
        r += exp(-0.2) * sqrt(s) + 3. * (cos(2. * x[i]) + sin(2. * x[i + 1]));
    }
    return r;
}

double bm_rastrigin(__global const double *x, int n) {
    double r = 10. * n;
    for(int i = 0; i < n; ++i) { r += x[i] * x[i] - 10. * cos(2. * BM_PI * x[i]); }
    return r;
}

double bm_schwefel(__global const double *x, int n) {
    double r = 0.;
    for(int i = 0; i < n; ++i) { r += -x[i] * sin(sqrt(fabs(x[i]))); }
    return r / n;
}

double bm_salomon(__global const double *x, int n) {
    double sq = 0.;
    for(int i = 0; i < n; ++i) { sq += x[i] * x[i]; }
    const double r = sqrt(sq);
    return -cos(2. * BM_PI * r) + 0.1 * r + 1.;
}

double bm_ackleyCanonical(__global const double *x, int n) {
    double sq = 0., cs = 0.;
    for(int i = 0; i < n; ++i) {
        sq += x[i] * x[i];
        cs += cos(2. * BM_PI * x[i]);
    }
    const double inv_n = 1. / n;
    return -20. * exp(-0.2 * sqrt(sq * inv_n)) - exp(cs * inv_n) + 20. + BM_E;
}

double bm_griewank(__global const double *x, int n) {
    double sq = 0., prod = 1.;
    for(int i = 0; i < n; ++i) {
        sq += x[i] * x[i];
        prod *= cos(x[i] / sqrt((double)(i + 1)));
    }
    return sq / 4000. - prod + 1.;
}

double bm_levy(__global const double *x, int n) {
    const double w0 = 1. + (x[0] - 1.) / 4.;
    double r = sin(BM_PI * w0) * sin(BM_PI * w0);
    for(int i = 0; i < n - 1; ++i) {
        const double wi = 1. + (x[i] - 1.) / 4.;
        const double wi1 = 1. + (x[i + 1] - 1.) / 4.;
        const double sm = sin(BM_PI * wi1);
        r += (wi - 1.) * (wi - 1.) * (1. + 10. * sm * sm);
    }
    const double wn = 1. + (x[n - 1] - 1.) / 4.;
    const double sn = sin(2. * BM_PI * wn);
    r += (wn - 1.) * (wn - 1.) * (1. + sn * sn);
    return r;
}

double bm_styblinskiTang(__global const double *x, int n) {
    double r = 0.;
    for(int i = 0; i < n; ++i) {
        const double xi = x[i];
        const double x2 = xi * xi;
        r += x2 * x2 - 16. * x2 + 5. * xi;
    }
    return 0.5 * r;
}

double bm_ellipsoid(__global const double *x, int n) {
    double r = 0.;
    for(int i = 0; i < n; ++i) {
        const double e = (n > 1) ? 6. * i / (n - 1.) : 0.;
        r += pow(10., e) * x[i] * x[i];
    }
    return r;
}

double bm_michalewicz(__global const double *x, int n) {
    double r = 0.;
    for(int i = 0; i < n; ++i) {
        const double s = sin((double)(i + 1) * x[i] * x[i] / BM_PI);
        const double s2 = s * s;
        const double s20 = s2 * s2 * s2 * s2 * s2 * s2 * s2 * s2 * s2 * s2; // s^20
        r -= sin(x[i]) * s20;
    }
    return r;
}

double bm_zakharov(__global const double *x, int n) {
    double sq = 0., lin = 0.;
    for(int i = 0; i < n; ++i) {
        sq += x[i] * x[i];
        lin += 0.5 * (i + 1) * x[i];
    }
    return sq + lin * lin + lin * lin * lin * lin;
}

double bm_eval(int fid, __global const double *x, int n) {
    switch(fid) {
    case 0:  return bm_parabola(x, n);
    case 1:  return bm_noisyParabola(x, n);
    case 2:  return bm_rosenbrock(x, n);
    case 3:  return bm_ackley(x, n);
    case 4:  return bm_rastrigin(x, n);
    case 5:  return bm_schwefel(x, n);
    case 6:  return bm_salomon(x, n);
    case 7:  return -bm_parabola(x, n);
    case 8:  return bm_ackleyCanonical(x, n);
    case 9:  return bm_griewank(x, n);
    case 10: return bm_levy(x, n);
    case 11: return bm_styblinskiTang(x, n);
    case 12: return bm_ellipsoid(x, n);
    case 13: return bm_michalewicz(x, n);
    case 14: return bm_zakharov(x, n);
    default: return 0.;
    }
}

__kernel void evaluate(
    __global const double *params, int n_items, int dim,
    __global const uchar *pconst, int pconst_size,
    __global double *fitness,
    int threads_per_item)
{
    const int i = get_global_id(0);
    if(i >= n_items) {
        return;
    }
    int fid = 0;
    if(pconst_size >= (int)sizeof(int)) {
        fid = *(__global const int *)(pconst);
    }
    __global const double *x = params + (long)i * dim;
    fitness[i] = bm_eval(fid, x, dim);
}
