/**
 * @file GInt32GaussAdaptor.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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

#include "geneva/GInt32GaussAdaptor.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GInt32GaussAdaptor)

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * The default constructor. Note that we need to use a different default value for sigma,
 * as there is a "natural" gap of 1 between integers, and the DEFAULTSIGMA might not be
 * suitable for us.
 */
GInt32GaussAdaptor::GInt32GaussAdaptor()
	: GNumGaussAdaptorT<boost::int32_t, double>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GInt32GaussAdaptor object
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(const GInt32GaussAdaptor& cp)
	: GNumGaussAdaptorT<boost::int32_t, double>(cp)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Initialization with a adaption probability.  Note that we need to use a different default
 * value for sigma, as there is a "natural" gap of 1 between integers, and the DEFAULTSIGMA
 * might not be suitable for us.
 *
 * @param adProb The adaption probability
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(const double& adProb)
	: GNumGaussAdaptorT<boost::int32_t, double>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA, adProb)
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
	: GNumGaussAdaptorT<boost::int32_t, double> (sigma, sigmaSigma, minSigma, maxSigma)
{ /* nothing */ }

/********************************************************************************************/
/**
 * This constructor lets a user set all sigma parameters, as well as the adaption
 * probability in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param maxSigma The maximal value allowed for sigma_
 */
GInt32GaussAdaptor::GInt32GaussAdaptor(const double& sigma, const double& sigmaSigma,
			        const double& minSigma, const double& maxSigma, const double& adProb)
	: GNumGaussAdaptorT<boost::int32_t, double> (sigma, sigmaSigma, minSigma, maxSigma, adProb)
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
	GInt32GaussAdaptor::load_(&cp);
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
	using namespace Gem::Common;
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
	using namespace Gem::Common;
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
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32GaussAdaptor>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GNumGaussAdaptorT<boost::int32_t, double>::checkRelationshipWith(cp, e, limit, "GInt32GaussAdaptor", y_name, withMessages));

	// no local data ...

	return evaluateDiscrepancies("GInt32GaussAdaptor", caller, deviations, e);
}

/*******************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GInt32GaussAdaptor object, camouflaged as a GObject
 */
void GInt32GaussAdaptor::load_(const GObject* cp){
    // Check that we are not accidently assigning this object to itself
    GObject::selfAssignmentCheck<GInt32GaussAdaptor>(cp);

	// Load our parent class'es data ...
	GNumGaussAdaptorT<boost::int32_t, double>::load_(cp);

	// ... no local data
}

/*******************************************************************************************/
/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
Gem::Geneva::adaptorId GInt32GaussAdaptor::getAdaptorId() const {
	return Gem::Geneva::GINT32GAUSSADAPTOR;
}

/* ----------------------------------------------------------------------------------
 * - Tested in GInt32GaussAdaptor::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/*******************************************************************************************/
/**
 * The actual adaption of the supplied value takes place here.
 *
 * @param value The value that is going to be adapted in situ
 */
void GInt32GaussAdaptor::customAdaptions(boost::int32_t &value) {
	// adapt the value in situ. Note that this changes
	// the argument of this function
#if defined (DEBUG)
	boost::int32_t addition = boost::numeric_cast<boost::int32_t>(gr->normal_distribution(sigma_));
#else
	boost::int32_t addition = static_cast<boost::int32_t>(gr->normal_distribution(sigma_));
#endif /* DEBUG */

	if(addition == 0) { // Enforce a minimal change of 1.
		gr->uniform_bool()?(addition=1):(addition=-1);
	}

#if defined (CHECKOVERFLOWS)
	// Prevent over- and underflows.
	if(value >= 0){
		if(addition >= 0 && (std::numeric_limits<boost::int32_t>::max()-value < addition)) {
#ifdef DEBUG
			std::cout << "Warning in GInt32GaussAdaptor::customAdaptions(): Had to change adaption due to overflow" << std::endl;
#endif
			addition *= -1;
		}
	}
	else { // < 0
		if(addition < 0 && (std::numeric_limits<boost::int32_t>::min()-value > addition)) {
#ifdef DEBUG
			std::cout << "Warning in GInt32GaussAdaptor::customAdaptions(): Had to change adaption due to underflow" << std::endl;
#endif
			addition *= -1;
		}
	}
#endif /* CHECKOVERFLOWS  */

	value += addition;
}

/* ----------------------------------------------------------------------------------
 * - Tested in GNumGaussAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

#ifdef GENEVATESTING
/*******************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GInt32GaussAdaptor::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GNumGaussAdaptorT<boost::int32_t, double>::modify_GUnitTests()) result = true;

	return result;
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GInt32GaussAdaptor::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GNumGaussAdaptorT<boost::int32_t, double>::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Check that the adaptor returns the correct adaptor id
		boost::shared_ptr<GInt32GaussAdaptor> p_test = this->clone<GInt32GaussAdaptor>();

		BOOST_CHECK_MESSAGE(
			p_test->getAdaptorId() == GINT32GAUSSADAPTOR
			,  "\n"
			<< "p_test->getAdaptorId() = " << p_test->getAdaptorId()
			<< "GINT32GAUSSADAPTOR     = " << GINT32GAUSSADAPTOR << "\n"
		);
	}

	//------------------------------------------------------------------------------
}

/*******************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GInt32GaussAdaptor::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GNumGaussAdaptorT<boost::int32_t, double>::specificTestsFailuresExpected_GUnitTests();
}

/*******************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
