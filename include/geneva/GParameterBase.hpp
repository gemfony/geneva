/**
 * @file GParameterBase.hpp
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

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/optional.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#ifndef GPARAMETERBASE_HPP_
#define GPARAMETERBASE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "GMutableI.hpp"
#include "GObject.hpp"
#include "GObjectExpectationChecksT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************************/
/**
 * The purpose of this class is to provide a common base for all parameter classes, so
 * that a GParameterSet can be built from different parameter types. The class also
 * defines the interface that needs to be implemented by parameter classes.
 *
 * Note: It is required that derived classes make sure that a useful operator=() is available!
 */
class GParameterBase
	: public GMutableI
	, public GObject
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
         & BOOST_SERIALIZATION_NVP(adaptionsActive_)
         & BOOST_SERIALIZATION_NVP(randomInitializationBlocked_);
    }
    ///////////////////////////////////////////////////////////////////////
public:
	/** @brief The standard constructor */
	GParameterBase();
	/** @brief The copy constructor */
	GParameterBase(const GParameterBase&);
	/** @brief The standard destructor */
	virtual ~GParameterBase();

	/** @brief The adaption interface */
	void adapt();
	/** @brief The actual adaption logic */
	virtual void adaptImpl() = 0;

	/** @brief Switches on adaptions for this object */
	bool setAdaptionsActive();
	/** @brief Disables adaptions for this object */
	bool setAdaptionsInactive();
	/** @brief Determines whether adaptions are performed for this object */
	bool adaptionsActive() const;

	/** @brief Checks for equality with another GParameter Base object */
	bool operator==(const GParameterBase&) const;
	/** @brief Checks for inequality with another GParameterBase object */
	bool operator!=(const GParameterBase&) const;

	/** @brief Triggers random initialization of the parameter(-collection) */
	virtual void randomInit();

	/** @brief Initializes double-based parameters with a given value */
	virtual void fpFixedValueInit(const float& val);
	/** @brief Multiplies double-based parameters with a given value */
	virtual void fpMultiplyBy(const float& val);
	/** @brief Multiplies with a random floating point number in a given range */
	virtual void fpMultiplyByRandom(const float&, const float&);
	/** @brief Multiplies with a random floating point number in the range [0, 1[ */
	virtual void fpMultiplyByRandom();
	/** @brief Adds the floating point parameters of another GParameterBase object to this one */
	virtual void fpAdd(boost::shared_ptr<GParameterBase>);
	/** @brief Subtract the floating point parameters of another GParameterBase object from this one */
	virtual void fpSubtract(boost::shared_ptr<GParameterBase>);

	/** @brief Attach parameters of type double to the vector */
	virtual void doubleStreamline(std::vector<double>& parVec) const;
	/** @brief Attach parameters of type boost::int32_t to the vector */
	virtual void int32Streamline(std::vector<boost::int32_t>& parVec) const;
	/** @brief Attach parameters of type bool to the vector */
	virtual void booleanStreamline(std::vector<bool>& parVec) const;

	/** @brief Count the number of double parameters */
	virtual std::size_t countDoubleParameters() const;
	/** @brief Count the number of boost::int32_t parameters */
	virtual std::size_t countInt32Parameters() const;
	/** @brief Count the number of bool parameters */
	virtual std::size_t countBoolParameters() const;

	/** @brief Assigns part of a value vector to the parameter */
	virtual void assignDoubleValueVector(const std::vector<double>&, std::size_t&);
	/** @brief Assigns part of a value vector to the parameter */
	virtual void assignInt32ValueVector(const std::vector<boost::int32_t>&, std::size_t&);
	/** @brief Assigns part of a value vector to the parameter */
	virtual void assignBooleanValueVector(const std::vector<bool>&, std::size_t&);

	/** @brief Specifies that no random initialization should occur anymore */
	void blockRandomInitialization();
	/** @brief Makes random initialization possible */
	void allowRandomInitialization();
	/** @brief Checks whether initialization has been blocked */
	bool randomInitializationBlocked() const;

	/** @brief Convenience function so we do not need to always cast derived classes */
	virtual bool hasAdaptor() const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Assigns a random number generator from another object. */
	virtual void assignGRandomPointer(Gem::Hap::GRandomBaseT<double, boost::int32_t> *);
	/** @brief Re-connects the local random number generator to gr */
	virtual void resetGRandomPointer();
	/** @brief Checks whether the local random number generator is used */
	virtual bool usesLocalRNG() const;
	/** @brief Checks whether the assigned random number generator is used throughout */
	virtual bool assignedRNGUsed() const;

	/**************************************************************************************************/
	/**
	 * This function converts a GParameterBase boost::shared_ptr to the target type.  Note that this
	 * template will only be accessible to the compiler if GParameterBase is a base type of load_type.
	 *
	 * @param load_ptr A boost::shared_ptr<load_type> to the item to be converted
	 * @param dummy A dummy argument needed for boost's enable_if and type traits magic
	 * @return A boost::shared_ptr holding the converted object
	 */
	template <typename load_type>
	boost::shared_ptr<load_type> parameterbase_cast (
			boost::shared_ptr<GParameterBase> load_ptr
		  , typename boost::enable_if<boost::is_base_of<Gem::Geneva::GParameterBase, load_type> >::type* dummy = 0
	) const {
#ifdef DEBUG
		boost::shared_ptr<load_type> p = boost::dynamic_pointer_cast<load_type>(load_ptr);
		if(p) return p;
		else {
			std::ostringstream error;
			error << "In boost::shared_ptr<load_type> GParameterBase::conversion_cast<load_type>() : Error!" << std::endl
				  << "Invalid conversion" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}
#else
		return boost::static_pointer_cast<load_type>(load_ptr);
#endif
	}

	/**************************************************************************************************/

protected:
	/**************************************************************************************************/
	/**
     * A random number generator. This reference and the associated pointer is either
     * connected to a local random number generator assigned in the constructor, or
     * to a "factory" generator located in the surrounding GParameterSet object.
     */
	Gem::Hap::GRandomBaseT<double, boost::int32_t> *gr_local;
	Gem::Hap::GRandomBaseT<double, boost::int32_t> *gr;

	/**************************************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

	/** @brief Triggers random initialization of the parameter(-collection) */
	virtual void randomInit_() = 0;

private:
	bool adaptionsActive_; ///< Specifies whether adaptions of this object should be carried out
	bool randomInitializationBlocked_; ///< Specifies that this object should not be initialized again

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

/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterBase)

/**************************************************************************************************/

#endif /* GPARAMETERBASE_HPP_ */
