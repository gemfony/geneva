/**
 * @file GTestIndividual3.cpp
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

#include "geneva-individuals/GTestIndividual3.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GTestIndividual3)

namespace Gem {
namespace Tests {

const std::size_t GTI_DEF_NITEMS = 300;
const double GTI_DEF_SIGMA = 0.025;
const double GTI_DEF_SIGMASIGMA = 0.1;
const double GTI_DEF_MINSIGMA = 0.001;
const double GTI_DEF_MAXSIGMA = 1.0;
const double GTI_DEF_ADPROB = 0.05;

/******************************************************************************/
/**
 * The default constructor
 */
GTestIndividual3::GTestIndividual3() : GParameterSet() {
	using namespace Gem::Geneva;

	/////////////////////////////////////////////////////////////////////////////
	// Create suitable adaptors

	// Gaussian distributed random numbers
	std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr_tmpl(
		new GDoubleGaussAdaptor(
			GTI_DEF_SIGMA
			, GTI_DEF_SIGMASIGMA
			, GTI_DEF_MINSIGMA
			, GTI_DEF_MAXSIGMA
		)
	);
	gdga_ptr_tmpl->setAdaptionProbability(GTI_DEF_ADPROB);

	/////////////////////////////////////////////////////////////////////////////
	// Set up a hierarchical data structure

	// Create one GParameterObjectCollection for each data item
	for (std::size_t i_cnt = 0; i_cnt < GTI_DEF_NITEMS; i_cnt++) {
		std::shared_ptr <GParameterObjectCollection> gpoc_ptr(new GParameterObjectCollection());

		//--------------------------------------------------------------------------------------------
		std::shared_ptr <GConstrainedDoubleCollection> a_ptr(new GConstrainedDoubleCollection(2, 0., 1.));
		a_ptr->addAdaptor(gdga_ptr_tmpl);
		gpoc_ptr->push_back(a_ptr);

		//--------------------------------------------------------------------------------------------
		std::shared_ptr <GConstrainedDoubleObject> b_ptr(new GConstrainedDoubleObject(0., 0.3));
		b_ptr->addAdaptor(gdga_ptr_tmpl);
		gpoc_ptr->push_back(b_ptr);

		//--------------------------------------------------------------------------------------------
		std::shared_ptr <GConstrainedDoubleCollection> c_ptr(new GConstrainedDoubleCollection(3, 0., 1.));
		c_ptr->addAdaptor(gdga_ptr_tmpl);
		gpoc_ptr->push_back(c_ptr);

		//--------------------------------------------------------------------------------------------
		std::shared_ptr <GConstrainedDoubleCollection> d_ptr(new GConstrainedDoubleCollection(3, 0., 1.));
		d_ptr->addAdaptor(gdga_ptr_tmpl);
		gpoc_ptr->push_back(d_ptr);

		//--------------------------------------------------------------------------------------------
		std::shared_ptr <GConstrainedDoubleObject> e_ptr(new GConstrainedDoubleObject(0.3, 0.6));
		e_ptr->addAdaptor(gdga_ptr_tmpl);
		gpoc_ptr->push_back(e_ptr);

		//--------------------------------------------------------------------------------------------

		// Finally add the collection to the individual
		this->push_back(gpoc_ptr);
	}
}

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A constant reference to another GTestIndividual3 object
 */
