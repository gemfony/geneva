/**
 * @file GParameterSet.cpp
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

#include "geneva/GParameterSet.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GParameterSet)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * The default constructor.
 */
GParameterSet::GParameterSet()
	: GMutableSetT<Gem::Geneva::GParameterBase>()
  { /* nothing */ }

/************************************************************************************************************/
/**
 * The copy constructor. Note that we cannot rely on the operator=() of the vector
 * here, as we do not know the actual type of T objects.
 *
 * @param cp A copy of another GParameterSet object
 */
GParameterSet::GParameterSet(const GParameterSet& cp)
	: GMutableSetT<Gem::Geneva::GParameterBase>(cp)
	, eval_(cp.eval_)
  { /* nothing */ }

/************************************************************************************************************/
/**
 * The destructor
 */
GParameterSet::~GParameterSet()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * A Standard assignment operator
 *
 * @param cp A copy of another GParameterSet object
 * @return A constant reference to this object
 */
const GParameterSet& GParameterSet::operator=(const GParameterSet& cp){
	GParameterSet::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Checks for equality with another GParameterSet object
 *
 * @param  cp A constant reference to another GParameterSet object
 * @return A boolean indicating whether both objects are equal
 */
bool GParameterSet::operator==(const GParameterSet& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GParameterSet::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GParameterSet object
 *
 * @param  cp A constant reference to another GParameterSet object
 * @return A boolean indicating whether both objects are inequal
 */
bool GParameterSet::operator!=(const GParameterSet& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GParameterSet::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
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
boost::optional<std::string> GParameterSet::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
	using namespace Gem::Common;

	// Check that we are not accidently assigning this object to itself
	GObject::selfAssignmentCheck<GParameterSet>(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GMutableSetT<Gem::Geneva::GParameterBase>::checkRelationshipWith(cp, e, limit, "GParameterSet", y_name, withMessages));

	// ... and there is no local data

	return evaluateDiscrepancies("GParameterSet", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * Creates a deep clone of this object.
 *
 * @return A deep clone of this object
 */
GObject * GParameterSet::clone_() const {
	return new GParameterSet(*this);
}

/************************************************************************************************************/
/**
 * Loads the data of another GParameterSet object, camouflaged as a GObject.
 *
 * @param cp A copy of another GParameterSet object, camouflaged as a GObject
 */
void GParameterSet::load_(const GObject* cp){
	// Convert to local format
	const GParameterSet *p_load = this->conversion_cast<GParameterSet>(cp);

	// Load the parent class'es data
	GMutableSetT<Gem::Geneva::GParameterBase>::load_(cp);

	// Then load our local data - here the evaluation function (if any)
	// NOTE: THIS IS DANGEROUS WHEN OPERATING IN A MULTITHREADED ENVIRONMENT.
	// IT WILL ALSO NOT WORK IN A NETWORKED ENVIRONMENT
	eval_ = p_load->eval_;
}

/************************************************************************************************************/
/**
 * A wrapper for GParameterSet::customUpdateOnStall() that restricts parameter set updates to parents
 * in the case of evolutionary algorithms in DEBUG mode.
 *
 * @return A boolean indicating whether an update was performed and the individual has changed
 */
bool GParameterSet::updateOnStall() {
	switch (getPersonality()) {
	case NONE:
	case GD:
	case SWARM:
		break;

	case EA:
#ifdef DEBUG
	{
		// This function should only be called for parents. Check ...
		if(!getEAPersonalityTraits()->isParent()) {
			std::ostringstream error;
			error << "In GParameterSet::updateOnStall() (called for EA personality): Error!" << std::endl
					<< "This function should only be called for parent individuals." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
	}
#endif /* DEBUG */
	break;

	}

	return GIndividual::updateOnStall();
}

/* ----------------------------------------------------------------------------------
 * Throwing of an exception is tested in GTestIndividual1::specificTestsFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Registers an evaluation function with this class. Note that the function object
 * can not be serialized. Hence, in a networked optimization run, you need to derive
 * your own class from GParameterSet and specify an evaluation function.
 */
void GParameterSet::registerEvaluator(const boost::function<double (const GParameterSet&)>& eval){
	if(eval.empty()){ // empty function ?
		std::ostringstream error;
		error << "In GParameterSet::registerEvaluator(): Error" << std::endl
				<< "Received empty function" << std::endl;

		throw Gem::Common::gemfony_error_condition(error.str());
	}

	eval_ = eval;
}

/* ----------------------------------------------------------------------------------
 * Untested -- deprecated
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Allows to randomly initialize parameter members
 */
void GParameterSet::randomInit() {
	// Trigger random initialization of all our parameter objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->randomInit();
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Recursively initializes floating-point-based parameters with a given value. Allows e.g. to set all
 * floating point parameters to 0. "float" is used as the largest common denominator of float, double
 * and long double.
 *
 * @param val The value to be assigned to the parameters
 */
void GParameterSet::fpFixedValueInit(const float& val) {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpFixedValueInit(val);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Multiplies floating-point-based parameters with a given value.
 *
 * @param val The value to be multiplied with parameters
 * @param dummy A dummy parameter needed to ensure that fp_type is indeed a floating point value
 */
void GParameterSet::fpMultiplyBy(const float& val) {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpMultiplyBy(val);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Triggers multiplication of floating point parameters with a random floating point number in a given range
 *
 * @param min The lower boundary for random number generation
 * @param max The upper boundary for random number generation
 */
void GParameterSet::fpMultiplyByRandom(const float& min, const float& max) {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpMultiplyByRandom(min, max);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Triggers multiplication of floating point parameters with a random floating point number in the range [0,1[
 */
void GParameterSet::fpMultiplyByRandom() {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpMultiplyByRandom();
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Adds the floating point parameters of another GParameterSet object to this one
 *
 * @param cp A boost::shared_ptr to another GParameterSet object whose floating point parameters should be added to this one
 */
void GParameterSet::fpAdd(boost::shared_ptr<GParameterSet> p) {
	// Some error checking
	if(p->size() != this->size()) {
		std::ostringstream error;
		error << "In GParameterSet::fpAdd(): Error! " << std::endl
			  << "Sizes do not match: " << this->size() << " " << p->size() << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Loop over all GParameterBase objects in this and the other object
	GParameterSet::iterator it;
	GParameterSet::const_iterator cit;
	// Note that the GParameterBase objects need to accept a
	// boost::shared_ptr<GParameterBase>, contrary to the calling conventions
	// of this function.
	for(it=this->begin(), cit=p->begin(); it!=this->end(); ++it, ++cit) {
		(*it)->fpAdd(*cit);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Subtract the floating point parameters of another GParameterSet object from this one
 *
 * @param cp A boost::shared_ptr to another GParameterSet object whose floating point parameters should be subtracted from this one
 */
void GParameterSet::fpSubtract(boost::shared_ptr<GParameterSet> p) {
	// Some error checking
	if(p->size() != this->size()) {
		std::ostringstream error;
		error << "In GParameterSet::fpAdd(): Error! " << std::endl
			  << "Sizes do not match: " << this->size() << " " << p->size() << std::endl;
		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Loop over all GParameterBase objects in this and the other object
	GParameterSet::iterator it;
	GParameterSet::iterator it_p;
	// Note that the GParameterBase objects need to accept a
	// boost::shared_ptr<GParameterBase>, contrary to the calling conventions
	// of this function.
	for(it=this->begin(), it_p=p->begin(); it!=this->end(); ++it, ++it_p) {
		(*it)->fpSubtract(*it_p);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************/
/**
 * Provides easy access to parameters of type double
 *
 * @param parVec the vector in which the parameters will be stored
 */
void GParameterSet::streamline(std::vector<double>& parVec) const {
	// Make sure the vector is clean
	parVec.clear();

	// Loop over all GParameterBase objects. Each object
	// will add the values of its parameters to the vector,
	// if they comply with the type of the parameters to
	// be stored in the vector.
	GParameterSet::const_iterator cit;
	for(cit=this->begin(); cit!=this->end(); ++cit) {
		(*cit)->doubleStreamline(parVec);
	}
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************/
/**
 * Provides easy access to parameters of type boost::int32_t
 *
 * @param parVec the vector in which the parameters will be stored
 */
void GParameterSet::streamline(std::vector<boost::int32_t>& parVec) const {
	// Make sure the vector is clean
	parVec.clear();

	// Loop over all GParameterBase objects. Each object
	// will add the values of its parameters to the vector,
	// if they comply with the type of the parameters to
	// be stored in the vector.
	GParameterSet::const_iterator cit;
	for(cit=this->begin(); cit!=this->end(); ++cit) {
		(*cit)->int32Streamline(parVec);
	}
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************/
/**
 * Provides easy access to parameters of type bool
 *
 * @param parVec the vector in which the parameters will be stored
 */
void GParameterSet::streamline(std::vector<bool>& parVec) const {
	// Make sure the vector is clean
	parVec.clear();

	// Loop over all GParameterBase objects. Each object
	// will add the values of its parameters to the vector,
	// if they comply with the type of the parameters to
	// be stored in the vector.
	GParameterSet::const_iterator cit;
	for(cit=this->begin(); cit!=this->end(); ++cit) {
		(*cit)->booleanStreamline(parVec);
	}
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Updates the random number generators contained in this object's GParameterBase-derivatives
 */
void GParameterSet::updateRNGs() {
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->assignGRandomPointer(&(GMutableSetT<Gem::Geneva::GParameterBase>::gr));
	}
}

/* ----------------------------------------------------------------------------------
 * - Assigning a random number generator is tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Restores the local random number generators contained in this object's GParameterBase-derivatives
 */
void GParameterSet::restoreRNGs() {
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->resetGRandomPointer();
	}
}

/* ----------------------------------------------------------------------------------
 * - Restoring the random number generators is tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Checks whether all GParameterBase derivatives use local random number generators. The function will return
 * false if at least one object is found in this collection that does not use a local RNG.
 *
 * @return A boolean which indicates whether all objects in this collection use local random number generators
 */
bool GParameterSet::localRNGsUsed() const {
	bool result = true;

	GParameterSet::const_iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		if(!(*it)->usesLocalRNG()) result = false;
	}

	return result;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Checks whether all GParameterBase derivatives use the assigned random number generator. The function will return
 * false if at least one object is found in this collection that uses a local RNG.
 *
 * @return A boolean which indicates whether all objects in this collection use the assigned random number generator
 */
bool GParameterSet::assignedRNGUsed() const {
	bool result = true;

	GParameterSet::const_iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		if((*it)->usesLocalRNG()) result = false;
	}

	return result;
}

/* ----------------------------------------------------------------------------------
 * Tested in GParameterSet::specificTestsNoFailuresExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * Prevent shadowing of std::vector<GParameterBase>::at()
 *
 * @param pos The position of the item we aim to retrieve from the std::vector<GParameterBase>
 * @return The item we aim to retrieve from the std::vector<GParameterBase>
 */
boost::shared_ptr<Gem::Geneva::GParameterBase> GParameterSet::at(const std::size_t& pos) {
	return data.at(pos);
}

/* ----------------------------------------------------------------------------------
 * So far untested
 * ----------------------------------------------------------------------------------
 */

/************************************************************************************************************/
/**
 * The actual fitness calculation takes place here. Note that you need
 * to overload this function if you do not want to use the GEvaluator
 * mechanism.
 *
 * @return The newly calculated fitness of this object
 */
double GParameterSet::fitnessCalculation(){
	if(eval_.empty()){ // Has an evaluator been stored in this class ?
		std::ostringstream error;
		error << "In GParameterSet::fitnessCalculation(): Error" << std::endl
				<< "No evaluation function present" << std::endl;

		throw Gem::Common::gemfony_error_condition(error.str());
	}

	// Trigger the actual calculation
	return eval_(*this);
}

/* ----------------------------------------------------------------------------------
 * Untested -- deprecated. Will be replaced by the registration of a	 GEvaluator object
 * ----------------------------------------------------------------------------------
 */

/**********************************************************************************/
/**
 * The actual adaption operations. Easy, as we know that all objects
 * in this collection must implement the adapt() function, as they are
 * derived from the GMutableI class / interface.
 */
void GParameterSet::customAdaptions() {
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->adapt();
	}
}

/* ----------------------------------------------------------------------------------
 * Tested in GTestIndividual1::specificTestsNoFailureExpected_GUnitTests()
 * ----------------------------------------------------------------------------------
 */

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GParameterSet::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GMutableSetT<Gem::Geneva::GParameterBase>::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GParameterSet::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GMutableSetT<Gem::Geneva::GParameterBase>::specificTestsNoFailureExpected_GUnitTests();

	//---------------------------------------------------------------------

	{ // All tests below use the same, cloned collection
		// Some settings for the collection of tests below
		const double MINGCONSTRDOUBLE    = -4.;
		const double MAXGCONSTRDOUBLE    =  4.;
		const double MINGDOUBLE          = -5.;
		const double MAXGDOUBLE          =  5.;
		const double MINGDOUBLECOLL      = -3.;
		const double MAXGDOUBLECOLL      =  3.;
		const std::size_t NGDOUBLECOLL   = 10 ;
		const std::size_t FPLOOPCOUNT    =  5 ;
		const double FPFIXEDVALINITMIN   = -3.;
		const double FPFIXEDVALINITMAX   =  3.;
		const double FPMULTIPLYBYRANDMIN = -5.;
		const double FPMULTIPLYBYRANDMAX =  5.;
		const double FPADD               =  2.;
		const double FPSUBTRACT          =  2.;

		// Create a GParameterSet object as a clone of this object for further usage
		boost::shared_ptr<GParameterSet> p_test_0 = this->clone<GParameterSet>();
		// Clear the collection
		p_test_0->clear();
		// Make sure it is really empty
		BOOST_CHECK(p_test_0->empty());
		// Add some floating pount parameters
		for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
			p_test_0->push_back(boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(gr.uniform_real(MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE), MINGCONSTRDOUBLE, MAXGCONSTRDOUBLE)));
			p_test_0->push_back(boost::shared_ptr<GDoubleObject>(new GDoubleObject(gr.uniform_real(MINGDOUBLE,MAXGDOUBLE))));
			p_test_0->push_back(boost::shared_ptr<GDoubleCollection>(new GDoubleCollection(NGDOUBLECOLL, MINGDOUBLECOLL, MAXGDOUBLECOLL)));
		}

		// Attach a few other parameter types
		p_test_0->push_back(boost::shared_ptr<GConstrainedInt32Object>(new GConstrainedInt32Object(7, -10, 10)));
		p_test_0->push_back(boost::shared_ptr<GBooleanObject>(new GBooleanObject(true)));

		//-----------------------------------------------------------------

		{ // Test setting and resetting of the random number generator
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			// Distribute our own random number generator
			BOOST_CHECK_NO_THROW(p_test->updateRNGs());
			BOOST_CHECK(p_test->assignedRNGUsed() == true);

			// Restore the original generators in all objects in the container
			BOOST_CHECK_NO_THROW(p_test->restoreRNGs());
			BOOST_CHECK(p_test->localRNGsUsed() == true);
		}

		//-----------------------------------------------------------------

		{ // Test random initialization
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

			BOOST_CHECK_NO_THROW(p_test->randomInit());
			BOOST_CHECK(*p_test != *p_test_0);
		}

		//-----------------------------------------------------------------
		{ // Test initialization of all fp parameters with a fixed value
			for(double d=FPFIXEDVALINITMIN; d<FPFIXEDVALINITMAX; d+=1.) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Make sure the individual is clean by manually setting the dirty flag to false
				// We might not have any evaluation code, so we cannot just call the fitness() function
				// p_test->setDirtyFlag(false);

				// Initialize all fp-values with 0.
				p_test->fpFixedValueInit(d);

				// Make sure the dirty flag is set
				BOOST_CHECK(p_test->isDirty() == true);

				// Cross-check
				std::size_t counter = 0;
				for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() == d);
					counter++;
					BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == d);
					counter++;
					boost::shared_ptr<GDoubleCollection> p_gdc;
					BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
					for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
						BOOST_CHECK_MESSAGE (
								p_gdc->at(gdc_cnt) == d
								,  "\n"
								<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
								<< "expected " << d << "\n"
								<< "iteration = " << gdc_cnt << "\n"
						);
					}
					counter++;
				}

				// The int32 parameter should have stayed the same
				boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
				boost::shared_ptr<GConstrainedInt32Object> p_int32;
				BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK(*p_int32_0 == *p_int32);
				counter++;

				// Likewise, the boolean parameter should have stayed the same
				boost::shared_ptr<GBooleanObject> p_boolean_orig;
				boost::shared_ptr<GBooleanObject> p_boolean_cloned;
				BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
				BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
				BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
				counter++;
			}
		}

		//-----------------------------------------------------------------

		{ // Test multiplication of all fp parameters with a fixed value
			for(double d=-3; d<3; d+=1.) {
				// Create a GParameterSet object as a clone of p_test_0 for further usage
				boost::shared_ptr<GParameterSet> p_test = p_test_0->clone<GParameterSet>();

				// Make sure the individual is clean by manually setting the dirty flag to false
				// We might not have any evaluation code, so we cannot just call the fitness() function
				// p_test->setDirtyFlag(false);

				// Initialize all fp-values with FPFIXEDVALINITMAX
				BOOST_CHECK_NO_THROW(p_test->fpFixedValueInit(FPFIXEDVALINITMAX));

				// Multiply this fixed value by d
				BOOST_CHECK_NO_THROW(p_test->fpMultiplyBy(d));

				// Make sure the dirty flag is set
				BOOST_CHECK(p_test->isDirty() == true);

				// Cross-check
				std::size_t counter = 0;
				for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
					// A constrained value does not have to assume the value d*FPFIXEDVALINITMAX,
					// but needs to stay within its boundaries
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
					BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
					counter++;
					BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == d*FPFIXEDVALINITMAX);
					counter++;
					boost::shared_ptr<GDoubleCollection> p_gdc;
					BOOST_CHECK_NO_THROW(p_gdc = p_test->at<GDoubleCollection>(counter));
					for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
						BOOST_CHECK_MESSAGE (
								p_gdc->at(gdc_cnt) == d*FPFIXEDVALINITMAX
								,  "\n"
								<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
								<< "expected " << d*FPFIXEDVALINITMAX << "\n"
								<< "iteration = " << gdc_cnt << "\n"
						);
					}
					counter++;
				}

				// The int32 parameter should have stayed the same
				boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
				boost::shared_ptr<GConstrainedInt32Object> p_int32;
				BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
				BOOST_CHECK(*p_int32_0 == *p_int32);
				counter++;

				// Likewise, the boolean parameter should have stayed the same
				boost::shared_ptr<GBooleanObject> p_boolean_orig;
				boost::shared_ptr<GBooleanObject> p_boolean_cloned;
				BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
				BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
				BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
				counter++;
			}
		}

		//-----------------------------------------------------------------

		{ // Test that fpMultiplyByRandom(min,max) changes every single parameter
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test       = p_test_0->clone<GParameterSet>();

			// Make sure the individual is clean by manually setting the dirty flag to false
			// We might not have any evaluation code, so we cannot just call the fitness() function
			// p_test->setDirtyFlag(false);

			// Multiply each floating point value with a constrained random value
			BOOST_CHECK_NO_THROW(p_test->fpMultiplyByRandom(FPMULTIPLYBYRANDMIN, FPMULTIPLYBYRANDMAX));

			// Make sure the dirty flag is set
			BOOST_CHECK(p_test->isDirty() == true);

			// Cross-check
			std::size_t counter = 0;
			for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() != p_test_0->at<GConstrainedDoubleObject>(counter)->value());
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() != p_test_0->at<GDoubleObject>(counter)->value());
				counter++;
				boost::shared_ptr<GDoubleCollection> p_gdc;
				boost::shared_ptr<GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc   = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) != p_gdc_0->at(gdc_cnt)
							,  "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
							<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
			boost::shared_ptr<GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			boost::shared_ptr<GBooleanObject> p_boolean_orig;
			boost::shared_ptr<GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Test that fpMultiplyByRandom() changes every single parameter
			// Create a GParameterSet object as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test       = p_test_0->clone<GParameterSet>();

			// Make sure the individual is clean by manually setting the dirty flag to false
			// We might not have any evaluation code, so we cannot just call the fitness() function
			// p_test->setDirtyFlag(false);

			// Multiply each floating point value with a constrained random value
			BOOST_CHECK_NO_THROW(p_test->fpMultiplyByRandom());

			// Make sure the dirty flag is set
			BOOST_CHECK(p_test->isDirty() == true);

			// Cross-check
			std::size_t counter = 0;
			for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() != p_test_0->at<GConstrainedDoubleObject>(counter)->value());
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() != p_test_0->at<GDoubleObject>(counter)->value());
				counter++;
				boost::shared_ptr<GDoubleCollection> p_gdc;
				boost::shared_ptr<GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc   = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) != p_gdc_0->at(gdc_cnt)
							,  "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
							<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
			boost::shared_ptr<GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			boost::shared_ptr<GBooleanObject> p_boolean_orig;
			boost::shared_ptr<GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Check adding of individuals
			// Create two GParameterSet objects as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test       = p_test_0->clone<GParameterSet>();
			boost::shared_ptr<GParameterSet> p_test_fixed = p_test_0->clone<GParameterSet>();

			// Initialize all fp-values of the "add" individual with a fixed valie
			BOOST_CHECK_NO_THROW(p_test_fixed->fpFixedValueInit(FPADD));

			// Add p_test_fixed to p_test
			BOOST_CHECK_NO_THROW(p_test->fpAdd(p_test_fixed));

			// Check the results
			std::size_t counter = 0;
			for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
				// A constrained value does not have to assume the value value()+FPADD
				// but needs to stay within its boundaries
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == p_test_0->at<GDoubleObject>(counter)->value() + FPADD);
				counter++;
				boost::shared_ptr<GDoubleCollection> p_gdc;
				boost::shared_ptr<GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc   = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) + FPADD
							,  "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
							<< "FPADD = " << FPADD
							<< "p_gdc_0->at(gdc_cnt) + FPADD = " << p_gdc_0->at(gdc_cnt) + FPADD << "\n"
							<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
			boost::shared_ptr<GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			boost::shared_ptr<GBooleanObject> p_boolean_orig;
			boost::shared_ptr<GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------

		{ // Check subtraction of individuals
			// Create two GParameterSet objects as a clone of p_test_0 for further usage
			boost::shared_ptr<GParameterSet> p_test       = p_test_0->clone<GParameterSet>();
			boost::shared_ptr<GParameterSet> p_test_fixed = p_test_0->clone<GParameterSet>();

			// Initialize all fp-values of the "add" individual with a fixed valie
			BOOST_CHECK_NO_THROW(p_test_fixed->fpFixedValueInit(FPSUBTRACT));

			// Add p_test_fixed to p_test
			BOOST_CHECK_NO_THROW(p_test->fpSubtract(p_test_fixed));

			// Check the results
			std::size_t counter = 0;
			for(std::size_t i=0; i<FPLOOPCOUNT; i++) {
				// A constrained value does not have to assume the value value()-FPSUBTRACT
				// but needs to stay within its boundaries
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() >= MINGCONSTRDOUBLE);
				BOOST_CHECK(p_test->at<GConstrainedDoubleObject>(counter)->value() <= MAXGCONSTRDOUBLE);
				counter++;
				BOOST_CHECK(p_test->at<GDoubleObject>(counter)->value() == p_test_0->at<GDoubleObject>(counter)->value() - FPSUBTRACT);
				counter++;
				boost::shared_ptr<GDoubleCollection> p_gdc;
				boost::shared_ptr<GDoubleCollection> p_gdc_0;
				BOOST_CHECK_NO_THROW(p_gdc   = p_test->at<GDoubleCollection>(counter));
				BOOST_CHECK_NO_THROW(p_gdc_0 = p_test_0->at<GDoubleCollection>(counter));
				for(std::size_t gdc_cnt=0; gdc_cnt<NGDOUBLECOLL; gdc_cnt++) {
					BOOST_CHECK_MESSAGE (
							p_gdc->at(gdc_cnt) == p_gdc_0->at(gdc_cnt) - FPSUBTRACT
							,  "\n"
							<< "p_gdc->at(gdc_cnt) = " << p_gdc->at(gdc_cnt) << "\n"
							<< "p_gdc_0->at(gdc_cnt) = " << p_gdc_0->at(gdc_cnt) << "\n"
							<< "FPSUBTRACT = " << FPSUBTRACT
							<< "p_gdc_0->at(gdc_cnt) - FPSUBTRACT = " << p_gdc_0->at(gdc_cnt) - FPSUBTRACT << "\n"
							<< "iteration = " << gdc_cnt << "\n"
					);
				}
				counter++;
			}

			// The int32 parameter should have stayed the same
			boost::shared_ptr<GConstrainedInt32Object> p_int32_0;
			boost::shared_ptr<GConstrainedInt32Object> p_int32;
			BOOST_CHECK_NO_THROW(p_int32_0   = p_test_0->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK_NO_THROW(p_int32 =   p_test->at<GConstrainedInt32Object>(counter));
			BOOST_CHECK(*p_int32_0 == *p_int32);
			counter++;

			// Likewise, the boolean parameter should have stayed the same
			boost::shared_ptr<GBooleanObject> p_boolean_orig;
			boost::shared_ptr<GBooleanObject> p_boolean_cloned;
			BOOST_CHECK_NO_THROW(p_boolean_orig   = p_test_0->at<GBooleanObject>(counter));
			BOOST_CHECK_NO_THROW(p_boolean_cloned =   p_test->at<GBooleanObject>(counter));
			BOOST_CHECK(*p_boolean_orig == *p_boolean_cloned);
			counter++;
		}

		//-----------------------------------------------------------------
	}

	//---------------------------------------------------------------------
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GParameterSet::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GMutableSetT<Gem::Geneva::GParameterBase>::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/**
 * As the Gem::Geneva::GParameterSet has a private default constructor, we need to provide a
 * specialization of the factory function that creates objects of this type.
 */
template <>
boost::shared_ptr<Gem::Geneva::GParameterSet> TFactory_GUnitTests<Gem::Geneva::GParameterSet>() {
	return boost::shared_ptr<Gem::Geneva::GParameterSet>(new Gem::Geneva::GParameterSet());
}

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */
