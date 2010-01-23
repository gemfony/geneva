/**
 * @file GDoubleGaussAdaptor.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#include "GDoubleGaussAdaptor.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GDoubleGaussAdaptor)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GDoubleGaussAdaptor object
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const GDoubleGaussAdaptor& cp)
	: GGaussAdaptorT<double>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a mutation probability
 *
 * @param mutProb The mutation probability
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const double& mutProb)
	: GGaussAdaptorT<double>(mutProb)
{ /* nothing */ }

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param maxSigma The maximal value allowed for sigma_
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const double& sigma, const double& sigmaSigma,
			        const double& minSigma, const double& maxSigma)
	: GGaussAdaptorT<double> (sigma, sigmaSigma, minSigma, maxSigma)
{ /* nothing */ }

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters, as well as the mutation
 * probability in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param maxSigma The maximal value allowed for sigma_
 */
GDoubleGaussAdaptor::GDoubleGaussAdaptor(const double& sigma, const double& sigmaSigma,
			        const double& minSigma, const double& maxSigma, const double& mutProb)
	: GGaussAdaptorT<double> (sigma, sigmaSigma, minSigma, maxSigma, mutProb)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GDoubleGaussAdaptor::~GDoubleGaussAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GDoubleGaussAdaptor object
 * @return A constant reference to this object
 */
const GDoubleGaussAdaptor& GDoubleGaussAdaptor::operator=(const GDoubleGaussAdaptor& cp){
	GDoubleGaussAdaptor::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GDoubleGaussAdaptor::clone() const {
	return new GDoubleGaussAdaptor(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GDoubleGaussAdaptor object
 *
 * @param  cp A constant reference to another GDoubleGaussAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GDoubleGaussAdaptor::operator==(const GDoubleGaussAdaptor& cp) const {
	return GDoubleGaussAdaptor::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GDoubleGaussAdaptor object
 *
 * @param  cp A constant reference to another GDoubleGaussAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDoubleGaussAdaptor::operator!=(const GDoubleGaussAdaptor& cp) const {
	return !GDoubleGaussAdaptor::isEqualTo(cp, boost::logic::indeterminate);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GDoubleGaussAdaptor object.  If T is an object type,
 * then it must implement operator!= .
 *
 * @param  cp A constant reference to another GDoubleGaussAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GDoubleGaussAdaptor::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GDoubleGaussAdaptor *p_load = GObject::conversion_cast(&cp,  this);

	// Check equality of the parent class
	if(!GGaussAdaptorT<double>::isEqualTo(*p_load, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Checks for similarity with another GDoubleGaussAdaptor object.
 *
 * @param  cp A constant reference to another GDoubleGaussAdaptor object
 * @param limit A double value specifying the acceptable level of differences of floating point values
 * @return A boolean indicating whether both objects are similar to each other
 */
bool GDoubleGaussAdaptor::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

	// Check that we are indeed dealing with a GParamterT reference
	const GDoubleGaussAdaptor *p_load = GObject::conversion_cast(&cp,  this);

	// Check similarity of the parent class
	if(!GGaussAdaptorT<double>::isSimilarTo(*p_load, limit, expected)) return false;

	// No local data

	return true;
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GDoubleGaussAdaptor object, camouflaged as a GObject
 */
void GDoubleGaussAdaptor::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GDoubleGaussAdaptor *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GGaussAdaptorT<double>::load(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::GenEvA::adaptorId GDoubleGaussAdaptor::getAdaptorId() const {
	return Gem::GenEvA::GDOUBLEGAUSSADAPTOR;
}

/*******************************************************************************************/
/**
 * The actual mutation of the supplied value takes place here.
 *
 * @param value The value that is going to be mutated in situ
 */
void GDoubleGaussAdaptor::customMutations(double &value) {
	// adapt the value in situ. Note that this changes
	// the argument of this function
#if defined (CHECKOVERFLOWS)
	// Prevent over- and underflows. Note that we currently do not check the
	// size of "addition" in comparison to "value".
	double addition = this->gr.gaussRandom(0.,sigma_);

	if(value >= 0){
		if(addition >= 0. && (std::numeric_limits<double>::max()-value < addition)) {
#ifdef DEBUG
			std::cout << "Warning in GDoubleGaussAdaptor::customMutations(): Had to change mutation due to overflow" << std::endl;
#endif
			addition *= -1.;
		}
	}
	else { // < 0
		if(addition < 0. && (std::numeric_limits<double>::min()-value > addition)) {
#ifdef DEBUG
			std::cout << "Warning in GDoubleGaussAdaptor::customMutations(): Had to change mutation due to underflow" << std::endl;
#endif
			addition *= -1.;
		}
	}

	value += addition;
#else
	// We do not check for over- or underflows for performance reasons.
	value += this->gr.gaussRandom(0.,sigma_);
#endif /* CHECKOVERFLOWS */
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