GTestIndividual3::GTestIndividual3(const GTestIndividual3 &cp)
	: Gem::Geneva::GParameterSet(cp)
{   /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GTestIndividual3::~GTestIndividual3()
{ /* nothing */   }

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GTestIndividual3::compare(
	const GObject &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const {
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are dealing with a GTestIndividual3 reference independent of this object and convert the pointer
	const GTestIndividual3 *p_load = Gem::Common::g_convert_and_compare<GObject, GTestIndividual3>(cp, this);

	Gem::Common::GToken token("GTestIndividual3", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSet>(IDENTITY(*this, *p_load), token);

	// ...no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GTestIndividual3, camouflaged as a GObject.
 *
 * @param cp A copy of another GTestIndividual3, camouflaged as a GObject
 */
void GTestIndividual3::load_(const GObject *cp) {
	using namespace Gem::Common;
	using namespace Gem::Geneva;

	// Check that we are dealing with a GTestIndividual3 reference independent of this object and convert the pointer
	const GTestIndividual3 *p_load = Gem::Common::g_convert_and_compare<GObject, GTestIndividual3>(cp, this);

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
Gem::Geneva::GObject *GTestIndividual3::clone_() const {
	return new GTestIndividual3(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GTestIndividual3::fitnessCalculation() {
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
 * Get all data members of this class as a plain array
 */
std::shared_ptr <float> GTestIndividual3::getPlainData() const {
	using namespace Gem::Geneva;

#ifdef DEBUG
	if(this->size() != GTI_DEF_NITEMS) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GTestIndividual3::getPlainData(): Error!" << std::endl
				<< "Invalid number of entries in this class " << this->size() << " / " << GTI_DEF_NITEMS << std::endl
		);
	}
#endif /* DEBUG */

	// Note that we need to provide a deleter as we are dealing with an array. See e.g. http://stackoverflow.com/questions/13061979/shared-ptr-to-an-array-should-it-be-used
	std::shared_ptr <float> result(new float[10 * GTI_DEF_NITEMS], [](float *p) { delete[] p; });
	for (std::size_t i = 0; i < GTI_DEF_NITEMS; i++) {
		std::shared_ptr <GParameterObjectCollection> gpoc_ptr = this->at<GParameterObjectCollection>(i);

		//---------------------------------------------------------
		// Extract the data of the middle of the circle
		std::shared_ptr <GConstrainedDoubleCollection> a_ptr = gpoc_ptr->at<GConstrainedDoubleCollection>(0);
		(result.get())[i * 10 + 0] = boost::numeric_cast<float>(
			a_ptr->at(0)); // std::shared_ptr doesn't support subscripting, contrary to boost:shared_array
		(result.get())[i * 10 + 1] = boost::numeric_cast<float>(a_ptr->at(1));

		//---------------------------------------------------------
		std::shared_ptr <GConstrainedDoubleObject> b_ptr = gpoc_ptr->at<GConstrainedDoubleObject>(1);
		(result.get())[i * 10 + 2] = boost::numeric_cast<float>(b_ptr->value());

		//---------------------------------------------------------
		// Extract the three angles
		std::shared_ptr <GConstrainedDoubleCollection> c_ptr = gpoc_ptr->at<GConstrainedDoubleCollection>(2);
		(result.get())[i * 10 + 3] = boost::numeric_cast<float>(c_ptr->at(0));
		(result.get())[i * 10 + 4] = boost::numeric_cast<float>(c_ptr->at(1));
		(result.get())[i * 10 + 5] = boost::numeric_cast<float>(c_ptr->at(2));

		//---------------------------------------------------------
		// Extract the three colors
		std::shared_ptr <GConstrainedDoubleCollection> d_ptr = gpoc_ptr->at<GConstrainedDoubleCollection>(3);
		(result.get())[i * 10 + 6] = boost::numeric_cast<float>(d_ptr->at(0));
		(result.get())[i * 10 + 7] = boost::numeric_cast<float>(d_ptr->at(1));
		(result.get())[i * 10 + 8] = boost::numeric_cast<float>(d_ptr->at(2));

		//---------------------------------------------------------
		// Extract the alpha channel
		std::shared_ptr <GConstrainedDoubleObject> e_ptr = gpoc_ptr->at<GConstrainedDoubleObject>(4);
		(result.get())[i * 10 + 9] = boost::numeric_cast<float>(e_ptr->value());

		//---------------------------------------------------------
	}

	// Let the audience know
	return result;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual3::modify_GUnitTests() {
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
   Gem::Common::condnotset("GTestIndividual3::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual3::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests();

	const std::size_t NTESTS = 100;

	//------------------------------------------------------------------------------

	{ // Test that repeated extraction of an object's data results in the same output
		std::shared_ptr <GTestIndividual3> p;
		std::shared_ptr <float> result_old, result_new;

		BOOST_CHECK_NO_THROW(p = std::shared_ptr<GTestIndividual3>(new GTestIndividual3()));
		BOOST_CHECK_NO_THROW(result_old = p->getPlainData());
		for (std::size_t i = 0; i < NTESTS; i++) {
			BOOST_CHECK_NO_THROW(result_new = p->getPlainData());
			for (std::size_t m = 0; m < GTI_DEF_NITEMS * 10; i++) {
				BOOST_CHECK((result_old.get())[i] == (result_new.get())[i]); // std::shared_ptr doesn't support subscripting
			}
		}
	}

	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GTestIndividual3::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual3::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING

	using namespace Gem::Geneva;

	using boost::unit_test_framework::test_suite;
	using boost::unit_test_framework::test_case;

	// Call the parent classes' functions
	Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GTestIndividual3::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Tests */
} /* namespace Gem */
