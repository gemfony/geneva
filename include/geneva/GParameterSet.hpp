/**
 * @file GParameterSet.hpp
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


// Standard header files go here
#include <sstream>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost header files go here

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>

#ifndef GPARAMETERSET_HPP_
#define GPARAMETERSET_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GMutableSetT.hpp"
#include "GObject.hpp"
#include "GParameterBase.hpp"

#ifdef GENEVATESTING
#include "common/GUnitTestFrameworkT.hpp"
#include "GBooleanObject.hpp"
#include "GConstrainedInt32Object.hpp"
#include "GConstrainedDoubleObject.hpp"
#include "GConstrainedDoubleObjectCollection.hpp"
#include "GDoubleCollection.hpp"
#include "GDoubleObject.hpp"
#include "GInt32Collection.hpp"
#include "hap/GRandomT.hpp"
#endif /* GENEVATESTING */


namespace Gem {
namespace Tests {
class GTestIndividual1; // forward declaration, needed for test purposes
} /* namespace Tests */
} /* namespace Gem */


namespace Gem {
namespace Geneva {

/**************************************************************************/
/**
 * This class implements a collection of GParameterBase objects. It
 * will form the basis of many user-defined individuals.
 */
class GParameterSet:
	public GMutableSetT<Gem::Geneva::GParameterBase>
{
	friend class Gem::Tests::GTestIndividual1; ///< Needed for testing purposes

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;
	  ar & make_nvp("GMutableSetT_GParameterBase",
			  boost::serialization::base_object<GMutableSetT<Gem::Geneva::GParameterBase> >(*this));
	}
	///////////////////////////////////////////////////////////////////////
public:
	/** @brief The default constructor */
	GParameterSet();
	/** @brief The copy constructor */
	GParameterSet(const GParameterSet&);
	/** @brief The destructor */
	virtual ~GParameterSet();

	/** @brief Standard assignment operator */
	const GParameterSet& operator=(const GParameterSet&);

	/** @brief Checks for equality with another GParameterSet object */
	bool operator==(const GParameterSet&) const;
	/** @brief Checks for inequality with another GParameterSet object */
	bool operator!=(const GParameterSet&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Registers an evaluation function */
	void registerEvaluator(const boost::function<double (const GParameterSet&)>&);

	/** @brief Triggers updates when the optimization process has stalled */
	virtual bool updateOnStall();

	/** @brief Allows to randomly initialize parameter members */
	virtual void randomInit();

	/** @brief Multiplies with a random floating point number in a given range */
	void fpMultiplyByRandom(const float&, const float&);
	/** @brief Multiplies with a random floating point number in the range [0, 1[ */
	void fpMultiplyByRandom();

	/** @brief Multiplies floating-point parameters with a given value */
	void fpMultiplyBy(const float& val);

	/** @brief Initializes floating-point parameters with a given value */
	void fpFixedValueInit(const float&);

	/** @brief Adds the floating point parameters of another GParameterSet object to this one */
	void fpAdd(boost::shared_ptr<GParameterSet>);
	/** @brief Subtracts the floating point parameters of another GParameterSet object from this one */
	void fpSubtract(boost::shared_ptr<GParameterSet>);

	/** @brief Updates the random number generators contained in this object's GParameterBase-derivatives */
	virtual void updateRNGs();
	/** @brief Restores the local random number generators contained in this object's GParameterBase-derivatives */
	virtual void restoreRNGs();
	/** @brief Checks whether all GParameterBase derivatives use local random number generators */
	virtual bool localRNGsUsed() const;
	/** @brief Checks whether all GParameterBase derivatives use the assigned random number generator */
	virtual bool assignedRNGUsed() const;

	/** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
	boost::shared_ptr<Gem::Geneva::GParameterBase> at(const std::size_t& pos);

	/**********************************************************************/
	/**
	 * This function returns a parameter set at a given position of the data set.
	 * Note that this function will only be accessible to the compiler if parameter_type
	 * is a derivative of GParameterBase, thanks to the magic of Boost's enable_if and
	 * Type Traits libraries.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GParameterBase object, as required by the user
	 */
	template <typename parameter_type>
	inline const boost::shared_ptr<parameter_type> at(
			  const std::size_t& pos
			, typename boost::enable_if<boost::is_base_of<GParameterBase, parameter_type> >::type* dummy = 0
	)  const {
#ifdef DEBUG
		boost::shared_ptr<parameter_type> p = boost::static_pointer_cast<parameter_type>(data.at(pos));

		if(p) return p;
		else {
			std::ostringstream error;
			error << "In GParameterSet::at<>() : Conversion error" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#else
		return boost::static_pointer_cast<parameter_type>(data[pos]);
#endif /* DEBUG */
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested. See also the second version of the at() function.
	 * ----------------------------------------------------------------------------------
	 */

	/**************************************************************************************************/

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation();
	/* @brief The actual adaption operations. */
	virtual void customAdaptions();

	/** @brief Allows to store an evaluation function for this object */
	boost::function<double (const GParameterSet&)> eval_;

private:
	explicit GParameterSet(const float&); ///< Intentionally private and undefined

#ifdef GENEVATESTING
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/** @brief We need to provide a specialization of the factory function that creates objects of this type. */
template <> boost::shared_ptr<Gem::Geneva::GParameterSet> TFactory_GUnitTests<Gem::Geneva::GParameterSet>();

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */

#endif /* GPARAMETERSET_HPP_ */
