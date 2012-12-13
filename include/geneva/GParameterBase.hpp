/**
 * @file GParameterBase.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


// Standard header files go here

// Boost header files go here

#ifndef GPARAMETERBASE_HPP_
#define GPARAMETERBASE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GMutableI.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GObjectExpectationChecksT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The purpose of this class is to provide a common base for all parameter classes, so
 * that a GParameterSet can be built from different parameter types. The class also
 * defines the interface that needs to be implemented by parameter classes.
 *
 * Note: It is required that derived classes make sure that a useful operator=() is available!
 */
class GParameterBase
	: public GObject
	, public GMutableI
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

	/** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
	virtual bool isIndividualParameter() const;
	/** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
	bool isParameterCollection() const;

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

	/***************************************************************************/
	/**
	 * Allows to add all parameters of a specific type to the vector. This function is a
	 * trap, needed to catch streamlining attempts with unsupported types. Use the supplied
	 * specializations instead.
	 *
	 * @oaram parVec The vector to which the items should be added
	 */
	template <typename par_type>
	void streamline(std::vector<par_type>& parVec) const
	{
	   glogger
	   << "In GParameterBase::streamline(std::vector<>&)" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
	}

	/***************************************************************************/
	/** @brief Attach parameters of type float to the vector */
	virtual void floatStreamline(std::vector<float>&) const;
	/** @brief Attach parameters of type double to the vector */
	virtual void doubleStreamline(std::vector<double>&) const;
	/** @brief Attach parameters of type boost::int32_t to the vector */
	virtual void int32Streamline(std::vector<boost::int32_t>&) const;
	/** @brief Attach parameters of type bool to the vector */
	virtual void booleanStreamline(std::vector<bool>&) const;

	/***************************************************************************/
	/**
	 * Allows to add all boundaries if parameters of a specific type to the vectors. This
	 * function is a trap, needed to catch streamlining attempts with unsupported types.
	 * Use the supplied specializations instead.
	 *
	 * @oaram lBndVec The vector with lower boundaries of parameters
	 * @oaram uBndVec The vector with upper boundaries of parameters
	 */
	template <typename par_type>
	void boundaries(
			std::vector<par_type>& lBndVec
			, std::vector<par_type>& uBndVec
	) const	{
	   glogger
	   << "In GParameterBase::boundaries(std::vector<>&)" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
	}

	/***************************************************************************/
	/** @brief Attach boundaries of type float to the vectors */
	virtual void floatBoundaries(std::vector<float>&, std::vector<float>&) const;
	/** @brief Attach boundaries of type double to the vectors */
	virtual void doubleBoundaries(std::vector<double>&, std::vector<double>&) const;
	/** @brief Attach boundaries of type boost::int32_t to the vectors */
	virtual void int32Boundaries(std::vector<boost::int32_t>&, std::vector<boost::int32_t>&) const;
	/** @brief Attach boundaries of type bool to the vectors */
	virtual void booleanBoundaries(std::vector<bool>&, std::vector<bool>&) const;

	/***************************************************************************/
	/**
	 * Allows to count parameters of a specific type. This function is a trap, needed to
	 * catch attempts to use this function with unsupported types. Use the supplied
	 * specializations instead.
	 *
	 * @return The number of parameters of a given Type
	 */
	template <typename par_type>
	std::size_t countParameters() const
	{
	   glogger
	   << "In GParameterBase::countParameters()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;

	   // Make the compiler happy
	   return (std::size_t)0;
	}

	/***************************************************************************/

	/** @brief Count the number of float parameters */
	virtual std::size_t countFloatParameters() const;
	/** @brief Count the number of double parameters */
	virtual std::size_t countDoubleParameters() const;
	/** @brief Count the number of boost::int32_t parameters */
	virtual std::size_t countInt32Parameters() const;
	/** @brief Count the number of bool parameters */
	virtual std::size_t countBoolParameters() const;

	/***************************************************************************/
	/**
	 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
	 * This function is a trap, needed to catch attempts to use this function with unsupported
	 * types. Use the supplied specializations instead.
	 *
	 * @param parVec The vector with the parameters to be assigned to the object
	 * @param pos The position from which parameters will be taken (will be updated by the call)
	 */
	template <typename par_type>
	void assignValueVector(const std::vector<par_type>& parVec, std::size_t& pos)
	{
	   glogger
	   << "In GParameterBase::assignValueVector()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
	}

	/***************************************************************************/

	/** @brief Assigns part of a value vector to the parameter */
	virtual void assignFloatValueVector(const std::vector<float>&, std::size_t&);
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
	virtual void assignGRandomPointer(Gem::Hap::GRandomBase *);
	/** @brief Re-connects the local random number generator to gr */
	virtual void resetGRandomPointer();
	/** @brief Checks whether the local random number generator is used */
	virtual bool usesLocalRNG() const;
	/** @brief Checks whether the assigned random number generator is used throughout */
	virtual bool assignedRNGUsed() const;

	/***************************************************************************/
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
		   glogger
		   << "In boost::shared_ptr<load_type> GParameterBase::gobject_conversion<load_type>() :" << std::endl
         << "Invalid conversion" << std::endl
         << GEXCEPTION;

		   // Make the compiler happy
		   return boost::shared_ptr<load_type>();
		}
#else
		return boost::static_pointer_cast<load_type>(load_ptr);
#endif
	}


protected:
	/***************************************************************************/
	/**
     * A random number generator. This reference and the associated pointer is either
     * connected to a local random number generator assigned in the constructor, or
     * to a "factory" generator located in the surrounding GParameterSet object.
     */
	Gem::Hap::GRandomBase *gr_local;
	Gem::Hap::GRandomBase *gr;

	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

	/** @brief Triggers random initialization of the parameter(-collection) */
	virtual void randomInit_() = 0;

private:
	bool adaptionsActive_; ///< Specifies whether adaptions of this object should be carried out
	bool randomInitializationBlocked_; ///< Specifies that this object should not be initialized again

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
};

/******************************************************************************/
/**
 * Specializations of some template functions
 */
template <>	void GParameterBase::streamline<float>(std::vector<float>&) const;
template <>	void GParameterBase::streamline<double>(std::vector<double>&) const;
template <>	void GParameterBase::streamline<boost::int32_t>(std::vector<boost::int32_t>&) const;
template <>	void GParameterBase::streamline<bool>(std::vector<bool>&) const;

template <>	void GParameterBase::boundaries<float>(std::vector<float>&, std::vector<float>&) const;
template <>	void GParameterBase::boundaries<double>(std::vector<double>&, std::vector<double>&) const;
template <>	void GParameterBase::boundaries<boost::int32_t>(std::vector<boost::int32_t>&, std::vector<boost::int32_t>&) const;
template <>	void GParameterBase::boundaries<bool>(std::vector<bool>&, std::vector<bool>&) const;

template <>	std::size_t GParameterBase::countParameters<float>() const;
template <>	std::size_t GParameterBase::countParameters<double>() const;
template <>	std::size_t GParameterBase::countParameters<boost::int32_t>() const;
template <>	std::size_t GParameterBase::countParameters<bool>() const;

template <>	void GParameterBase::assignValueVector<float>(const std::vector<float>&, std::size_t&);
template <>	void GParameterBase::assignValueVector<double>(const std::vector<double>&, std::size_t&);
template <>	void GParameterBase::assignValueVector<boost::int32_t>(const std::vector<boost::int32_t>&, std::size_t&);
template <>	void GParameterBase::assignValueVector<bool>(const std::vector<bool>&, std::size_t&);

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterBase)

/******************************************************************************/

#endif /* GPARAMETERBASE_HPP_ */
