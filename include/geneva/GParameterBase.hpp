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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here

// Boost header files go here
#include <boost/lexical_cast.hpp>

#ifndef GPARAMETERBASE_HPP_
#define GPARAMETERBASE_HPP_

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

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
      & BOOST_SERIALIZATION_NVP(adaptionsActive_)
      & BOOST_SERIALIZATION_NVP(randomInitializationBlocked_)
      & BOOST_SERIALIZATION_NVP(parameterName_);
    }
    ///////////////////////////////////////////////////////////////////////
public:
	/** @brief The standard constructor */
   G_API_GENEVA GParameterBase();
	/** @brief The copy constructor */
   G_API_GENEVA GParameterBase(const GParameterBase&);
	/** @brief The standard destructor */
	virtual G_API_GENEVA ~GParameterBase();

   /** @brief The standard assignment operator */
   G_API_GENEVA const GParameterBase& operator=(const GParameterBase&);

	/** @brief The adaption interface */
	virtual G_API_GENEVA std::size_t adapt() OVERRIDE;
	/** @brief The actual adaption logic */
	virtual G_API_GENEVA std::size_t adaptImpl() BASE = 0;

   /** @brief Triggers updates when the optimization process has stalled */
   virtual G_API_GENEVA bool updateAdaptorsOnStall(const std::size_t&) BASE = 0;
   /** @brief Retrieves information from an adaptor on a given property */
   virtual G_API_GENEVA void queryAdaptor(
      const std::string& adaptorName
      , const std::string& property
      , std::vector<boost::any>& data
   ) const BASE = 0;

	/** @brief Switches on adaptions for this object */
   G_API_GENEVA bool setAdaptionsActive();
	/** @brief Disables adaptions for this object */
   G_API_GENEVA bool setAdaptionsInactive();
	/** @brief Determines whether adaptions are performed for this object */
   G_API_GENEVA bool adaptionsActive() const;
   /** @brief Determines whether adaptions are inactive for this object */
   G_API_GENEVA bool adaptionsInactive() const;

	/** @brief Checks for equality with another GParameter Base object */
   G_API_GENEVA bool operator==(const GParameterBase&) const;
	/** @brief Checks for inequality with another GParameterBase object */
   G_API_GENEVA bool operator!=(const GParameterBase&) const;

	/** @brief Triggers random initialization of the parameter(-collection) */
   virtual G_API_GENEVA void randomInit(const activityMode&) BASE;

	/** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
	virtual G_API_GENEVA bool isIndividualParameter() const BASE;
	/** @brief Allows to identify whether we are dealing with a collection or an individual parameter */
	virtual G_API_GENEVA bool isParameterCollection() const BASE;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const OVERRIDE;

	/** @brief Allows to assign a name to this parameter */
	G_API_GENEVA void setParameterName(const std::string&);
	/** @brief Allows to retrieve the name of this parameter */
	G_API_GENEVA std::string getParameterName() const;

	/** @brief Checks whether this object matches a given activity mode */
	G_API_GENEVA bool amMatch(const activityMode&) const;
	/** @brief Returns true on the case of an activity mode mismatch */
	G_API_GENEVA bool amMismatch(const activityMode&) const;

	/** @brief Checks whether this object matches a given activity mode and is modifiable */
	G_API_GENEVA bool modifiableAmMatchOrHandover(const activityMode&) const;

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
   G_API_GENEVA void blockRandomInitialization();
	/** @brief Makes random initialization possible */
   G_API_GENEVA void allowRandomInitialization();
	/** @brief Checks whether initialization has been blocked */
   G_API_GENEVA bool randomInitializationBlocked() const;

	/** @brief Convenience function so we do not need to always cast derived classes */
	virtual G_API_GENEVA bool hasAdaptor() const BASE;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

   /** @brief Searches for compliance with expectations with respect to another object of the same type */
   virtual G_API_GENEVA void compare(
      const GObject& // the other object
      , const Gem::Common::expectation& // the expectation for this object, e.g. equality
      , const double& // the limit for allowed deviations of floating point types
   ) const OVERRIDE;

   /** @brief Converts the local data to a boost::property_tree node */
   virtual G_API_GENEVA void toPropertyTree(pt::ptree&, const std::string&) const = 0;
   /** @brief Lets the audience know whether this is a leaf or a branch object */
   virtual G_API_GENEVA bool isLeaf() const BASE;

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
		   << "In boost::shared_ptr<load_type> GParameterBase::parameterbase_cast<load_type>() :" << std::endl
         << "Invalid conversion with load_type = " << typeid(load_type).name() << std::endl
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
   /** @brief Count the number of float parameters */
   virtual G_API_GENEVA std::size_t countFloatParameters(const activityMode& am) const BASE;
   /** @brief Count the number of double parameters */
   virtual G_API_GENEVA std::size_t countDoubleParameters(const activityMode& am) const BASE;
   /** @brief Count the number of boost::int32_t parameters */
   virtual G_API_GENEVA std::size_t countInt32Parameters(const activityMode& am) const BASE;
   /** @brief Count the number of bool parameters */
   virtual G_API_GENEVA std::size_t countBoolParameters(const activityMode& am) const BASE;

   /** @brief Attach boundaries of type float to the vectors */
   virtual G_API_GENEVA void floatBoundaries(std::vector<float>&, std::vector<float>&, const activityMode&) const BASE;
   /** @brief Attach boundaries of type double to the vectors */
   virtual G_API_GENEVA void doubleBoundaries(std::vector<double>&, std::vector<double>&, const activityMode&) const BASE;
   /** @brief Attach boundaries of type boost::int32_t to the vectors */
   virtual G_API_GENEVA void int32Boundaries(std::vector<boost::int32_t>&, std::vector<boost::int32_t>&, const activityMode&) const BASE;
   /** @brief Attach boundaries of type bool to the vectors */
   virtual G_API_GENEVA void booleanBoundaries(std::vector<bool>&, std::vector<bool>&, const activityMode&) const BASE;

   /** @brief Attach parameters of type float to the vector */
   virtual G_API_GENEVA void floatStreamline(std::vector<float>&, const activityMode&) const BASE;
   /** @brief Attach parameters of type double to the vector */
   virtual G_API_GENEVA void doubleStreamline(std::vector<double>&, const activityMode&) const BASE;
   /** @brief Attach parameters of type boost::int32_t to the vector */
   virtual G_API_GENEVA void int32Streamline(std::vector<boost::int32_t>&, const activityMode&) const BASE;
   /** @brief Attach parameters of type bool to the vector */
   virtual G_API_GENEVA void booleanStreamline(std::vector<bool>&, const activityMode&) const BASE;

   /** @brief Attach parameters of type float to the map */
   virtual G_API_GENEVA void floatStreamline(std::map<std::string, std::vector<float> >&, const activityMode&) const BASE;
   /** @brief Attach parameters of type double to the map */
   virtual G_API_GENEVA void doubleStreamline(std::map<std::string, std::vector<double> >&, const activityMode&) const BASE;
   /** @brief Attach parameters of type boost::int32_t to the map */
   virtual G_API_GENEVA void int32Streamline(std::map<std::string, std::vector<boost::int32_t> >&, const activityMode&) const BASE;
   /** @brief Attach parameters of type bool to the map */
   virtual G_API_GENEVA void booleanStreamline(std::map<std::string, std::vector<bool> >&, const activityMode&) const BASE;

   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignFloatValueVector(const std::vector<float>&, std::size_t&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignDoubleValueVector(const std::vector<double>&, std::size_t&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignInt32ValueVector(const std::vector<boost::int32_t>&, std::size_t&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignBooleanValueVector(const std::vector<bool>&, std::size_t&, const activityMode&) BASE;

   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignFloatValueVectors(const std::map<std::string, std::vector<float> >&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignDoubleValueVectors(const std::map<std::string, std::vector<double> >&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignInt32ValueVectors(const std::map<std::string, std::vector<boost::int32_t> >&, const activityMode&) BASE;
   /** @brief Assigns part of a value vector to the parameter */
   virtual G_API_GENEVA void assignBooleanValueVectors(const std::map<std::string, std::vector<bool> >&, const activityMode&) BASE;

   /** @brief Multiplication with a random value in a given range */
   virtual G_API_GENEVA void floatMultiplyByRandom(const float& min, const float& max, const activityMode& am) BASE;
   /** @brief Multiplication with a random value in a given range */
   virtual G_API_GENEVA void doubleMultiplyByRandom(const double& min, const double& max, const activityMode& am) BASE;
   /** @brief Multiplication with a random value in a given range */
   virtual G_API_GENEVA void int32MultiplyByRandom(const boost::int32_t& min, const boost::int32_t& max, const activityMode& am) BASE;
   /** @brief Multiplication with a random value in a given range */
   virtual G_API_GENEVA void booleanMultiplyByRandom(const bool& min, const bool& max, const activityMode& am) BASE;

   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual G_API_GENEVA void floatMultiplyByRandom(const activityMode& am) BASE;
   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual G_API_GENEVA void doubleMultiplyByRandom(const activityMode& am) BASE;
   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual G_API_GENEVA void int32MultiplyByRandom(const activityMode& am) BASE;
   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual G_API_GENEVA void booleanMultiplyByRandom(const activityMode& am) BASE;

   /** @brief Multiplication with a constant value */
   virtual G_API_GENEVA void floatMultiplyBy(const float& value, const activityMode& am) BASE;
   /** @brief Multiplication with a constant value */
   virtual G_API_GENEVA void doubleMultiplyBy(const double& value, const activityMode& am) BASE;
   /** @brief Multiplication with a constant value */
   virtual G_API_GENEVA void int32MultiplyBy(const boost::int32_t& value, const activityMode& am) BASE;
   /** @brief Multiplication with a random value in the range [0,1[ */
   virtual G_API_GENEVA void booleanMultiplyBy(const bool& value, const activityMode& am) BASE;

   /** @brief Initialization with a constant value */
   virtual G_API_GENEVA void floatFixedValueInit(const float& value, const activityMode& am) BASE;
   /** @brief Initialization with a constant value */
   virtual G_API_GENEVA void doubleFixedValueInit(const double& value, const activityMode& am) BASE;
   /** @brief Initialization with a constant value */
   virtual G_API_GENEVA void int32FixedValueInit(const boost::int32_t& value, const activityMode& am) BASE;
   /** @brief Initialization with a random value in the range [0,1[ */
   virtual G_API_GENEVA void booleanFixedValueInit(const bool& value, const activityMode& am) BASE;

   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual G_API_GENEVA void floatAdd(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual G_API_GENEVA void doubleAdd(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual G_API_GENEVA void int32Add(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual G_API_GENEVA void booleanAdd(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;

   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual G_API_GENEVA void floatSubtract(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual G_API_GENEVA void doubleSubtract(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual G_API_GENEVA void int32Subtract(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;
   /** @brief Adds the "same-type" parameters of another GParameterBase object to this one */
   virtual G_API_GENEVA void booleanSubtract(boost::shared_ptr<GParameterBase>, const activityMode& am) BASE;

	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual G_API_GENEVA void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject* clone_() const = 0;

	/** @brief Triggers random initialization of the parameter(-collection) */
	virtual G_API_GENEVA void randomInit_(const activityMode&) BASE = 0;

private:
	bool adaptionsActive_; ///< Specifies whether adaptions of this object should be carried out
	bool randomInitializationBlocked_; ///< Specifies that this object should not be initialized again
	std::string parameterName_; ///< A name assigned to this parameter object

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/
/**
 * Specializations of some template functions
 */

/******************************************************************************/
/**
 * Allows to add all parameters of type float to the vector.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<float>(
   std::vector<float>& parVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->floatStreamline(parVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type double to the vector.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<double>(
   std::vector<double>& parVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->doubleStreamline(parVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type boost::int32_t to the vector.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<boost::int32_t>(
   std::vector<boost::int32_t>& parVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->int32Streamline(parVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type bool to the vector.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<bool>(
   std::vector<bool>& parVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->booleanStreamline(parVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type float to the map.
 *
 * @oaram parVec The map to which the items should be added
 */
template <>
inline void GParameterBase::streamline<float>(
   std::map<std::string, std::vector<float> >& parVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->floatStreamline(parVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type double to the map.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<double>(
   std::map<std::string, std::vector<double> >& parVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->doubleStreamline(parVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type boost::int32_t to the map.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<boost::int32_t>(
   std::map<std::string, std::vector<boost::int32_t> >& parVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->int32Streamline(parVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to add all parameters of type bool to the map.
 *
 * @oaram parVec The vector to which the items should be added
 */
template <>
inline void GParameterBase::streamline<bool>(
   std::map<std::string, std::vector<bool> >& parVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->booleanStreamline(parVec, am);
   }
}


/******************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type float
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
template <>
inline void GParameterBase::boundaries<float>(
   std::vector<float>& lBndVec
   , std::vector<float>& uBndVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->floatBoundaries(lBndVec, uBndVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type double
 *
 * @param lBndVec A vector of lower double parameter boundaries
 * @param uBndVec A vector of upper double parameter boundaries
 */
template <>
inline void GParameterBase::boundaries<double>(
   std::vector<double>& lBndVec
   , std::vector<double>& uBndVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->doubleBoundaries(lBndVec, uBndVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type boost::int32_t
 *
 * @param lBndVec A vector of lower boost::int32_t parameter boundaries
 * @param uBndVec A vector of upper boost::int32_t parameter boundaries
 */
template <>
inline void GParameterBase::boundaries<boost::int32_t>(
   std::vector<boost::int32_t>& lBndVec
   , std::vector<boost::int32_t>& uBndVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->int32Boundaries(lBndVec, uBndVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to retrieve the values of lower and upper boundaries of type bool
 *
 * @param lBndVec A vector of lower bool parameter boundaries
 * @param uBndVec A vector of upper bool parameter boundaries
 */
template <>
inline void GParameterBase::boundaries<bool>(
   std::vector<bool>& lBndVec
   , std::vector<bool>& uBndVec
   , const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->booleanBoundaries(lBndVec, uBndVec, am);
   }
}

/******************************************************************************/
/**
 * Allows to count parameters of type float.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of parameters of type float
 */
template <>
inline std::size_t GParameterBase::countParameters<float>(
   const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      return this->countFloatParameters(am);
   } else {
      return 0;
   }
}

/******************************************************************************/
/**
 * Allows to count parameters of type double.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of parameters of type double
 */
template <>
inline std::size_t GParameterBase::countParameters<double>(
   const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      return this->countDoubleParameters(am);
   } else {
      return 0;
   }
}

/******************************************************************************/
/**
 * Allows to count parameters of type boost::int32_t.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of parameters of type boost::int32_t
 */
template <>
inline std::size_t GParameterBase::countParameters<boost::int32_t>(
   const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      return this->countInt32Parameters(am);
   } else {
      return 0;
   }
}

/******************************************************************************/
/**
 * Allows to count parameters of type bool.
 *
 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
 * @return The number of parameters of type bool
 */
template <>
inline std::size_t GParameterBase::countParameters<bool>(
   const activityMode& am
) const {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      return this->countBoolParameters(am);
   } else {
      return 0;
   }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param parVec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
inline void GParameterBase::assignValueVector<float>(
   const std::vector<float>& parVec
   , std::size_t& pos
   , const activityMode& am
) {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->assignFloatValueVector(parVec, pos, am);
   }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param parVec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
inline void GParameterBase::assignValueVector<double>(
   const std::vector<double>& parVec
   , std::size_t& pos
   , const activityMode& am
) {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->assignDoubleValueVector(parVec, pos, am);
   }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param parVec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
inline void GParameterBase::assignValueVector<boost::int32_t>(
   const std::vector<boost::int32_t>& parVec
   , std::size_t& pos
   , const activityMode& am
) {
   this->assignInt32ValueVector(parVec, pos, am);
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a vector the corresponding parameter objects.
 *
 * @param parVec The vector with the parameters to be assigned to the object
 * @param pos The position from which parameters will be taken (will be updated by the call)
 */
template <>
inline void GParameterBase::assignValueVector<bool>(
   const std::vector<bool>& parVec
   , std::size_t& pos
   , const activityMode& am
) {
   this->assignBooleanValueVector(parVec, pos, am);
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a map to the corresponding parameter objects.
 *
 * @param parMap The vector with the parameters to be assigned to the object
 */
template <>
inline void GParameterBase::assignValueVectors<float>(
   const std::map<std::string
   , std::vector<float> >& parMap
   , const activityMode& am
) {
   if(
      this->modifiableAmMatchOrHandover(am)
   ) {
      this->assignFloatValueVectors(parMap, am);
   }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a map to the corresponding parameter objects.
 *
 * @param parMap The vector with the parameters to be assigned to the object
 */
template <>
inline  void GParameterBase::assignValueVectors<double>(
   const std::map<std::string
   , std::vector<double> >& parMap
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->assignDoubleValueVectors(parMap, am);
    }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a map to the corresponding parameter objects.
 *
 * @param parMap The vector with the parameters to be assigned to the object
 */
template <>
inline  void GParameterBase::assignValueVectors<boost::int32_t>(
   const std::map<std::string, std::vector<boost::int32_t> >& parMap
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->assignInt32ValueVectors(parMap, am);
    }
}

/******************************************************************************/
/**
 * Allows to assign the parameters inside of a map to the corresponding parameter objects.
 *
 * @param parMap The vector with the parameters to be assigned to the object
 */
template <>
inline  void GParameterBase::assignValueVectors<bool>(
   const std::map<std::string, std::vector<bool> >& parMap
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->assignBooleanValueVectors(parMap, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
template <>
inline  void GParameterBase::multiplyByRandom<float>(
   const float& min
   , const float& max
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->floatMultiplyByRandom(min, max, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
template <>
inline  void GParameterBase::multiplyByRandom<double>(
   const double& min
   , const double& max
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->doubleMultiplyByRandom(min, max, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range
 */
template <>
inline  void GParameterBase::multiplyByRandom<boost::int32_t>(
   const boost::int32_t& min
   , const boost::int32_t& max
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->int32MultiplyByRandom(min, max, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in a given range. This specialization for
 * boolean values has been added for completeness and error-detection. It will throw
 * when called.
 */
template <>
inline  void GParameterBase::multiplyByRandom<bool>(
   const bool& min
   , const bool& max
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      // NOTE: This will throw
      this->booleanMultiplyByRandom(min, max, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
template <>
inline  void GParameterBase::multiplyByRandom<float>(
   const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->floatMultiplyByRandom(am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
template <>
inline  void GParameterBase::multiplyByRandom<double>(
   const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->doubleMultiplyByRandom(am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[
 */
template <>
inline  void GParameterBase::multiplyByRandom<boost::int32_t>(
   const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->int32MultiplyByRandom(am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a random value in the range [0,1[. This specialization for
 * boolean values has been added for completeness and error-detection. It will throw
 * when called.
 */
template <>
inline  void GParameterBase::multiplyByRandom<bool>(
   const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      // NOTE: This will throw
      this->booleanMultiplyByRandom(am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
template <>
inline  void GParameterBase::multiplyBy<float>(
   const float& val
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->floatMultiplyBy(val, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
template <>
inline  void GParameterBase::multiplyBy<double>(
   const double& val
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->doubleMultiplyBy(val, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value
 */
template <>
inline  void GParameterBase::multiplyBy<boost::int32_t>(
   const boost::int32_t& val
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->int32MultiplyBy(val, am);
    }
}

/******************************************************************************/
/**
 * Multiplication with a constant value. This specialization for
 * boolean values has been added for completeness and error-detection.
 * It will throw when called.
 */
template <>
inline  void GParameterBase::multiplyBy<bool>(
   const bool& val
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      // NOTE: This will throw
      this->booleanMultiplyBy(val, am);
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
template <>
inline  void GParameterBase::fixedValueInit<float>(
   const float& val
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->floatFixedValueInit(val, am);
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
template <>
inline  void GParameterBase::fixedValueInit<double>(
   const double& val
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->doubleFixedValueInit(val, am);
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
template <>
inline  void GParameterBase::fixedValueInit<boost::int32_t>(
   const boost::int32_t& val
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->int32FixedValueInit(val, am);
    }
}

/******************************************************************************/
/**
 * Initialization with a constant value
 */
template <>
inline  void GParameterBase::fixedValueInit<bool>(
   const bool& val
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->booleanFixedValueInit(val, am);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
template <>
inline  void GParameterBase::add<float>(
   boost::shared_ptr<GParameterBase> p
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->floatAdd(p, am);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
template <>
inline  void GParameterBase::add<double>(
   boost::shared_ptr<GParameterBase> p
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->doubleAdd(p, am);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one
 */
template <>
inline  void GParameterBase::add<boost::int32_t>(
   boost::shared_ptr<GParameterBase> p
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->int32Add(p, am);
    }
}

/******************************************************************************/
/**
 * Adds the "same-type" parameters of another GParameterBase object to this one.
 * This specialization for boolean values has been added for completeness and error-detection.
 * It will throw when called.
 */
template <>
inline  void GParameterBase::add<bool>(
   boost::shared_ptr<GParameterBase> p
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      // Note: This call will throw!
      this->booleanAdd(p, am);
    }
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
template <>
inline  void GParameterBase::subtract<float>(
   boost::shared_ptr<GParameterBase> p
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->floatSubtract(p, am);
    }
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
template <>
inline  void GParameterBase::subtract<double>(
   boost::shared_ptr<GParameterBase> p
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->doubleSubtract(p, am);
    }
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one
 */
template <>
inline  void GParameterBase::subtract<boost::int32_t>(
   boost::shared_ptr<GParameterBase> p
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      this->int32Subtract(p, am);
    }
}

/******************************************************************************/
/**
 * Subtracts the "same-type" parameters of another GParameterBase object from this one.
 * This specialization for boolean values has been added for completeness and error-detection.
 * It will throw when called.
 */
template <>
inline  void GParameterBase::subtract<bool>(
   boost::shared_ptr<GParameterBase> p
   , const activityMode& am
) {
   if(
       this->modifiableAmMatchOrHandover(am)
    ) {
      // NOTE: This call will throw
      this->booleanSubtract(p, am);
    }
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterBase)

/******************************************************************************/

#endif /* GPARAMETERBASE_HPP_ */
