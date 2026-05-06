/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/
#include "geneva/GBooleanAdaptor.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBooleanAdaptor) // NOLINT
namespace Gem::Geneva
{

	/******************************************************************************/
	/**
 * The default constructor
 */
	GBooleanAdaptor::GBooleanAdaptor()
		: GAdaptorT<bool>(DEFAULTBITADPROB)
	{ /* nothing */ }

	// Tested in this class

	/******************************************************************************/
	/**
 * Initialization with a adaption probability
 *
 * @param adProb The adaption probability
 */
	GBooleanAdaptor::GBooleanAdaptor(const double &adProb)
		: GAdaptorT<bool>(adProb)
	{ /* nothing */ }

	// Tested in this class

	/******************************************************************************/
	/**
 * Creates a deep clone of this object.
 *
 * @return A copy of this object, camouflaged as a GObject
 */
	GObject *GBooleanAdaptor::clone_() const {
		return new GBooleanAdaptor(*this);
	}

	/******************************************************************************/
	/**
 * Flip the value up or down by 1, depending on a random number
 */
	void GBooleanAdaptor::customAdaptions(
		bool &value
		, const bool &/*range*/
		, Gem::Hap::GRandomBase& /*gr*/
	) {
		value == true ? value = false : value = true;
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
	void GBooleanAdaptor::compare_(
		const GObject &cp
		, const Gem::Common::expectation &e
		, const double &/*limit*/
	) const {
		using namespace Gem::Common;

		// Check that we are dealing with a GBooleanAdaptor reference independent of this object and convert the pointer
		const GBooleanAdaptor *p_load = Gem::Common::g_convert_and_compare<GObject, GBooleanAdaptor >(cp, this);

		GToken token("GBooleanAdaptor", e);

		// Compare our parent data ...
		Gem::Common::compare_base_t<GAdaptorT<bool>>(*this, *p_load, token);

		// ... no local data

		// React on deviations from the expectation
		token.evaluate();
	}

	/***********************************************************************************/
	/**
 * Emits a name for this class / object
 */
	std::string GBooleanAdaptor::name_() const {
		return std::string("GBooleanAdaptor");
	}

	/***********************************************************************************/
	/**
 * Allows to randomly initialize parameter members. No local data, hence no
 * action taken.
 */
	bool GBooleanAdaptor::randomInit(Gem::Hap::GRandomBase&) {
		return false;
	}

	/******************************************************************************/
	/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GBooleanAdaptor object, camouflaged as a GObject
 */
	void GBooleanAdaptor::load_(const GObject *cp) {
		// Convert the pointer to our target type and check for self-assignment
		const GBooleanAdaptor * p_load = Gem::Common::g_convert_and_compare<GObject, GBooleanAdaptor >(cp, this);

		// Load our parent class'es data ...
		GAdaptorT<bool>::load_(cp);

		// ... no local data
	}

	/******************************************************************************/
	/**
 * Retrieves the id of this adaptor
 *
 * @return The id of this adaptor
 */
	Gem::Geneva::adaptorId GBooleanAdaptor::getAdaptorId_() const {
		return Gem::Geneva::adaptorId::GBOOLEANADAPTOR;
	}

	/* ----------------------------------------------------------------------------------
 * Tested in GBooleanAdaptor::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

	/******************************************************************************/
	/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
	bool GBooleanAdaptor::modify_GUnitTests_() {
#ifdef GEM_TESTING

		bool result = false;

		// Call the parent class'es function
		if (GAdaptorT<bool>::modify_GUnitTests_()) result = true;

		return result;
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GBooleanAdaptor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
	}

	/******************************************************************************/
	/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
	void GBooleanAdaptor::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING

		// Call the parent class'es function
		GAdaptorT<bool>::specificTestsNoFailureExpected_GUnitTests_();

		// --------------------------------------------------------------------------

		{ // Check default construction
			GBooleanAdaptor gba;
			INFO("\n"
				<< "gba.getAdaptionProbability() = " <<
				gba.getAdaptionProbability() << "\n"
				<< "DEFAULTADPROB = " << DEFAULTBITADPROB);
			CHECK(gba.getAdaptionProbability() == DEFAULTBITADPROB);
		}

		// --------------------------------------------------------------------------

		{ // Check construction with a given adaption probability
			const double TRIALADPROB = 0.1;
			GBooleanAdaptor gba(TRIALADPROB);
			INFO("\n"
				<< "gba.getAdaptionProbability() = " <<
				gba.getAdaptionProbability()
				<< "TRIALADPROB = " << TRIALADPROB);
			CHECK(gba.getAdaptionProbability() == TRIALADPROB);
		}

		// --------------------------------------------------------------------------

		{ // Check copy construction
			const double TRIALADPROB = 0.1;
			GBooleanAdaptor gba1(TRIALADPROB);
			GBooleanAdaptor gba2(gba1);
			INFO("\n"
				<< "gba2.getAdaptionProbability() = " <<
				gba2.getAdaptionProbability()
				<< "TRIALADPROB = " << TRIALADPROB);
			CHECK(gba2.getAdaptionProbability() == TRIALADPROB);
		}

		// --------------------------------------------------------------------------

		{ // Check that the adaptor returns the correct adaptor id
			std::shared_ptr <GBooleanAdaptor> p_test = this->clone<GBooleanAdaptor>();

			INFO("\n"
				<< "p_test->getAdaptorId() = " << p_test->getAdaptorId()
				<< "GBOOLEANADAPTOR        = " << adaptorId::GBOOLEANADAPTOR << "\n");
			CHECK(p_test->getAdaptorId() == adaptorId::GBOOLEANADAPTOR);
		}

		// --------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GBooleanAdaptor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/******************************************************************************/
	/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
	void GBooleanAdaptor::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING

		// Call the parent class'es function
		GAdaptorT<bool>::specificTestsFailuresExpected_GUnitTests_();

		// no local data - nothing to test

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   Gem::Common::condnotset("GBooleanAdaptor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/******************************************************************************/

} /* namespace Gem::Geneva */
