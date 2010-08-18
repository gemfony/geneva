/**
 * @file GParameterSet.cpp
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

	GIndividual::updateOnStall();

	return false;
}


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

/************************************************************************************************************/
/**
 * Triggers multiplication of floating point parameters with a random floating point number in a given range
 *
 * @param min The lower boundary for random number generation
 * @param max The upper boundary for random number generation
 */
void GParameterSet::fpRandomMultiplyBy(const float& min, const float& max) {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpRandomMultiplyBy(min, max);
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

/************************************************************************************************************/
/**
 * Triggers multiplication of floating point parameters with a random floating point number in the range [0,1[
 */
void GParameterSet::fpRandomMultiplyBy() {
	// Loop over all GParameterBase objects
	GParameterSet::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->fpRandomMultiplyBy();
	}

	// As we have modified our internal data sets, make sure the dirty flag is set
	GIndividual::setDirtyFlag();
}

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
}

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
}

/************************************************************************************************************/
/**
 * Small convenience function that helps to add a GParameterSet objects to this one in
 * more transparent ways.
 */
void GParameterSet::operator+=(boost::shared_ptr<GParameterSet> p) {
	fpAdd(p);
}

/************************************************************************************************************/
/**
 * Small convenience function that helps to subtract a GParameterSet objects from this one in
 * more transparent ways.
 */
void GParameterSet::operator-=(boost::shared_ptr<GParameterSet> p) {
	fpSubtract(p);
}

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
	// Instantiate a local random number generator
	using namespace Gem::Hap;
	GRandomT<RANDOMLOCAL> gr;

	// Call the parent class'es function
	GMutableSetT<Gem::Geneva::GParameterBase>::specificTestsNoFailureExpected_GUnitTests();

	//---------------------------------------------------------------------
	// Create a GParameterSet object as a clone of this object for further usage
	boost::shared_ptr<GParameterSet> p_test = this->clone<GParameterSet>();
	// Clear the collection
	p_test->clear();
	// Make sure it is really empty
	BOOST_CHECK(p_test->empty());
	// Add some parameters
	for(std::size_t i=0; i<5; i++) {
		p_test->push_back(boost::shared_ptr<GConstrainedDouble>(new GConstrainedDouble(gr.uniform_real(-3.,3.), -3., 3.)));
		p_test->push_back(boost::shared_ptr<GDouble>(new GDouble(gr.uniform_real(-3.,3.))));
	}

	//---------------------------------------------------------------------
	// Test whether fpMultipyBy has an effect
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
