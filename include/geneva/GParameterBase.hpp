/**
 * @file GParameterBase.hpp
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


// Standard header files go here

// Boost header files go here
#include <boost/lexical_cast.hpp>

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
         & BOOST_SERIALIZATION_NVP(randomInitializationBlocked_)
         & BOOST_SERIALIZATION_NVP(parameterName_);
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
	virtual std::size_t adapt() OVERRIDE;
	/** @brief The actual adaption logic */
	virtual std::size_t adaptImpl() BASE = 0;

   /** @brief Triggers updates when the optimization process has stalled */
   virtual bool updateAdaptorsOnStall(const std::size_t&) BASE = 0;
   /** @brief Retrieves information from an adaptor on a given property */
   virtual void queryAdaptor(
      const std::string& adaptorName
      , const std::string& property
      , std::vector<boost::any>& data
   ) const BASE = 0;

	/** @brief Switches on adaptions for this object */
	bool setAdaptionsActive();
	/** @brief Disables adaptions for this object */
	bool setAdaptionsInactive();
	/** @brief Determines whether adaptions are performed for this object */
	bool adaptionsActive() const;
   /** @brief Determines whether adaptions are inactive for this object */
   bool adaptionsInactive() const;

	/** @brief Checks for equality with another GParameter Base object */
	bool operator==(const GParameterBase&) const;
	/** @brief Checks for inequality with another GParameterBase object */
	bool operator!=(const GParameterBase&) const;

	/** @brief Triggers random initialization of the parameter(-collection) */
	virtual void randomInit(const activityMode&) BASE;

	/** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
	virtual bool isIndividualParameter() const BASE;
	/** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
	virtual bool isParameterCollection() const BASE;

	/** @brief Initializes double-based parameters with a given value */
	virtual void fpFixedValueInit(const float& val) BASE;
	/** @brief Multiplies double-based parameters with a given value */
	virtual void fpMultiplyBy(const float& val) BASE;
	/** @brief Multiplies with a random floating point number in a given range */
	virtual void fpMultiplyByRandom(const float&, const float&) BASE;
	/** @brief Multiplies with a random floating point number in the range [0, 1[ */
	virtual void fpMultiplyByRandom() BASE;
	/** @brief Adds the floating point parameters of another GParameterBase object to this one */
	virtual void fpAdd(boost::shared_ptr<GParameterBase>) BASE;
	/** @brief Subtract the floating point parameters of another GParameterBase object from this one */
	virtual void fpSubtract(boost::shared_ptr<GParameterBase>) BASE;

	/** @brief Emits a name for this class / object */
	virtual std::string name() const OVERRIDE;

	/** @brief Allows to assign a name to this parameter */
	void setParameterName(const std::string&);
	/** @brief Allows to retrieve the name of this parameter */
	std::string getParameterName() const;

	/** @brief Checks whether this object matches a given activity mode */
	bool amMatch(const activityMode&) const;
	/** @brief Returns true on the case of an activity mode mismatch */
	bool amMismatch(const activityMode&) const;

	/** @brief Checks whether this object matches a given activity mode and is modifiable */
	bool modifiableAmMatchOrHandover(const activityMode&) const;

   /***************************************************************************/
   /**
    * Allows to count parameters of a specific type. This function is a trap, needed to
    * catch attempts to use this function with unsupported types. Use the supplied
    * specializations instead.
    *
    * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
    * @return The number of parameters of a given Type
    */
   template <typename par_type>
   std::size_t countParameters(
      const activityMode& am
   ) const {
      glogger
      << "In GParameterBase::countParameters()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;

      // Make the compiler happy
      return (std::size_t)0;
   }

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
      , const activityMode& am
   ) const  {
      glogger
      << "In GParameterBase::boundaries(std::vector<>&)" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

	/***************************************************************************/
	/**
	 * Allows to add all parameters of a specific type to the vector. This function is a
	 * trap, needed to catch streamlining attempts with unsupported types. Use the supplied
	 * specializations instead.
	 *
	 * @oaram parVec The vector to which the items should be added
	 */
	template <typename par_type>
	void streamline(
      std::vector<par_type>& parVec
      , const activityMode& am
   ) const {
	   glogger
	   << "In GParameterBase::streamline(std::vector<par_type>&)" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
	}

   /***************************************************************************/
   /**
    * Allows to add all parameters of a specific type to the map. This function is a
    * trap, needed to catch streamlining attempts with unsupported types. Use the supplied
    * specializations instead.
    *
    * @oaram parVec The vector to which the items should be added
    */
   template <typename par_type>
   void streamline(
      std::map<std::string, std::vector<par_type> >& parVec
      , const activityMode& am
   ) const {
      glogger
      << "In GParameterBase::streamline(std::map<std::string, std::vec<par_type> >)" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

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
	void assignValueVector(
      const std::vector<par_type>& parVec
      , std::size_t& pos
      , const activityMode& am
   ) {
	   glogger
	   << "In GParameterBase::assignValueVector()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
	}

   /***************************************************************************/
   /**
    * Assigns values from a std::map<std::string, std::vector<par_type> > to the parameter
    *
    * @param parMao The map with the parameters to be assigned to the object
    */
   template <typename par_type>
   void assignValueVectors(
      const std::map<std::string, std::vector<par_type> >& parMap
      , const activityMode& am
   ) {
      glogger
      << "In GParameterBase::assignValueVectors()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

   /***************************************************************************/
   /**
    * Multiplication with a random value in a given range
    */
   template <typename par_type>
   void multiplyByRandom(
      const par_type& min
      , const par_type& max
      , const activityMode& am
   ) {
      glogger
      << "In GParameterBase::multiplyByRandom()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

   /***************************************************************************/
   /**
    * Multiplication with a random value in the range [0, 1[
    */
   template <typename par_type>
   void multiplyByRandom(
      const activityMode& am
   ) {
      glogger
      << "In GParameterBase::multiplyByRandom()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

   /***************************************************************************/
   /**
    * Multiplication with a constant value
    */
   template <typename par_type>
   void multiplyBy(
      const par_type& val
      , const activityMode& am
   ) {
      glogger
      << "In GParameterBase::multiplyBy()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

   /***************************************************************************/
   /**
    * Initializes all parameters of a given type with a constant value
    */
   template <typename par_type>
   void fixedValueInit(
      const par_type& val
      , const activityMode& am
   ) {
      glogger
      << "In GParameterBase::fixedValueInit()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

   /***************************************************************************/
   /**
    * Adds the parameters of another GParameterSet object to this one
    */
   template <typename par_type>
   void add(
      boost::shared_ptr<GParameterBase> p
      , const activityMode& am
   ) {
      glogger
      << "In GParameterBase::add()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

   /***************************************************************************/
   /**
    * Subtracts the parameters of another GParameterSet object from this one
    */
   template <typename par_type>
   void subtract(
      boost::shared_ptr<GParameterBase> p
      , const activityMode& am
   ) {
      glogger
      << "In GParameterBase::subtract()" << std::endl
      << "Function called for unsupported type!" << std::endl
      << GEXCEPTION;
   }

   /***************************************************************************/

	/** @brief Specifies that no random initialization should occur anymore */
	void blockRandomInitialization();
	/** @brief Makes random initialization possible */
	void allowRandomInitialization();
	/** @brief Checks whether initialization has been blocked */
	bool randomInitializationBlocked() const;

	/** @brief Convenience function so we do not need to always cast derived classes */
	virtual bool hasAdaptor() const BASE;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

   /** @brief Converts the local data to a boost::property_tree node */
   virtual void toPropertyTree(pt::ptree&, const std::string&) const = 0;
   /** @brief Returns a human-readable name for the base type of derived objects */
   virtual std::string baseType() const BASE;
   /** @brief Lets the audience know whether this is a leaf or a branch object */
   virtual bool isLeaf() const BASE;

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
   /** @brief Count the number of float parameters */
   virtual std::size_t countFloatParameters(const activityMode& am) const BASE;
   /** @brief Count the number of double parameters */
   virtual std::size_t countDoubleParameters(const activityMode& am) const BASE;
   /** @brief Count the number of boost::int32_t parameters */
   virtual std::size_t countInt32Parameters(const activityMode& am) const BASE;
   /** @brief Count the number of bool parameters */
   virtual std::size_t countBoolParameters(const activityMode& am) const BASE;

   /** @brief Attach boundaries of type float to the vectors */
   virtual void floatBoundaries(std::vector<float>&, std::vector<float>&, const activityMode&) const BASE;
   /** @brief Attach boundaries of type double to the vectors */
   virtual void doubleBoundaries(std::vector<double>&, std::vector<double>&, const activityMode&) const BASE;
   /** @brief Attach boundaries of type boost::int32_t to the vectors */
   virtual void int32Boundaries(std::vector<boost::int32_t>&, std::vector<boost::int32_t>&, const activityMode&) const BASE;
   /** @brief Attach boundaries of type bool to the vectors */
   virtual void booleanBoundaries(std::vector<bool>&, std::vector<bool>&, const activityMode&) const BASE;

   /** @brief Attach parameters of type float to the vector */
   virtual void floatStreamline(std::vector<float>&, const activityMode&) const BASE;
   /** @brief Attach parameters of type double to the vector */
   virtual void doubleStreamline(std::vector<double>&, const activityMode&) const BASE;
   /** @brief Attach parameters of type boost::int32_t to the vector */
   virtual void int32Streamline(std::vector<boost::int32_t>&, const activityMode&) const BASE;
   /** @brief Attach parameters of type bool to the vector */
   virtual void booleanStreamline(std::vector<bool>&, const activityMode&) const BASE;

   /** @brief Attach parameters of type float to the map */
   virtual void floatStreamline(std::map<std::string, std::vector<float> >&, const activityMode&) const BASE;
   /** @brief Attach parameters of type double to the map */
   virtual void doubleStreamline(std::map<std::string, std::vector<double> >&, const activityMode&) const BASE;
   /** @brief Attach parameters of type boost::int32_t to the map */
   virtual void int32Streamline(std::map<std::string, std::vector<boost::int32_t> >&, const activityMode&) const BASE;
   /** @brief Attach parameters of type bool to the map */
   virtual void booleanStreamline(std::map<std::string, std::vector<bool> >&, const activityMode&) const BASE;

   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignFloatValueVector(const std::vector<float>&, std::size_t&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignDoubleValueVector(const std::vector<double>&, std::size_t&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignInt32ValueVector(const std::vector<boost::int32_t>&, std::size_t&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignBooleanValueVector(const std::vector<bool>&, std::size_t&, const activityMode&) BASE;

   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignFloatValueVectors(const std::map<std::string, std::vector<float> >&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignDoubleValueVectors(const std::map<std::string, std::vector<double> >&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignInt32ValueVectors(const std::map<std::string, std::vector<boost::int32_t> >&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual void assignBooleanValueVectors(const std::map<std::string, std::vector<bool> >&, const activityMode&) BASE;

   /** @brief Multiplication with a random value in a given range */
   virtual void floatMultiplyByRandom(const float& min, const float& max, const activityMode& am) BASE;
   /** @brief Multiplication with a random value in a given range */
   virtual void doubleMultiplyByRandom(const double& min, const double& max, const activityMode& am) BASE;
   /** @brief Multiplication with a random value in a given range */
   virtual void int32MultiplyByRandom(const boost::int32_t& min, const boost::int32_t& max, const activityMode& am) BASE;
   /** @brief Multiplication with a random value in a given range */
   virtual void booleanMultiplyByRandom(const bool& min, const bool& max, const activityMode& am) BASE;

   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual void floatMultiplyByRandom(const activityMode& am) BASE;
   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual void doubleMultiplyByRandom(const activityMode& am) BASE;
   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual void int32MultiplyByRandom(const activityMode& am) BASE;
   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual void booleanMultiplyByRandom(const activityMode& am) BASE;

   /** @brief Multiplication with a constant value */
   virtual void floatMultiplyBy(const float& value, const activityMode& am) BASE;
   /** @brief Multiplication with a constant value */
   virtual void doubleMultiplyBy(const double& value, const activityMode& am) BASE;
   /** @brief Multiplication with a constant value */
   virtual void int32MultiplyBy(const boost::int32_t& value, const activityMode& am) BASE;
   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual void booleanMultiplyBy(const bool& value, const activityMode& am) BASE;

   /** @brief Initialization with a constant value */
   virtual void floatFixedValueInit(const float& value, const activityMode& am) BASE;
   /** @brief Initialization with a constant value */
   virtual void doubleFixedValueInit(const double& value, const activityMode& am) BASE;
   /** @brief Initialization with a constant value */
   virtual void int32FixedValueInit(const boost::int32_t& value, const activityMode& am) BASE;
   /** @brief Initialization with a random value in the range [0,1[ */
   virtual void booleanFixedValueInit(const bool& value, const activityMode& am) BASE;

   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual void floatAdd(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual void doubleAdd(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual void int32Add(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual void booleanAdd(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;

   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual void floatSubtract(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual void doubleSubtract(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual void int32Subtract(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual void booleanSubtract(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;

	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

	/** @brief Triggers random initialization of the parameter(-collection) */
	virtual void randomInit_() = 0;

private:
	bool adaptionsActive_; ///< Specifies whether adaptions of this object should be carried out
	bool randomInitializationBlocked_; ///< Specifies that this object should not be initialized again
	std::string parameterName_; ///< A name assigned to this parameter object

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/
/**
 * Specializations of some template functions
 */
template <>	void GParameterBase::streamline<float>(std::vector<float>&, const activityMode&) const;
template <>	void GParameterBase::streamline<double>(std::vector<double>&, const activityMode&) const;
template <>	void GParameterBase::streamline<boost::int32_t>(std::vector<boost::int32_t>&, const activityMode&) const;
template <>	void GParameterBase::streamline<bool>(std::vector<bool>&, const activityMode&) const;

template <> void GParameterBase::streamline<float>(std::map<std::string, std::vector<float> >&, const activityMode&) const;
template <> void GParameterBase::streamline<double>(std::map<std::string, std::vector<double> >&, const activityMode&) const;
template <> void GParameterBase::streamline<boost::int32_t>(std::map<std::string, std::vector<boost::int32_t> >&, const activityMode&) const;
template <> void GParameterBase::streamline<bool>(std::map<std::string, std::vector<bool> >&, const activityMode&) const;

template <>	void GParameterBase::boundaries<float>(std::vector<float>&, std::vector<float>&, const activityMode&) const;
template <>	void GParameterBase::boundaries<double>(std::vector<double>&, std::vector<double>&, const activityMode&) const;
template <>	void GParameterBase::boundaries<boost::int32_t>(std::vector<boost::int32_t>&, std::vector<boost::int32_t>&, const activityMode&) const;
template <>	void GParameterBase::boundaries<bool>(std::vector<bool>&, std::vector<bool>&, const activityMode&) const;

template <>	std::size_t GParameterBase::countParameters<float>(const activityMode& am) const;
template <>	std::size_t GParameterBase::countParameters<double>(const activityMode& am) const;
template <>	std::size_t GParameterBase::countParameters<boost::int32_t>(const activityMode& am) const;
template <>	std::size_t GParameterBase::countParameters<bool>(const activityMode& am) const;

template <>	void GParameterBase::assignValueVector<float>(const std::vector<float>&, std::size_t&, const activityMode&);
template <>	void GParameterBase::assignValueVector<double>(const std::vector<double>&, std::size_t&, const activityMode&);
template <>	void GParameterBase::assignValueVector<boost::int32_t>(const std::vector<boost::int32_t>&, std::size_t&, const activityMode&);
template <>	void GParameterBase::assignValueVector<bool>(const std::vector<bool>&, std::size_t&, const activityMode&);

template <> void GParameterBase::assignValueVectors<float>(const std::map<std::string, std::vector<float> >&, const activityMode&);
template <> void GParameterBase::assignValueVectors<double>(const std::map<std::string, std::vector<double> >&, const activityMode&);
template <> void GParameterBase::assignValueVectors<boost::int32_t>(const std::map<std::string, std::vector<boost::int32_t> >&, const activityMode&);
template <> void GParameterBase::assignValueVectors<bool>(const std::map<std::string, std::vector<bool> >&, const activityMode&);

template <> void GParameterBase::multiplyByRandom<float>(const float& min, const float& max, const activityMode& am);
template <> void GParameterBase::multiplyByRandom<double>(const double& min, const double& max, const activityMode& am);
template <> void GParameterBase::multiplyByRandom<boost::int32_t>(const boost::int32_t& min, const boost::int32_t& max, const activityMode& am);
template <> void GParameterBase::multiplyByRandom<bool>(const bool& min, const bool& max, const activityMode& am);

template <> void GParameterBase::multiplyByRandom<float>(const activityMode& am);
template <> void GParameterBase::multiplyByRandom<double>(const activityMode& am);
template <> void GParameterBase::multiplyByRandom<boost::int32_t>(const activityMode& am);
template <> void GParameterBase::multiplyByRandom<bool>(const activityMode& am);

template <> void GParameterBase::multiplyBy<float>(const float& val, const activityMode& am);
template <> void GParameterBase::multiplyBy<double>(const double& val, const activityMode& am);
template <> void GParameterBase::multiplyBy<boost::int32_t>(const boost::int32_t& val, const activityMode& am);
template <> void GParameterBase::multiplyBy<bool>(const bool& val, const activityMode& am);

template <> void GParameterBase::fixedValueInit<float>(const float& val, const activityMode& am);
template <> void GParameterBase::fixedValueInit<double>(const double& val, const activityMode& am);
template <> void GParameterBase::fixedValueInit<boost::int32_t>(const boost::int32_t& val, const activityMode& am);
template <> void GParameterBase::fixedValueInit<bool>(const bool& val, const activityMode& am);

template <> void GParameterBase::add<float>(boost::shared_ptr<GParameterBase> p, const activityMode& am);
template <> void GParameterBase::add<double>(boost::shared_ptr<GParameterBase> p, const activityMode& am);
template <> void GParameterBase::add<boost::int32_t>(boost::shared_ptr<GParameterBase> p, const activityMode& am);
template <> void GParameterBase::add<bool>(boost::shared_ptr<GParameterBase> p, const activityMode& am);

template <> void GParameterBase::subtract<float>(boost::shared_ptr<GParameterBase> p, const activityMode& am);
template <> void GParameterBase::subtract<double>(boost::shared_ptr<GParameterBase> p, const activityMode& am);
template <> void GParameterBase::subtract<boost::int32_t>(boost::shared_ptr<GParameterBase> p, const activityMode& am);
template <> void GParameterBase::subtract<bool>(boost::shared_ptr<GParameterBase> p, const activityMode& am);

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterBase)

/******************************************************************************/

#endif /* GPARAMETERBASE_HPP_ */
