#include <cmath>
#include "GRandom.hpp"

void boxMuller (const double& x1, const double& x2, double& y1, double& y2) {
    y1 = sqrt(-2*log(x1))*cos(2*M_PI*x2 );
    y2 = sqrt(-2*log(x1))*sin(2*M_PI*x2 );
}

void boxMullerPolar (const double& x1, const double& x2, double& y1, double& y2) {
	double z1, z2 , r;
	do {
		z1 = 2.0 * x1 - 1.0;
		z2 = 2.0 * x2 - 1.0;
		r = z1*z1 + z2*z2;
	}
	while(r >= 1.0);

	r = sqrt( (-2.0 * ln( r ) ) / r );

    y1 = z1 * w;
    y2 = z2 * w;
}
