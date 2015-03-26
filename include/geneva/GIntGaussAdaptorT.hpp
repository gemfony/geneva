/**
 * @file GIntGaussAdaptorT.hpp
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

// Standard headers go here

// Boost headers go here

#ifndef GINTGAUSSADAPTORT_HPP_
#define GINTGAUSSADAPTORT_HPP_

// Geneva headers go here

#include "geneva/GNumGaussAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GIntGaussAdaptorT<> class represents an adaptor used for the adaption of
 * integer values through the addition of gaussian-distributed random numbers.
 * See the documentation of GAdaptorT<T> for further information on adaptors
 * in the Geneva context. Most functionality is currently implemented in the
 * GNumGaussAdaptorT parent-class. Note that, for the purpose of adapting integer
 * values, it is generally not useful to choose very small sigma values for the
 * gaussian. A value of 1 might be a good choice. Similarly, the minSigma
 * parameter should be set accordingly, so sigma cannot get too small when
 * being adapted.
 */
template <typename int_type>
class GIntGaussAdaptorT
	:public GNumGaussAdaptorT<int_type, double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar
	  & make_nvp("GNumGaussAdaptorT_int", boost::serialization::base_object<GNumGaussAdaptorT<int_type, double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GIntGaussAdaptorT()
		: GNumGaussAdaptorT<int_type, double>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA)
    { /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GIntGaussAdaptorT<int_type> object
	 */
	GIntGaussAdaptorT(const GIntGaussAdaptorT<int_type>& cp)
		: GNumGaussAdaptorT<int_type, double>(cp)
    { /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a adaption probability.  Note that we need to use a different default
	 * value for sigma, as there is a "natural" gap of 1 between integers, and the DEFAULTSIGMA
	 * might not be suitable for us.
	 *
	 * @param adProb The adaption probability
	 */
	explicit GIntGaussAdaptorT(const double& adProb)
		: GNumGaussAdaptorT<int_type, double>(DEFAULTINT32SIGMA, DEFAULTSIGMASIGMA, DEFAULTMINSIGMA, DEFAULTMAXSIGMA, adProb)
    { /* nothing */ }

	/***************************************************************************/
	/**
	 * This constructor lets a user set all sigma parameters in one go.
	 *
	 * @param sigma The initial value for the sigma_ parameter
	 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	 * @param minSigma The minimal value allowed for sigma_
	 * @param maxSigma The maximal value allowed for sigma_
	 */
	GIntGaussAdaptorT(
		const double& sigma
		, const double& sigmaSigma
		, const double& minSigma
		, const double& maxSigma
	)
		: GNumGaussAdaptorT<int_type, double> (sigma, sigmaSigma, minSigma, maxSigma)
    { /* nothing */ }

	/***************************************************************************/
	/**
	 * This constructor lets a user set all sigma parameters, as well as the adaption
	 * probability in one go.
	 *
	 * @param sigma The initial value for the sigma_ parameter
	 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	 * @param minSigma The minimal value allowed for sigma_
	 * @param maxSigma The maximal value allowed for sigma_
	 * @param adProb The adaption probability
	 */
	GIntGaussAdaptorT(
		const double& sigma
		, const double& sigmaSigma
		, const double& minSigma
		, const double& maxSigma
		, const double& adProb
	)
		: GNumGaussAdaptorT<int_type, double> (sigma, sigmaSigma, minSigma, maxSigma, adProb)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GIntGaussAdaptorT()
	{ /* nothing */ }

   /***************************************************************************/
   /**
    * The standard assignment operator
    */
   const GIntGaussAdaptorT<int_type>& operator=(const GIntGaussAdaptorT<int_type>& cp) {
      this->load_(&cp);
      return *this;
   }

   /***************************************************************************/
   /**
    * Checks for equality with another GIntGaussAdaptorT<int_type> object
    *
    * @param  cp A constant reference to another GIntGaussAdaptorT<int_type> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GIntGaussAdaptorT<int_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GIntGaussAdaptorT<int_type>::operator==","cp", CE_SILENT);
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GIntGaussAdaptorT<int_type> object
    *
    * @param  cp A constant reference to another GIntGaussAdaptorT<int_type> object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GIntGaussAdaptorT<int_type>& cp) const {
      using namespace Gem::Common;
      // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
      return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GIntGaussAdaptorT<int_type>::operator==","cp", CE_SILENT);
   }

	/***************************************************************************/
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
	virtual boost::optional<std::string> checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
	) const OVERRIDE {
	    using namespace Gem::Common;

	    // Check that we are not accidently assigning this object to itself
	    GObject::selfAssignmentCheck<GIntGaussAdaptorT<int_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GNumGaussAdaptorT<int_type, double>::checkRelationshipWith(cp, e, limit, "GIntGaussAdaptorT<int_type>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GIntGaussAdaptorT<int_type>", caller, deviations, e);
	}

	/***************************************************************************/
	/** @brief Retrieves the id of this adaptor */
	virtual Gem::Geneva::adaptorId getAdaptorId() const = 0;

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const OVERRIDE {
      return std::string("GIntGaussAdaptorT");
   }

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GObject
	 *
	 * @param cp A copy of another GIntGaussAdaptorT<int_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) OVERRIDE {
	    // Check that we are not accidently assigning this object to itself
	    GObject::selfAssignmentCheck<GIntGaussAdaptorT<int_type> >(cp);

		// Load our parent class'es data ...
		GNumGaussAdaptorT<int_type, double>::load_(cp);

		// ... no local data
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const = 0;

	/***************************************************************************/
	/**
	 * The actual adaption of the supplied value takes place here.
	 *
	 * @param value The value that is going to be adapted in situ
	 * @param range A typical range for the parameter with type num_type
	 */
	virtual void customAdaptions(
      int_type& value
      , const int_type& range
   ) OVERRIDE {
	   // Calculate a suitable addition to the current parameter value
		int_type addition = static_cast<int_type>(static_cast<double>(range) * this->gr->normal_distribution(this->getSigma()));

		if(addition == 0) { // Enforce a minimal change of 1.
			addition = this->gr->uniform_bool()?1:-1;
		}

      // adapt the value in situ. Note that this changes
      // the argument of this function
		value += addition;
	}

	/* ----------------------------------------------------------------------------------
	 * - Tested in GNumGaussAdaptorT<int_type, double>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent class'es function
		if(GNumGaussAdaptorT<int_type, double>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GIntGaussAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		// Call the parent class'es function
		GNumGaussAdaptorT<int_type, double>::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------
		// nothing yet
		//------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GIntGaussAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		// Call the parent class'es function
		GNumGaussAdaptorT<int_type, double>::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GIntGaussAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
};


} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename int_type>
		struct is_abstract<Gem::Geneva::GIntGaussAdaptorT<int_type> > : public boost::true_type {};
		template<typename int_type>
		struct is_abstract< const Gem::Geneva::GIntGaussAdaptorT<int_type> > : public boost::true_type {};
	}
}
/******************************************************************************/

#endif /* GINTGAUSSADAPTORT_HPP_ */
