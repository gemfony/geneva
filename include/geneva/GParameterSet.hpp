/**
 * @file GParameterSet.hpp
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
#include <map>

// Boost header files go here

#ifndef GPARAMETERSET_HPP_
#define GPARAMETERSET_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "courtier/GSubmissionContainerT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GParameterBase.hpp"

#ifdef GEM_TESTING
#include "geneva/GBooleanObject.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GParameterObjectCollection.hpp"
#include "geneva/GInt32Collection.hpp"
#include "common/GUnitTestFrameworkT.hpp"
#include "hap/GRandomT.hpp"
#endif /* GEM_TESTING */

namespace Gem {

namespace Tests {
class GTestIndividual1; // forward declaration, needed for testing purposes
} /* namespace Tests */

namespace Geneva {

/******************************************************************************/
/**
 * This class implements a collection of GParameterBase objects. It
 * will form the basis of many user-defined individuals.
 */
class GParameterSet
	: public GMutableSetT<Gem::Geneva::GParameterBase>
   , public Gem::Courtier::GSubmissionContainerT<GParameterSet>
{
	friend class Gem::Tests::GTestIndividual1; ///< Needed for testing purposes

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;
	  ar
	  & make_nvp("GMutableSetT_GParameterBase", boost::serialization::base_object<GMutableSetT<Gem::Geneva::GParameterBase> >(*this))
     & make_nvp("GSubmissionContainerT_ParameterSet", boost::serialization::base_object<Gem::Courtier::GSubmissionContainerT<GParameterSet> >(*this))
	  & BOOST_SERIALIZATION_NVP(perItemCrossOverProbability_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GParameterSet();
   /** @brief Initialization with the number of fitness criteria */
   GParameterSet(const std::size_t&);
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
	virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	/** @brief Allows to randomly initialize parameter members */
	virtual void randomInit(const activityMode&) OVERRIDE;

	/** @brief Specify whether we want to work in maximization (true) or minimization (false) mode */
	void setMaxMode(const bool&);

	/** @brief Emits a GParameterSet object that only has the GParameterBase objects attached to it */
	boost::shared_ptr<GParameterSet> parameter_clone() const;

   /** @brief Do the required processing for this object */
   virtual bool process() OVERRIDE;

	/** @brief Updates the random number generators contained in this object's GParameterBase-derivatives */
	virtual void updateRNGs() BASE;
	/** @brief Restores the local random number generators contained in this object's GParameterBase-derivatives */
	virtual void restoreRNGs() BASE;
	/** @brief Checks whether all GParameterBase derivatives use local random number generators */
	virtual bool localRNGsUsed() const BASE;
	/** @brief Checks whether all GParameterBase derivatives use the assigned random number generator */
	virtual bool assignedRNGUsed() const BASE;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&) OVERRIDE;

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual std::string getIndividualCharacteristic() const OVERRIDE;

	/** @brief Provides access to all data stored in the individual in a user defined selection */
	virtual void custom_streamline(std::vector<boost::any>&) BASE;

	/** @brief Transformation of the individual's parameter objects into a boost::property_tree object */
	void toPropertyTree(pt::ptree&, const std::string& = "parameterset") const BASE;
	/** @brief Transformation of the individual's parameter objects into a list of comma-separated values */
	std::string toCSV(bool=false, bool=true, bool=true, bool=true) const;

	/** @brief Emits a name for this class / object */
	virtual std::string name() const OVERRIDE;

   /** @brief Retrieves a parameter of a given type at the specified position */
   virtual boost::any getVarVal(
      const std::string&
      , const boost::tuple<std::size_t, std::string, std::size_t>& target
   ) OVERRIDE;

	/** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
   GMutableSetT<Gem::Geneva::GParameterBase>::reference at(const std::size_t& pos);

	/** @brief Checks whether this object is better than a given set of evaluations */
	bool isGoodEnough(const std::vector<double>&);

   /** @brief Perform a fusion operation between this object and another */
   virtual boost::shared_ptr<GParameterSet> amalgamate(const boost::shared_ptr<GParameterSet>) const BASE;

   /** @brief Performs a cross-over with another GParameterSet object on a "per item" basis */
   void perItemCrossOver(const GParameterSet&, const double&);

   /** @brief Allows to set the "per item" cross-over probability */
   void setPerItemCrossOverProbability(double);
   /** @brief Allows to retrieve the "per item" cross-over probability */
   double getPerItemCrossOverProbability() const;

   /** @brief Triggers updates of adaptors contained in this object */
   virtual void updateAdaptorsOnStall(const boost::uint32_t&);
   /** @brief Retrieves information from adaptors with a given property */
   virtual void queryAdaptor(
      const std::string& adaptorName
      , const std::string& property
      , std::vector<boost::any>& data
   ) const BASE;

	/***************************************************************************/
	/**
	 * This function returns a parameter set at a given position of the data set.
	 * Note that this function will only be accessible to the compiler if par_type
	 * is a derivative of GParameterBase, thanks to the magic of Boost's enable_if and
	 * Type Traits libraries.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GParameterBase object, as required by the user
	 */
	template <typename par_type>
	const boost::shared_ptr<par_type> at(
      const std::size_t& pos
      , typename boost::enable_if<boost::is_base_of<GParameterBase, par_type> >::type* dummy = 0
	)  const {
      // Does error checks on the conversion internally
      return Gem::Common::convertSmartPointer<GParameterBase, par_type>(data.at(pos));
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested. See also the second version of the at() function.
	 * ----------------------------------------------------------------------------------
	 */

	/******************************************************************************/
	/**
	 * Allows to retrieve a list of all variable names registered with the parameter set
	 */
   template <typename par_type>
	std::vector<std::string> getVariableNames() const {
      std::vector<std::string> varNames;
	   std::map<std::string, std::vector<par_type> > pMap;
	   this->streamline<par_type>(pMap);

	   typename std::map<std::string, std::vector<par_type> >::const_iterator cit;
	   for(cit=pMap.begin(); cit!=pMap.end(); ++cit) {
	      varNames.push_back(cit->first);
	   }
	}

   /***************************************************************************/
	/**
	 * Retrieves an item according to a description provided by the target tuple
	 */
	template <typename par_type>
	boost::any getVarItem(
      const boost::tuple<std::size_t, std::string, std::size_t>& target
	) {
	   boost::any result;

      switch(boost::get<0>(target)) {
         //---------------------------------------------------------------------
         case 0:
         {
            std::vector<par_type> vars;
            this->streamline<par_type>(vars);
            result = vars.at(boost::get<2>(target));
         }
         break;

         //---------------------------------------------------------------------
         case 1: // var[3]
         case 2: // var    --> treated as var[0]
         {
            std::map<std::string, std::vector<par_type> > varMap;
            this->streamline<par_type>(varMap);
            result = (Gem::Common::getMapItem<std::vector<par_type> >(varMap, boost::get<1>(target))).at(boost::get<2>(target));
         }
         break;

         //---------------------------------------------------------------------
         default:
         {
            glogger
            << "In GParameterSet::getVarVal(): Error!" << std::endl
            << "Got invalid mode setting: " << boost::get<0>(target) << std::endl
            << GEXCEPTION;
         }
         break;

         //---------------------------------------------------------------------
      }

      return result;
	}

   /***************************************************************************/
   /**
    * Retrieve information about the total number of parameters of type
    * par_type in the individual. Note that the GParameterBase-template
    * function will throw if this function is called for an unsupported type.
    *
    * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
    */
   template <typename par_type>
   std::size_t countParameters(
      const activityMode& am = DEFAULTACTIVITYMODE
   ) const {
      std::size_t result = 0;

      // Loop over all GParameterBase objects. Each object
      // will contribute the amount of its parameters of this type
      // to the result.
      GParameterSet::const_iterator cit;
      for(cit=this->begin(); cit!=this->end(); ++cit) {
         result += (*cit)->countParameters<par_type>(am);
      }

      return result;
   }

   /* ----------------------------------------------------------------------------------
    * So far untested.
    * ----------------------------------------------------------------------------------
    */

	/***************************************************************************/
	/**
	 * Loops over all GParameterBase objects. Each object will add the
	 * values of its parameters to the vector, if they comply with the
	 * type of the parameters to be stored in the vector.
	 *
	 * @param parVec The vector to which the parameters will be added
	 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
	 */
	template <typename par_type>
	void streamline(
      std::vector<par_type>& parVec
      , const activityMode& am = DEFAULTACTIVITYMODE
	) const {
		// Make sure the vector is clean
		parVec.clear();

		// Loop over all GParameterBase objects.
		GParameterSet::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			(*cit)->streamline<par_type>(parVec, am);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested.
	 * ----------------------------------------------------------------------------------
	 */

   /***************************************************************************/
   /**
    * Loops over all GParameterBase objects. Each object will add its name
    * and the values of its parameters to the map, if they comply with the
    * type of the parameters to be stored in the vector.
    *
    * @param parVec The map to which the parameters will be added
    * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
    */
   template <typename par_type>
   void streamline(
      std::map<std::string, std::vector<par_type> >& parVec
      , const activityMode& am = DEFAULTACTIVITYMODE
   ) const {
      // Make sure the vector is clean
      parVec.clear();

      // Loop over all GParameterBase objects.
      GParameterSet::const_iterator cit;
      for(cit=this->begin(); cit!=this->end(); ++cit) {
         (*cit)->streamline<par_type>(parVec, am);
      }
   }

   /* ----------------------------------------------------------------------------------
    * So far untested.
    * ----------------------------------------------------------------------------------
    */

	/***************************************************************************/
	/**
	 * Assigns values from a std::vector to the parameters in the collection
	 *
	 * @param parVec A vector of values, to be assigned to be added to GParameterBase derivatives
	 * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be assigned
	 */
	template <typename par_type>
	void assignValueVector(
      const std::vector<par_type>& parVec
      , const activityMode& am = DEFAULTACTIVITYMODE
   ) {
#ifdef DEBUG
		if(countParameters<par_type>() != parVec.size()) {
		   glogger
		   << "In GParameterSet::assignValueVector(const std::vector<pat_type>&):" << std::endl
         << "Sizes don't match: " <<  countParameters<par_type>() << " / " << parVec.size() << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		// Start assignment at the beginning of parVec
		std::size_t pos = 0;

		// Loop over all GParameterBase objects. Each object will extract the relevant
		// parameters and increment the position counter as required.
		GParameterSet::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			(*cit)->assignValueVector<par_type>(parVec, pos, am);
		}

		// As we have modified our internal data sets, make sure the dirty flag is set
		GOptimizableEntity::setDirtyFlag();
	}

   /***************************************************************************/
   /**
    * Assigns values from a std::map<std::string, std::vector<par_type> > to the parameters in the collection
    *
    * @param parMap A map of values, to be assigned to be added to GParameterBase derivatives
    * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be assigned
    */
   template <typename par_type>
   void assignValueVectors(
      const std::map<std::string, std::vector<par_type> >& parMap
      , const activityMode& am = DEFAULTACTIVITYMODE
   ) {
      // Loop over all GParameterBase objects. Each object will extract the relevant parameters
      GParameterSet::const_iterator cit;
      for(cit=this->begin(); cit!=this->end(); ++cit) {
         (*cit)->assignValueVectors<par_type>(parMap, am);
      }

      // As we have modified our internal data sets, make sure the dirty flag is set
      GOptimizableEntity::setDirtyFlag();
   }

   /***************************************************************************/
   /**
    * Loops over all GParameterBase objects. Each object will add the
    * lower and upper boundaries of its parameters to the vector, if
    * they comply with the type of the parameters to be stored in the
    * vector.
    *
    * @param lBndVec The vector to which the lower boundaries will be added
    * @param uBndVec The vector to which the upper boundaries will be added
    * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
    */
   template <typename par_type>
   void boundaries(
      std::vector<par_type>& lBndVec
      , std::vector<par_type>& uBndVec
      , const activityMode& am = DEFAULTACTIVITYMODE
   ) const {
      // Make sure the vectors are clean
      lBndVec.clear();
      uBndVec.clear();

      // Loop over all GParameterBase objects.
      GParameterSet::const_iterator cit;
      for(cit=this->begin(); cit!=this->end(); ++cit) {
         (*cit)->boundaries<par_type>(lBndVec, uBndVec, am);
      }
   }

   /* ----------------------------------------------------------------------------------
    * So far untested.
    * ----------------------------------------------------------------------------------
    */

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
      // Loop over all GParameterBase objects.
      GParameterSet::iterator it;
      for(it=this->begin(); it!=this->end(); ++it) {
         (*it)->multiplyByRandom<par_type>(min, max, am);
      }
   }

   /***************************************************************************/
   /**
    * Multiplication with a random value in the range [0, 1[
    */
   template <typename par_type>
   void multiplyByRandom(
      const activityMode& am
   ) {
      // Loop over all GParameterBase objects.
      GParameterSet::iterator it;
      for(it=this->begin(); it!=this->end(); ++it) {
         (*it)->multiplyByRandom<par_type>(am);
      }
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
      // Loop over all GParameterBase objects.
      GParameterSet::iterator it;
      for(it=this->begin(); it!=this->end(); ++it) {
         (*it)->multiplyBy<par_type>(val, am);
      }
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
      // Loop over all GParameterBase objects.
      GParameterSet::iterator it;
      for(it=this->begin(); it!=this->end(); ++it) {
         (*it)->fixedValueInit<par_type>(val, am);
      }
   }

   /***************************************************************************/
   /**
    * Adds the parameters of another GParameterSet object to this one
    */
   template <typename par_type>
   void add(
      boost::shared_ptr<GParameterSet> p
      , const activityMode& am
   ) {
      GParameterSet::iterator it;
      GParameterSet::const_iterator cit;

      // Note that the GParameterBase objects need to accept a
      // boost::shared_ptr<GParameterBase>, contrary to the calling conventions
      // of this function.
      for(it=this->begin(), cit=p->begin(); it!=this->end(); ++it, ++cit) {
         (*it)->add<par_type>(*cit, am);
      }
   }

   /***************************************************************************/
   /**
    * Subtracts the parameters of another GParameterSet object from this one
    */
   template <typename par_type>
   void subtract(
      boost::shared_ptr<GParameterSet> p
      , const activityMode& am
   ) {
      GParameterSet::iterator it;
      GParameterSet::const_iterator cit;

      // Note that the GParameterBase objects need to accept a
      // boost::shared_ptr<GParameterBase>, contrary to the calling conventions
      // of this function.
      for(it=this->begin(), cit=p->begin(); it!=this->end(); ++it, ++cit) {
         (*it)->subtract<par_type>(*cit, am);
      }
   }

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const OVERRIDE;

	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation() OVERRIDE ;
	/* @brief The actual adaption operations. */
	virtual std::size_t customAdaptions() OVERRIDE ;

private:
	explicit GParameterSet(const float&); ///< Intentionally private and undefined

	double perItemCrossOverProbability_; ///< A likelihood for "per item" cross-over operations to be performed

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
	/***************************************************************************/
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterSet)

/******************************************************************************/

#endif /* GPARAMETERSET_HPP_ */
