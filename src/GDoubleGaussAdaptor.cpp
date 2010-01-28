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
GObject* GDoubleGaussAdaptor::clone_() const {
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
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GDoubleGaussAdaptor::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GDoubleGaussAdaptor object
 *
 * @param  cp A constant reference to another GDoubleGaussAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GDoubleGaussAdaptor::operator!=(const GDoubleGaussAdaptor& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GDoubleGaussAdaptor::operator!=","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GDoubleGaussAdaptor::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GDoubleGaussAdaptor  *p_load = GObject::conversion_cast(&cp,  this);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GGaussAdaptorT<double>::checkRelationshipWith(cp, e, limit, "GDoubleGaussAdaptor", y_name, withMessages));

	// no local data ...

	return POD::evaluateDiscrepancies("GDoubleGaussAdaptor", caller, deviations, e);
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
