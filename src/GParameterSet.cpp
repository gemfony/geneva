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

#include "GParameterSet.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterSet)

namespace Gem
{
	namespace GenEvA
	{
	/**********************************************************************************/
	/**
	 * The default constructor.
	 */
	GParameterSet::GParameterSet()
	: GMutableSetT<Gem::GenEvA::GParameterBase>()
	  { /* nothing */ }

	/**********************************************************************************/
	/**
	 * The copy constructor. Note that we cannot rely on the operator=() of the vector
	 * here, as we do not know the actual type of T objects.
	 *
	 * @param cp A copy of another GParameterSet object
	 */
	GParameterSet::GParameterSet(const GParameterSet& cp)
	: GMutableSetT<Gem::GenEvA::GParameterBase>(cp)
	  , eval_(cp.eval_)
	  { /* nothing */ }

	/**********************************************************************************/
	/**
	 * The destructor
	 */
	GParameterSet::~GParameterSet()
	{ /* nothing */ }

	/**********************************************************************************/
	/**
	 * A Standard assignment operator
	 *
	 * @param cp A copy of another GParameterSet object
	 * @return A constant reference to this object
	 */
	const GParameterSet& GParameterSet::operator=(const GParameterSet& cp){
		GParameterSet::load(&cp);
		return *this;
	}

	/**********************************************************************************/
	/**
	 * Checks for equality with another GParameterSet object
	 *
	 * @param  cp A constant reference to another GParameterSet object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool GParameterSet::operator==(const GParameterSet& cp) const {
		return GParameterSet::isEqualTo(cp, boost::logic::indeterminate);
	}

	/**********************************************************************************/
	/**
	 * Checks for inequality with another GParameterSet object
	 *
	 * @param  cp A constant reference to another GParameterSet object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool GParameterSet::operator!=(const GParameterSet& cp) const {
		return !GParameterSet::isEqualTo(cp, boost::logic::indeterminate);
	}

	/**********************************************************************************/
	/**
	 * Checks for equality with another GParameterSet object. As we have no
	 * local data, we just check for equality of the parent class-
	 *
	 * @param  cp A constant reference to another GParameterSet object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool GParameterSet::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GIndividual reference
		const GParameterSet *p_load = GObject::conversion_cast(&cp,  this);

		// Check our parent class
		if(!GMutableSetT<Gem::GenEvA::GParameterBase>::isEqualTo(*p_load, expected)) return  false;
		return true;
	}

	/**********************************************************************************/
	/**
	 * Checks for similarity with another GParameterSet object. As we have
	 * no local data, we just check for similarity of the parent class.
	 *
	 * @param  cp A constant reference to another GParameterSet object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	bool GParameterSet::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GIndividual reference
		const GParameterSet *p_load = GObject::conversion_cast(&cp,  this);

		// Check our parent class
		if(!GMutableSetT<Gem::GenEvA::GParameterBase>::isSimilarTo(*p_load, limit, expected)) return  false;
		return true;
	}

	/**********************************************************************************/
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
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Check that we are indeed dealing with a GParamterBase reference
		const GParameterSet *p_load = GObject::conversion_cast(&cp,  this);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GMutableSetT<Gem::GenEvA::GParameterBase>::checkRelationshipWith(cp, e, limit, "GParameterSet", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GParameterSet", caller, deviations, e);
	}

	/**********************************************************************************/
	/**
	 * Creates a deep clone of this object.
	 *
	 * @return A deep clone of this object
	 */
	GObject * GParameterSet::clone_() const {
		return new GParameterSet(*this);
	}

	/**********************************************************************************/
	/**
	 * Loads the data of another GParameterSet object, camouflaged as a GObject.
	 *
	 * @param cp A copy of another GParameterSet object, camouflaged as a GObject
	 */
	void GParameterSet::load(const GObject* cp){
		// Convert to local format
		const GParameterSet *p_load = this->conversion_cast(cp, this);

		// Load the parent class'es data
		GMutableSetT<Gem::GenEvA::GParameterBase>::load(cp);

		// Then load our local data - here the evaluation function (if any)
		eval_ = p_load->eval_;
	}

	/**********************************************************************************/
	/**
	 * Registers an evaluation function with this class. Note that the function object
	 * can not be serialized. Hence, in a networked optimization run, you need to derive
	 * your own class from GParameterSet and specifiy an evaluation function.
	 */
	void GParameterSet::registerEvaluator(const boost::function<double (const GParameterSet&)>& eval){
		if(eval.empty()){ // empty function ?
			std::ostringstream error;
			error << "In GParameterSet::registerEvaluator(): Error" << std::endl
					<< "Received empty function" << std::endl;

			throw geneva_error_condition(error.str());
		}

		eval_ = eval;
	}

	/**********************************************************************************/
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

			throw geneva_error_condition(error.str());
		}

		// Trigger the actual calculation
		return eval_(*this);
	}
	/**********************************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
