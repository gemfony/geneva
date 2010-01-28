/**
 * @file GInt32GaussAdaptor.cpp
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

#include "GInt32GaussAdaptor.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GInt32GaussAdaptor)

namespace Gem {
namespace GenEvA {

/*******************************************************************************************/
/**
 * The default constructor
 */
GInt32GaussAdaptor::GInt32GaussAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32GaussAdaptor object
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(const GInt32GaussAdaptor& cp)
	: GGaussAdaptorT<boost::int32_t>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a mutation probability
 *
 * @param mutProb The mutation probability
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(const double& mutProb)
	: GGaussAdaptorT<boost::int32_t>(mutProb)
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
GInt32GaussAdaptor::GInt32GaussAdaptor(const double& sigma, const double& sigmaSigma,
			        const double& minSigma, const double& maxSigma)
	: GGaussAdaptorT<boost::int32_t> (sigma, sigmaSigma, minSigma, maxSigma)
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
GInt32GaussAdaptor::GInt32GaussAdaptor(const double& sigma, const double& sigmaSigma,
			        const double& minSigma, const double& maxSigma, const double& mutProb)
	: GGaussAdaptorT<boost::int32_t> (sigma, sigmaSigma, minSigma, maxSigma, mutProb)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GInt32GaussAdaptor::~GInt32GaussAdaptor()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GInt32GaussAdaptor object
 * @return A constant reference to this object
 */
const GInt32GaussAdaptor& GInt32GaussAdaptor::operator=(const GInt32GaussAdaptor& cp){
	GInt32GaussAdaptor::load(&cp);
	return *this;
}

/*******************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
GObject* GInt32GaussAdaptor::clone_() const {
	return new GInt32GaussAdaptor(*this);
}

/*******************************************************************************************/
/**
 * Checks for equality with another GInt32GaussAdaptor object
 *
 * @param  cp A constant reference to another GInt32GaussAdaptor object
 * @return A boolean indicating whether both objects are equal
 */
bool GInt32GaussAdaptor::operator==(const GInt32GaussAdaptor& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GInt32GaussAdaptor::operator==","cp", CE_SILENT);
}

/*******************************************************************************************/
/**
 * Checks for inequality with another GInt32GaussAdaptor object
 *
 * @param  cp A constant reference to another GInt32GaussAdaptor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GInt32GaussAdaptor::operator!=(const GInt32GaussAdaptor& cp) const {
	using namespace Gem::Util;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GInt32GaussAdaptor::operator!=","cp", CE_SILENT);
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
boost::optional<std::string> GInt32GaussAdaptor::checkRelationshipWith(const GObject& cp,
		const Gem::Util::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Util;
    using namespace Gem::Util::POD;

	// Check that we are indeed dealing with a GParamterBase reference
	const GInt32GaussAdaptor  *p_load = GObject::conversion_cast(&cp,  this);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GGaussAdaptorT<boost::int32_t>::checkRelationshipWith(cp, e, limit, "GInt32GaussAdaptor", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GInt32GaussAdaptor", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32GaussAdaptor object, camouflaged as a GObject
 */
void GInt32GaussAdaptor::load(const GObject* cp){
	// Convert cp into local format (also checks for the type of cp)
	const GInt32GaussAdaptor *p_load = GObject::conversion_cast(cp, this);

	// Load our parent class'es data ...
	GGaussAdaptorT<boost::int32_t>::load(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::GenEvA::adaptorId GInt32GaussAdaptor::getAdaptorId() const {
	return Gem::GenEvA::GINT32GAUSSADAPTOR;
}

/*******************************************************************************************/
/**
 * The actual mutation of the supplied value takes place here.
 *
 * @param value The value that is going to be mutated in situ
 */
void GInt32GaussAdaptor::customMutations(boost::int32_t &value) {
	// adapt the value in situ. Note that this changes
	// the argument of this function
#if defined (CHECKOVERFLOWS)

#if defined (DEBUG)
	boost::int32_t addition = boost::numeric_cast<boost::int32_t>(this->gr.gaussRandom(0.,sigma_));
#else
	boost::int32_t addition = static_cast<boost::int32_t>(this->gr.gaussRandom(0.,sigma_));
#endif /* DEBUG */

	// Prevent over- and underflows.
	if(value >= 0){
		if(addition >= 0 && (std::numeric_limits<boost::int32_t>::max()-value < addition)) {
#ifdef DEBUG
			std::cout << "Warning in GInt32GaussAdaptor::customMutations(): Had to change mutation due to overflow" << std::endl;
#endif
			addition *= -1;
		}
	}
	else { // < 0
		if(addition < 0 && (std::numeric_limits<boost::int32_t>::min()-value > addition)) {
#ifdef DEBUG
			std::cout << "Warning in GInt32GaussAdaptor::customMutations(): Had to change mutation due to underflow" << std::endl;
#endif
			addition *= -1;
		}
	}

	value += addition;
#else  /* CHECKOVERFLOWS */
	// We do not check for over- or underflows for performance reasons.
#if defined (DEBUG)
	value += boost::numeric_cast<boost::int32_t>(this->gr.gaussRandom(0.,sigma_));
#else
	value += static_cast<boost::int32_t>(this->gr.gaussRandom(0.,sigma_));
#endif /* DEBUG */

#endif /* CHECKOVERFLOWS  */
}

/*******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
