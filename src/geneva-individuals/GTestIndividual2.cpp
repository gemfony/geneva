/**
 * @file GTestIndividual2.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#include "geneva-individuals/GTestIndividual2.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GTestIndividual2)

namespace Gem {
namespace Tests {

/******************************************************************************/
/**
 * The default constructor -- private, as it is only needed for (de-)serialization purposes
 */
GTestIndividual2::GTestIndividual2()
	: GParameterSet() { /* nothing */ }

/******************************************************************************/
/**
 * The standard constructor
 *
 * @param nObjects The number of parameters to be added to this individual
 */
GTestIndividual2::GTestIndividual2(const std::size_t &nObjects, const PERFOBJECTTYPE &otype)
	: GParameterSet() {
	using namespace Gem::Geneva;

	// Fill with the requested amount of data of the requested type
	switch (otype) {
		case PERFGDOUBLEOBJECT: {
			for (std::size_t i = 0; i < nObjects; i++) {
				std::shared_ptr <GDoubleObject> gdo_ptr(new GDoubleObject(0.));
				std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.));
				gdo_ptr->addAdaptor(gdga_ptr);
				this->push_back(gdo_ptr);
			}
			break;
		}

		case PERFGCONSTRDOUBLEOBJECT: {
			for (std::size_t i = 0; i < nObjects; i++) {
				std::shared_ptr <GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(0., -10., 10.));
				std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.));
				gcdo_ptr->addAdaptor(gdga_ptr);
				this->push_back(gcdo_ptr);
			}
			break;
		}

		case PERFGCONSTRAINEDDOUBLEOBJECTCOLLECTION: {
			std::shared_ptr <GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(0., -10., 10.));
			std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.));
			gcdo_ptr->addAdaptor(gdga_ptr);
			std::shared_ptr <GConstrainedDoubleObjectCollection> gcdc_ptr(
				new GConstrainedDoubleObjectCollection(nObjects, gcdo_ptr));
			this->push_back(gcdc_ptr);
			break;
		}

		case PERFGDOUBLECOLLECTION: {
			std::shared_ptr <GDoubleCollection> gdc_ptr(new GDoubleCollection(nObjects, 0., -10., 10.));
			std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.));
			gdc_ptr->addAdaptor(gdga_ptr);
			this->push_back(gdc_ptr);
			break;
		}

		case PERFGCONSTRAINEDDOUBLECOLLECTION: {
			std::shared_ptr <GConstrainedDoubleCollection> gcdc_ptr(
				new GConstrainedDoubleCollection(nObjects, 0., -10., 10.));
			std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(0.025, 0.1, 0., 1., 1.));
			gcdc_ptr->addAdaptor(gdga_ptr);
			this->push_back(gcdc_ptr);
			break;
		}

		default: {
			glogger
			<< "In GTestIndividual2::GTestIndividual2(): Error!" << std::endl
			<< "Invalid object type requested: " << otype << std::endl
			<< GTERMINATION;
			break;
		}
	}
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GTestIndividual2 object
 */
GTestIndividual2::GTestIndividual2(const GTestIndividual2 &cp)
	: Gem::Geneva::GParameterSet(cp) {   /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GTestIndividual2::~GTestIndividual2() { /* nothing */   }

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GTestIndividual2 object
 * @return A constant reference to this object
 */
const GTestIndividual2 &GTestIndividual2::operator=(const GTestIndividual2 &cp) {
	GTestIndividual2::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GTestIndividual2 object
 *
 * @param  cp A constant reference to another GTestIndividual2 object
 * @return A boolean indicating whether both objects are equal
 */
bool GTestIndividual2::operator==(const GTestIndividual2 &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GTestIndividual2 object
 *
 * @param  cp A constant reference to another GTestIndividual2 object
 * @return A boolean indicating whether both objects are in-equal
 */
bool GTestIndividual2::operator!=(const GTestIndividual2 &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GTestIndividual2::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GBaseEA reference
	const GTestIndividual2 *p_load = GObject::gobject_conversion<GTestIndividual2>(&cp);

	Gem::Common::GToken token("GTestIndividual2", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSet>(IDENTITY(*this, *p_load), token);

	// ...no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GTestIndividual2, camouflaged as a GObject.
 *
 * @param cp A copy of another GTestIndividual2, camouflaged as a GObject
 */
void GTestIndividual2::load_(const GObject *cp) {
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are indeed dealing with a GTestIndividual2 object
	const GTestIndividual2 *p_load = gobject_conversion<GTestIndividual2>(cp);

	// Load our parent's data
	GParameterSet::load_(cp);

	// no local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject *GTestIndividual2::clone_() const {
	return new GTestIndividual2(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GTestIndividual2::fitnessCalculation() {
	double result = 0.;

	// We just calculate the square of all double values
	std::vector<double> parVec;
	this->streamline(parVec);

	// Calculate the value of the parabola
	for (std::size_t i = 0; i < parVec.size(); i++) {
		result += GSQUARED(parVec[i]);
	}

	return result;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual2::modify_GUnitTests() {
#ifdef GEM_TESTING
	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	bool result = false;

	// Call the parent classes' functions
	if (Gem::Geneva::GParameterSet::modify_GUnitTests()) result = true;

	// Change the parameter settings
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GTestIndividual2::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual2::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GTestIndividual2::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual2::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GTestIndividual2::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Tests */
} /* namespace Gem */
