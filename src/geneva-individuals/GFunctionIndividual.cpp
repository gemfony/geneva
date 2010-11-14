/**
 * @file GFunctionIndividual.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "geneva-individuals/GFunctionIndividual.hpp"

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(Gem::Geneva::GFunctionIndividual<Gem::Geneva::PARABOLA>, "GFunctionIndividual_PARABOLA")
BOOST_CLASS_EXPORT_GUID(Gem::Geneva::GFunctionIndividual<Gem::Geneva::BERLICH>, "GFunctionIndividual_BERLICH")
BOOST_CLASS_EXPORT_GUID(Gem::Geneva::GFunctionIndividual<Gem::Geneva::ROSENBROCK>, "GFunctionIndividual_ROSENBROCK")
BOOST_CLASS_EXPORT_GUID(Gem::Geneva::GFunctionIndividual<Gem::Geneva::ACKLEY>, "GFunctionIndividual_ACKLEY")
BOOST_CLASS_EXPORT_GUID(Gem::Geneva::GFunctionIndividual<Gem::Geneva::RASTRIGIN>, "GFunctionIndividual_RASTRIGIN")
BOOST_CLASS_EXPORT_GUID(Gem::Geneva::GFunctionIndividual<Gem::Geneva::SCHWEFEL>, "GFunctionIndividual_SCHWEFEL")
BOOST_CLASS_EXPORT_GUID(Gem::Geneva::GFunctionIndividual<Gem::Geneva::SALOMON>, "GFunctionIndividual_SALOMON")

namespace Gem
{
namespace Geneva
{

//////////////////////////////////////////////////////////////////////////////////////////////////
// A number of specializations of fitnessCalculation() for the three function types

/************************************************************************************************/
/**
 * A simple, multi-dimensional parabola
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<PARABOLA>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x->size();
	double result = 0.;
	for(std::size_t i=0; i<parameterSize; i++) result += GSQUARED(x_ref[i]);
	return result;

	/*
	// It is now also possible to extract all double values in the following way:
	double result = 0;
	std::vector<double> parVec;
	this->streamline(parVec); // The same has been implemented for bool and boost::int32_t
	std::size_t parameterSize = parVec.size();
	for(std::size_t i=0; i<parameterSize; i++) {
		result += GSQUARED(parVec[i]);
	}
	return result;
	*/
	}

/************************************************************************************************/
/**
 * A "noisy" parabola, i.e. a parabola with a very large number of local optima
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<BERLICH>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x->size();
	double xsquared = 0.;
	for(std::size_t i=0; i<parameterSize; i++){
		xsquared += GSQUARED(x_ref[i]);
	}
	return (cos(xsquared) + 2.) * xsquared;
}

/************************************************************************************************/
/**
 * The generalized Rosenbrock function (see e.g. http://en.wikipedia.org/wiki/Rosenbrock_function)
 * or http://www.it.lut.fi/ip/evo/functions/node5.html .
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<ROSENBROCK>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x->size();
	double result = 0.;

#ifdef DEBUG
	// Check the size of the parameter vector -- must be at least 2
	if(parameterSize < 2) {
		std::ostringstream error;
		error << "In GFunctionIndividual<dF>::rosenbrock(): Error!" << std::endl
			  << "Need to use at least two input dimensions, but got " << parameterSize << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */

	for(std::size_t i=0; i<(parameterSize-1); i++) {
		result += 100.*GSQUARED(GSQUARED(x_ref[i]) - x_ref[i+1]) + GSQUARED(1.-x_ref[i]);
	}

	return result;
}

/************************************************************************************************/
/**
 * The Ackeley function (see e.g. http://www.it.lut.fi/ip/evo/functions/node14.html)
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<ACKLEY>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x->size();
	double result = 0.;

#ifdef DEBUG
	// Check the size of the parameter vector -- must be at least 2
	if(parameterSize < 2) {
		std::ostringstream error;
		error << "In GFunctionIndividual<dF>::ackeley(): Error!" << std::endl
			  << "Need to use at least two input dimensions, but got " << parameterSize << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}
#endif /* DEBUG */

	for(std::size_t i=0; i<(parameterSize-1); i++) {
		result += (exp(-0.2)*sqrt(GSQUARED(x_ref[i]) + GSQUARED(x_ref[i+1])) + 3.*(cos(2.*x_ref[i]) + sin(2.*x_ref[i+1])));
	}

	return result;
}

/************************************************************************************************/
/**
 * The Rastrigin function (see e.g. http://www.it.lut.fi/ip/evo/functions/node6.html)
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<RASTRIGIN>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x->size();
	double result = 10*double(parameterSize);

	for(std::size_t i=0; i<parameterSize; i++) {
		result += (GSQUARED(x_ref[i]) - 10.*cos(2*M_PI*x_ref[i]));
	}

	return result;
}

/************************************************************************************************/
/**
 * The Schwefel function (see e.g. http://www.it.lut.fi/ip/evo/functions/node10.html)
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<SCHWEFEL>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x->size();
	double result = 0.;

	for(std::size_t i=0; i<parameterSize; i++) {
		result += -x_ref[i]*sin(sqrt(fabs(x_ref[i])));
	}

	return result/parameterSize;
}

/************************************************************************************************/
/**
 * The Salomon function (see e.g. http://www.it.lut.fi/ip/evo/functions/node12.html)
 *
 * @return The result of the calculation
 */
template<> double GFunctionIndividual<SALOMON>::fitnessCalculation() {
	// Extract the GDoubleCollection object
	boost::shared_ptr<GDoubleCollection> x = at<GDoubleCollection>(0);
	const GDoubleCollection& x_ref = *x; // Avoid frequent dereferencing

	std::size_t parameterSize = x->size();

	double sum_root = 0.;
	for(std::size_t i=0; i<parameterSize; i++) {
		sum_root += GSQUARED(x_ref[i]);
	}
	sum_root=sqrt(sum_root);

	return -cos(2*M_PI*sum_root) + 0.1*sum_root + 1;
}

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
