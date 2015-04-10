/**
 * @file GAdaptorT.hpp
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

#ifndef GADAPTORT_HPP_
#define GADAPTORT_HPP_

// Geneva headers go here

#include "common/GTSSAccessT.hpp"
#include "common/GTriboolSerialization.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GObjectExpectationChecksT.hpp"
#include "geneva/GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * In Geneva, two mechanisms exist that let the user specify the
 * type of adaption he wants to have executed on collections of
 * items (basic types or any other types).  The most basic
 * possibility is for the user to overload the GOptimizableEntity::customAdaptions()
 * function and manually specify the types of adaptions (s)he
 * wants. This allows great flexibility, but is not very practicable
 * for standard adaptions.
 *
 * Classes derived from GParameterBaseWithAdaptorsT<T> can additionally store
 * "adaptors". These are templatized function objects that can act
 * on the items of a collection of user-defined types. Predefined
 * adaptors exist for standard types (with the most prominent
 * examples being bits and double values).
 *
 * The GAdaptorT class mostly acts as an interface for these
 * adaptors, but also implements some functionality of its own. E.g., it is possible
 * to specify a function that shall be called every adaptionThreshold_ calls of the
 * adapt() function. It is also possible to set a adaption probability, only a certain
 * percentage of adaptions is actually performed at run-time.
 *
 * In order to use this class, the user must derive a class from
 * GAdaptorT<T> and specify the type of adaption he wishes to
 * have applied to items, by overloading of
 * GAdaptorT<T>::customAdaptions(T&) .  T will often be
 * represented by a basic value (double, long, bool, ...). Where
 * this is not the case, the adaptor will only be able to access
 * public functions of T, unless T declares the adaptor as a friend.
 *
 * As a derivative of GObject, this class follows similar rules as
 * the other Geneva classes.
 */
template <typename T>
class GAdaptorT
	: public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
		& BOOST_SERIALIZATION_NVP(adaptionCounter_)
		& BOOST_SERIALIZATION_NVP(adaptionThreshold_)
		& BOOST_SERIALIZATION_NVP(adProb_)
		& BOOST_SERIALIZATION_NVP(adaptAdProb_)
		& BOOST_SERIALIZATION_NVP(minAdProb_)
		& BOOST_SERIALIZATION_NVP(maxAdProb_)
		& BOOST_SERIALIZATION_NVP(adaptionMode_)
		& BOOST_SERIALIZATION_NVP(adaptAdaptionProbability_)
		& BOOST_SERIALIZATION_NVP(adProb_reset_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * Allows external callers to find out about the type stored in this object
	 */
	typedef T adaption_type;

	/***************************************************************************/
	/**
	 * The default constructor.
	 */
	GAdaptorT()
		: GObject()
		, adaptionCounter_(0)
		, adaptionThreshold_(DEFAULTADAPTIONTHRESHOLD)
		, adProb_(DEFAULTADPROB)
	   , adaptAdProb_(DEFAUPTADAPTADPROB)
	   , minAdProb_(DEFMINADPROB)
	   , maxAdProb_(DEFMAXADPROB)
		, adaptionMode_(DEFAULTADAPTIONMODE)
		, adaptAdaptionProbability_(DEFAULTADAPTADAPTIONPROB)
	   , adProb_reset_(adProb_)
	{
	   // Check that adProb_ is in the allowed range. Adapt, if necessary
	   if(!Gem::Common::checkRangeCompliance<double>(adProb_, minAdProb_, maxAdProb_)) {
	      glogger
	      << "In GAdaptorT<T>::GadaptorT():" << std::endl
	      << "adProb_ value " << adProb_ << " is outside of allowed value range [" << minAdProb_ << ", " << maxAdProb_ << "]" << std::endl
	      << "The value will be adapted to fit this range." << std::endl
	      << GWARNING;

	      Gem::Common::enforceRangeConstraint<double>(adProb_, minAdProb_, maxAdProb_);
	      Gem::Common::enforceRangeConstraint<double>(adProb_reset_, minAdProb_, maxAdProb_);
	   }
	}

	/***************************************************************************/
	/**
	 * This constructor allows to set the probability with which an adaption is indeed
	 * performed.
	 *
	 * @param adProb The likelihood for a an adaption to be actually carried out
	 */
	GAdaptorT(const double& adProb)
		: GObject()
		, adaptionCounter_(0)
		, adaptionThreshold_(DEFAULTADAPTIONTHRESHOLD)
		, adProb_(adProb)
	   , adaptAdProb_(DEFAUPTADAPTADPROB)
      , minAdProb_(DEFMINADPROB)
      , maxAdProb_(DEFMAXADPROB)
		, adaptionMode_(DEFAULTADAPTIONMODE)
		, adaptAdaptionProbability_(DEFAULTADAPTADAPTIONPROB)
	   , adProb_reset_(adProb_)
	{
		// Do some error checking
      // Check that adProb_ is in the allowed range. Adapt, if necessary
      if(!Gem::Common::checkRangeCompliance<double>(adProb_, minAdProb_, maxAdProb_)) {
         glogger
         << "In GAdaptorT<T>::GadaptorT(const double& adProb):" << std::endl
         << "adProb value " << adProb_ << " is outside of allowed value range [" << minAdProb_ << ", " << maxAdProb_ << "]" << std::endl
         << "The value will be adapted to fit this range." << std::endl
         << GWARNING;

         Gem::Common::enforceRangeConstraint<double>(adProb_, minAdProb_, maxAdProb_);
         Gem::Common::enforceRangeConstraint<double>(adProb_reset_, minAdProb_, maxAdProb_);
      }
	}

	/***************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp A copy of another GAdaptorT<T>
	 */
	GAdaptorT(const GAdaptorT<T>& cp)
		: GObject(cp)
		, adaptionCounter_(cp.adaptionCounter_)
		, adaptionThreshold_(cp.adaptionThreshold_)
		, adProb_(cp.adProb_)
	   , adaptAdProb_(cp.adaptAdProb_)
      , minAdProb_(cp.minAdProb_)
      , maxAdProb_(cp.maxAdProb_)
		, adaptionMode_(cp.adaptionMode_)
		, adaptAdaptionProbability_(cp.adaptAdaptionProbability_)
	   , adProb_reset_(cp.adProb_reset_)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor. Gets rid of the local random number generator, unless
	 * an external generator has been assigned.
	 */
	virtual ~GAdaptorT() { /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard assignment operator
	 */
	const GAdaptorT<T>& operator=(const GAdaptorT<T>& cp) {
	   this->load_(&cp);
	   return *this;
	}

   /***************************************************************************/
   /**
    * Checks for equality with another GAdaptorT<T> object
    *
    * @param  cp A constant reference to another GAdaptorT<T> object
    * @return A boolean indicating whether both objects are equal
    */
   bool operator==(const GAdaptorT<T>& cp) const {
      using namespace Gem::Common;
      try {
         this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation&) {
         return false;
      }
   }

   /***************************************************************************/
   /**
    * Checks for inequality with another GAdaptorT<T> object
    *
    * @param  cp A constant reference to another GAdaptorT<T> object
    * @return A boolean indicating whether both objects are inequal
    */
   bool operator!=(const GAdaptorT<T>& cp) const {
      using namespace Gem::Common;
      try {
         this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation&) {
         return false;
      }
   }

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
	   const GObject& cp
	   , const Gem::Common::expectation& e
	   , const double& limit
	) const OVERRIDE {
	   using namespace Gem::Common;

      // Check that we are indeed dealing with a GAdaptorT reference
      const GAdaptorT<T>  *p_load = gobject_conversion<GAdaptorT<T> >(&cp);

      GToken token("GAdaptorT<T>", e);

      // Compare our parent data ...
      compare_base<GObject>(IDENTITY(*this, *p_load), token);

      // ... and then the local data
      compare_t(IDENTITY(adaptionCounter_, p_load->adaptionCounter_), token);
      compare_t(IDENTITY(adaptionThreshold_, p_load->adaptionThreshold_), token);
      compare_t(IDENTITY(adProb_, p_load->adProb_), token);
      compare_t(IDENTITY(adaptAdProb_, p_load->adaptAdProb_), token);
      compare_t(IDENTITY(minAdProb_, p_load->minAdProb_), token);
      compare_t(IDENTITY(maxAdProb_, p_load->maxAdProb_), token);
      compare_t(IDENTITY(adaptionMode_, p_load->adaptionMode_), token);
      compare_t(IDENTITY(adaptAdaptionProbability_, p_load->adaptAdaptionProbability_), token);
      compare_t(IDENTITY(adProb_reset_, p_load->adProb_reset_), token);

      // React on deviations from the expectation
      token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Retrieves the id of the adaptor. Purely virtual, must be implemented by the
	 * actual adaptors.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::Geneva::adaptorId getAdaptorId() const = 0;

	/* ----------------------------------------------------------------------------------
	 * Tested in GBooleanAdaptor
	 * Tested in GInt32FlipAdaptor
	 * Tested in GInt32GaussAdaptor
	 * Tested in GDoubleGaussAdaptor
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Sets the adaption probability to a given value. This function will throw
	 * if the probability is not in the allowed range.
	 *
	 * @param adProb The new value of the probability of adaptions taking place
	 */
	void setAdaptionProbability(const double& adProb) {
		// Check the supplied probability value
		if(adProb < 0. || adProb > 1.) {
			glogger
			<< "In GAdaptorT<T>::setAdaptionProbability(const double&):" << std::endl
			<< "Bad probability value given: " << adProb << std::endl
			<< GEXCEPTION;
		}

		// Check that the new value fits in the allowed value range
      if(!Gem::Common::checkRangeCompliance<double>(adProb, minAdProb_, maxAdProb_)) {
         glogger
         << "In GAdaptorT<T>::setAdaptionProbability(const double& adProb):" << std::endl
         << "adProb value " << adProb << " is outside of allowed value range [" << minAdProb_ << ", " << maxAdProb_ << "]" << std::endl
         << "Set new boundaries first before setting a new \"adProb\" value" << std::endl
         << GEXCEPTION;
      }

		adProb_ = adProb;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of valid probabilities is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * Checks for setting of invalid probabilities is tested in GAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	 * The effects on the probability of adaptions actually taking place are tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Retrieves the current value of the adaption probability
	 *
	 * @return The current value of the adaption probability
	 */
	double getAdaptionProbability() const {
		return adProb_;
	}

	/* ----------------------------------------------------------------------------------
	 * Retrieval of probabilities is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

   /***************************************************************************/
   /**
    * Sets the "reset" adaption probability to a given value. This is the probability
    * to which adProb_ will be reset if updateOnStall() is called. This function will
    * throw if the probability is not in the allowed range.
    *
    * @param adProb_reset The new value of the "reset" probability
    */
	void setResetAdaptionProbability(const double& adProb_reset) {
      // Check the supplied probability value
      if(!Gem::Common::checkRangeCompliance<double>(adProb_reset, minAdProb_, maxAdProb_)) {
         glogger
         << "In GAdaptorT<T>::setResetAdaptionProbability(const double&):" << std::endl
         << "adProb_reset value " << adProb_reset << " is outside of allowed value range [" << minAdProb_ << ", " << maxAdProb_ << "]" << std::endl
         << "Set new boundaries first before setting a new \"adProb_reset\" value" << std::endl
         << GEXCEPTION;
      }

      adProb_reset_ = adProb_reset;
   }

   /***************************************************************************/
   /**
    * Retrieves the current value of the "reset" adaption probability
    *
    * @return The current value of the "reset" adaption probability
    */
	double getResetAdaptionProbability() const {
      return adProb_reset_;
   }

	/***************************************************************************/
	/**
	 * Sets the probability for the adaption of adaption parameters
	 *
	 * @param probability The new value of the probability of adaptions of adaption parameters
	 */
	void setAdaptAdaptionProbability(const double& probability) {
		// Check the supplied probability value
		if(!Gem::Common::checkRangeCompliance<double>(probability, 0., 1.)) {
			glogger
			<<	"In GAdaptorT<T>::setAdaptAdaptionProbability(const double&) :" << std::endl
			<< "Probability " << probability << " not in allowed range [0.,1.]" << std::endl
			<< GEXCEPTION;
		}

		adaptAdaptionProbability_ = probability;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of valid probabilities is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * Checks for setting of invalid probabilities is tested in GAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Retrieves the current value of the adaptAdaptionProbability_ variable
	 *
	 * @return The current value of the adaptAdaptionProbability_ variable
	 */
	double getAdaptAdaptionProbability() const {
		return adaptAdaptionProbability_;
	}

	/* ----------------------------------------------------------------------------------
	 * Retrieval of probabilities is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Allows to specify an adaption factor for adProb_ (or 0, if you do not
	 * want this feature)
	 */
	void setAdaptAdProb(double adaptAdProb) {
#ifdef DEBUG
	   if(adaptAdProb < 0.) {
	      glogger
	      << "In GAdaptorT<>::setAdaptAdProb(): Error!" << std::endl
	      << "adaptAdProb < 0: " << adaptAdProb << std::endl
	      << GEXCEPTION;
	   }
#endif /* DEBUG */

	   adaptAdProb_ = adaptAdProb;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the rate of evolutionary adaption of adProb_
	 */
	double getAdaptAdProb() const {
	   return adaptAdProb_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current value of the adaptionCounter_ variable.
	 *
	 * @return The value of the adaptionCounter_ variable
	 */
	virtual boost::uint32_t getAdaptionCounter() const  {
		return adaptionCounter_;
	}

	/* ----------------------------------------------------------------------------------
	 * It is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests() that the
	 * adaption counter does not exceed the set adaption threshold
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Sets the value of adaptionThreshold_. If set to 0, no adaption of the optimization
	 * parameters will take place
	 *
	 * @param adaptionCounter The value that should be assigned to the adaptionCounter_ variable
	 */
	void setAdaptionThreshold(const boost::uint32_t& adaptionThreshold)  {
		adaptionThreshold_ = adaptionThreshold;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of adaption thresholds is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Retrieves the value of the adaptionThreshold_ variable.
	 *
	 * @return The value of the adaptionThreshold_ variable
	 */
	boost::uint32_t getAdaptionThreshold() const  {
		return adaptionThreshold_;
	}

	/* ----------------------------------------------------------------------------------
	 * Retrieval of adaption threshold is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Allows to specify whether adaptions should happen always, never, or with a given
	 * probability. This uses the boost::logic::tribool class. The function is declared
	 * virtual so adaptors requiring adaptions to happen always or never can prevent
	 * resetting of the adaptionMode_ variable.
	 *
	 * @param adaptionMode The desired mode (always/never/with a given probability)
	 */
	virtual void setAdaptionMode(boost::logic::tribool adaptionMode) {
		adaptionMode_ = adaptionMode;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of the adaption mode is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * The effect of setting the adaption mode is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Returns the current value of the adaptionMode_ variable
	 *
	 * @return The current value of the adaptionMode_ variable
	 */
	boost::logic::tribool getAdaptionMode() const {
		return adaptionMode_;
	}

	/* ----------------------------------------------------------------------------------
	 * Retrieval of the adaption mode is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Allows to set the allowed range for adaption probability variation.
	 * NOTE that this function will silently adapt the values of adProb_ and
	 * adProb_reset_, if they fall outside of the new range.
	 */
	void setAdProbRange(double minAdProb, double maxAdProb) {
#ifdef DEBUG
	   if(minAdProb < 0.) {
	      glogger
	      << "In GAdaptorT<T>::setAdProbRange(): Error!" << std::endl
	      << "minAdProb < 0: " << minAdProb << std::endl
	      << GEXCEPTION;
	   }

      if(maxAdProb > 1.) {
         glogger
         << "In GAdaptorT<T>::setAdProbRange(): Error!" << std::endl
         << "maxAdProb > 1: " << maxAdProb << std::endl
         << GEXCEPTION;
      }

	   if(minAdProb > maxAdProb) {
         glogger
         << "In GAdaptorT<T>::setAdProbRange(): Error!" << std::endl
         << "Invalid minAdProb and/or maxAdProb: " << minAdProb << " / " << maxAdProb << std::endl
         << GEXCEPTION;
	   }
#endif /* DEBUG */

	   // Store the new values
	   minAdProb_ = minAdProb; if(minAdProb_ < DEFMINADPROB) minAdProb_ = DEFMINADPROB;
	   maxAdProb_ = maxAdProb;

	   // Make sure adProb_ and adProb_reset_ fit the new allowed range
	   Gem::Common::enforceRangeConstraint<double>(adProb_, minAdProb_, maxAdProb_);
	   Gem::Common::enforceRangeConstraint<double>(adProb_reset_, minAdProb_, maxAdProb_);
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the allowed range for adProb_ variation
	 */
	boost::tuple<double,double> getAdProbRange() const {
	   return boost::tuple<double,double>(minAdProb_, maxAdProb_);
	}

	/***************************************************************************/
	/**
	 * Common interface for all adaptors to the adaption functionality. The user
	 * specifies the actual actions in the customAdaptions() function.
	 *
	 * @param val The value that needs to be adapted
	 * @param range A typical value range for type T
	 * @return The number of adaptions that were carried out
	 */
	std::size_t adapt(
      T& val
      , const T& range
   ) {
	   using namespace Gem::Common;
	   using namespace Gem::Hap;

	   bool adapted = false;

	   // Update the adaption probability, if requested by the user
	   if(adaptAdProb_ > 0) {
	      adProb_ *= gexp(GObject::gr_ptr()->normal_distribution(adaptAdProb_));
	      Gem::Common::enforceRangeConstraint<double>(adProb_, minAdProb_, maxAdProb_);
	   }

		if(boost::logic::indeterminate(adaptionMode_)) { // The most likely case is indeterminate (means: "depends")
			if(GObject::gr_ptr()->GRandomBase::uniform_01<double>() <= adProb_) { // Should we perform adaption
			   adaptAdaption(range);
				customAdaptions(val, range);
				adapted = true;
			}
		} else if(true == adaptionMode_) { // always adapt
		   adaptAdaption(range);
			customAdaptions(val, range);
			adapted = true;
		}

		// No need to test for "adaptionMode_ == false" as no action is needed in this case

		if(adapted) {
		   return std::size_t(1);
		} else {
		   return std::size_t(0);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * Adaption is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

   /***************************************************************************/
   /**
    * Common interface for all adaptors to the adaption functionality. The user
    * specifies the actual actions in the customAdaptions() function. This function
    * deals with entire parameter vectors. The philosophy behind these vectors is
    * that they represent a common logical entity and should thus be mutated together,
    * using a single adaptor. However, it is not clear whether adaptions of mutation
    * parameters (such as adaption of the sigma value) should happen whenever
    * customAdaptions() is called (which would be equivalent to individual parameter
    * objects) or only once, before customAdaptions is applied to each position in
    * turn. As adaption e.g. of the sigma value slightly favors changes towards smaller
    * values, we incur a small bias in the first case, where mutations of parameters
    * at the end of the array might be smaller than at the beginning. In the second case,
    * metaAdaption might not be called often enough to adapt the mutation process
    * to different geometries of the quality surface. Our tests show that the latter
    * might be more severe, so we have implemented repeated adaption of mutation parameters
    * in this function.
    *
    * @param valVec A vector of values that need to be adapted
    * @param range A typical value range for type T
    * @return The number of adaptions that were carried out
    */
	std::size_t adapt(
      std::vector<T>& valVec
      , const T& range
   ) {
      using namespace Gem::Common;
      using namespace Gem::Hap;

      std::size_t nAdapted = 0;

      typename std::vector<T>::iterator it;

      // Update the adaption probability, if requested by the user
      if(adaptAdProb_ > 0) {
         adProb_ *= gexp(GObject::gr_ptr()->normal_distribution(adaptAdProb_));
         Gem::Common::enforceRangeConstraint<double>(adProb_, minAdProb_, maxAdProb_);
      }

      if(boost::logic::indeterminate(adaptionMode_)) { // The most likely case is indeterminate (means: "depends")
         for (it = valVec.begin(); it != valVec.end(); ++it) {
            if(GObject::gr_ptr()->GRandomBase::uniform_01<double>() <= adProb_) { // Should we perform adaption ?
               adaptAdaption(range);
               customAdaptions(*it, range);

               nAdapted += 1;
            }
         }
      } else if(true == adaptionMode_) { // always adapt
         for (it = valVec.begin(); it != valVec.end(); ++it) {
            adaptAdaption(range);
            customAdaptions(*it, range);

            nAdapted += 1;
         }
      }

      // No need to test for "adaptionMode_ == false" as no action is needed in this case

      return nAdapted;
   }

   /* ----------------------------------------------------------------------------------
    * Adaption is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
    * ----------------------------------------------------------------------------------
    */

   /***************************************************************************/
	/**
	 * Triggers updates when the optimization process has stalled. This function
	 * resets the adaption probability to its original value
	 *
	 * @param nStalls The number of consecutive stalls up to this point
	 * @param range A typical value range for type T
	 * @return A boolean indicating whether updates were performed
	 */
	virtual bool updateOnStall(
      const std::size_t& nStalls
      , const T& range
   ) BASE {
#ifdef DEBUG
	   if(0 == nStalls) {
	      glogger
	      << "In GAdaptorT<>::updateOnStall(" << nStalls << "): Error!" << std::endl
	      << "Function called for zero nStalls" << std::endl
	      << GEXCEPTION;
	   }
#endif

	   // Reset the adaption probability
	   if(adProb_ == adProb_reset_) {
	      return false;
	   } else {
	      adProb_ = adProb_reset_;
	      return true;
	   }
	}

	/***************************************************************************/
	/**
	 * Allows derived classes to print diagnostic messages
	 *
	 * @return A diagnostic message
	 */
	virtual std::string printDiagnostics() const {
		return std::string();
	}

   /***************************************************************************/
   /**
    * Emits a name for this class / object
    */
   virtual std::string name() const OVERRIDE {
      return std::string("GAdaptorT");
   }

   /***************************************************************************/
   /**
    * Allows to query specific properties of a given adaptor. Note that the
    * adaptor must have implemented a "response" for the query, as the function
    * will otherwise throw. This function is meant for debugging and profiling.
    * It might e.g. be useful if you want to know why an EA-based optimization has
    * stalled. Note that the permanent use of this function, e.g. from a permanently
    * enabled "pluggable optimization monitor, will be inefficient due to the
    * constant need to compare strings.
    *
    * @param adaoptorName The name of the adaptor to be queried
    * @param property The property for which information is sought
    * @param data A vector, to which the properties should be added
    */
   void queryPropertyFrom(
      const std::string& adaptorName
      , const std::string& property
      , std::vector<boost::any>& data
   ) const BASE {
      // Do nothing, if this query is not for us
      if(adaptorName != this->name()) {
         return;
      } else { // O.k., this query is for us!
         if(property == "adProb") { // The only property that can be queried for this class
            data.push_back(boost::any(adProb_));
         } else { // Ask derived classes
            if(!this->customQueryProperty(property, data)) {
               glogger
               << "In GAdaptorT<T>::queryPropertyFrom(): Error!" << std::endl
               << "Function was called for unimplemented property " << property << std::endl
               << "on adaptor " << adaptorName << std::endl
               << GEXCEPTION;
            }
         }
      }
   }

   /***************************************************************************/
   /** @brief Allows derived classes to randomly initialize parameter members */
   virtual bool randomInit() BASE = 0;

protected:
	/***************************************************************************/
	/**
	 * Loads the contents of another GAdaptorT<T>. The function
	 * is similar to a copy constructor (but with a pointer as
	 * argument). As this function might be called in an environment
	 * where we do not know the exact type of the class, the
	 * GAdaptorT<T> is camouflaged as a GObject . This implies the
	 * need for dynamic conversion.
	 *
	 * @param gb A pointer to another GAdaptorT<T>, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) OVERRIDE {
		// Convert cp into local format
		const GAdaptorT<T> *p_load = gobject_conversion<GAdaptorT<T> >(cp);

		// Load the parent class'es data
		GObject::load_(cp);

		// Then our own data
		adaptionCounter_ = p_load->adaptionCounter_;
		adaptionThreshold_ = p_load->adaptionThreshold_;
		adProb_ = p_load->adProb_;
		adaptAdProb_ = p_load->adaptAdProb_;
		minAdProb_ = p_load->minAdProb_;
		maxAdProb_ = p_load->maxAdProb_;
		adaptionMode_ = p_load->adaptionMode_;
		adaptAdaptionProbability_ = p_load->adaptAdaptionProbability_;
		adProb_reset_ = p_load->adProb_reset_;
	}

   /***************************************************************************/
   /**
    * This function helps to adapt the adaption parameters, if certain conditions are met.
    * Adaption is triggered by the parameter object.
    *
    *  @param range A typical range for the parameter with type T
    */
	void adaptAdaption(const T& range) {
	   using namespace Gem::Common;
	   using namespace Gem::Hap;

      // The adaption parameters are modified every adaptionThreshold_ number of adaptions.
      if(adaptionThreshold_ > 0) {
         if(++adaptionCounter_ >= adaptionThreshold_){
            adaptionCounter_ = 0;
            customAdaptAdaption(range);
         }
      } else if(adaptAdaptionProbability_) { // Do the same with probability settings
         if(GObject::gr_ptr()->GRandomBase::uniform_01<double>() <= adaptAdaptionProbability_) {
            customAdaptAdaption(range);
         }
      }
   }

   /***************************************************************************/
   /**
    * Adds a given property value to the vector or returns false, if the property
    * was not found. We do not check anymore if this query was for as, as this was
    * already done by  queryPropertyFrom(). Thus function needs to be re-implemented
    * by derived classes wishing to emit information. If there is no re-implementation,
    * this function will simply return false.
    */
   virtual bool customQueryProperty (
      const std::string& property
      , std::vector<boost::any>& data
   ) const {
      return false;
   }

   /***************************************************************************/
   /**
    *  This function is re-implemented by derived classes, if they wish to
    *  implement special behavior for a new adaption run. E.g., an internal
    *  variable could be set to a new value.
    *
    *  @param range A typical range for the parameter with type T
    */
   virtual void customAdaptAdaption(const T&) { /* nothing */ }

	/***************************************************************************/

	/** @brief Adaption of values as specified by the user */
	virtual void customAdaptions(T&, const T&) BASE = 0;

   /** @brief Creates a deep copy of this object */
   virtual GObject *clone_(void) const = 0;

private:
	/***************************************************************************/
	boost::uint32_t adaptionCounter_; ///< A local counter
	boost::uint32_t adaptionThreshold_; ///< Specifies after how many adaptions the adaption itself should be adapted
	double adProb_; ///< internal representation of the adaption probability
   double adaptAdProb_; ///< The rate, at which adProb_ should be adapted
   double minAdProb_; ///< The lower allowed value for adProb_ during variation
   double maxAdProb_; ///< The upper allowed value for adProb_ during variation
	boost::logic::tribool adaptionMode_; ///< false == never adapt; indeterminate == adapt with adProb_ probability; true == always adapt
	double adaptAdaptionProbability_; ///< Influences the likelihood for the adaption of the adaption parameters

	double adProb_reset_; ///< The value to which adProb_ will be reset if "updateOnStall()" is called

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GObject::modify_GUnitTests()) result = true;

		// Modify some local parameters
		if(this->getAdaptionProbability() <= 0.5) {
			this->setAdaptionProbability(0.75);
		}
		else {
			this->setAdaptionProbability(0.25);
		}

		result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAdaptorT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GObject::specificTestsNoFailureExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Test of GAdaptorT<T>::set/getAdaptionProbability()
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// The adaption probability should have been cloned
			BOOST_CHECK_MESSAGE(
					p_test->getAdaptionProbability() == this->getAdaptionProbability()
					,  "\n"
					<< "p_test->getAdaptionProbability() = " << p_test->getAdaptionProbability() << "\n"
					<< "this->getAdaptionProbability() = " << this->getAdaptionProbability() << "\n"
			);

			// Set an appropriate range for the adaption
			p_test->setAdProbRange(0.001, 1.);

			// Set the adaption probability to a sensible value and check the new setting
			double testAdProb = 0.5;
			BOOST_CHECK_NO_THROW(
					p_test->setAdaptionProbability(testAdProb);
			);
			BOOST_CHECK_MESSAGE(
					p_test->getAdaptionProbability() == testAdProb
					,  "\n"
					<< "p_test->getAdaptionProbability() = " << p_test->getAdaptionProbability() << "\n"
					<< "testAdProb = " << testAdProb << "\n"
			);
		}

		//------------------------------------------------------------------------------

		{ // Check that mutating a value with this class actually work with different likelihoods
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Make sure the adaption probability is taken into account
			p_test->setAdaptionMode(boost::logic::indeterminate);
			// Set an appropriate range for the adaption
			p_test->setAdProbRange(0.001, 1.);

			T testVal = T(0);
			for(double prob=0.001; prob<1.; prob+=0.01) {
				// Account for rounding problems
				if(prob > 1.) prob = 1.;

				p_test->setAdaptionProbability(prob);
				BOOST_CHECK_NO_THROW(p_test->setAdaptionProbability(prob));
				BOOST_CHECK_NO_THROW(p_test->adapt(testVal, T(1)));
			}
		}

		//------------------------------------------------------------------------------

		{ // Test of GAdaptorT<T>::setAdaptionProbability() regarding the effects on the likelihood for adaption of the variable
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Make sure the adaption probability is taken into account
			p_test->setAdaptionMode(boost::logic::indeterminate);
			// Prevent changes to adProb_
			p_test->setAdaptAdProb(0.);

         p_test->setAdProbRange(0., 1.);

			const std::size_t nTests=100000;

			for(double prob=0.1; prob<1.; prob+=0.1) {
				// Account for rounding problems
				if(prob > 1.) prob = 1.;

				std::size_t nChanged=0;

				T testVal = T(0);
				T prevTestVal = testVal;

				// Set the likelihood for adaption to "prob"
				p_test->setAdaptionProbability(prob);

				// Mutating a boolean value a number of times should now result in a certain number of changed values
				for(std::size_t i=0; i<nTests; i++) {
					p_test->adapt(testVal, T(1));
					if(testVal != prevTestVal) {
						nChanged++;
						prevTestVal=testVal;
					}
				}

				double changeProb = double(nChanged)/double(nTests);

            BOOST_CHECK_MESSAGE(
                  changeProb>0.8*prob && changeProb<1.2*prob
                  ,  "\n"
                  << "changeProb = " << changeProb << "\n"
                  << "prob = " << prob << "\n"
                  << "with allowed window = [" << 0.8*prob << " : " << 1.2*prob << "]" << "\n"
            );
			}
		}

		//------------------------------------------------------------------------------

		{ // Check setting and retrieval of the adaption mode
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Check setting of the different allowed values
			// false
			BOOST_CHECK_NO_THROW (p_test->setAdaptionMode(false));
			BOOST_CHECK_MESSAGE (
					!(p_test->getAdaptionMode())
					,  "\n"
					<< "p_test->getAdaptionMode() = " << p_test->getAdaptionMode() << "\n"
					<< "required value            = false\n"
			);

			// true
			BOOST_CHECK_NO_THROW (p_test->setAdaptionMode(true));
			BOOST_CHECK_MESSAGE (
					p_test->getAdaptionMode()
					,  "\n"
					<< "p_test->getAdaptionMode() = " << p_test->getAdaptionMode() << "\n"
					<< "required value            = true\n"
			);

			// boost::logic::indeterminate
			BOOST_CHECK_NO_THROW (p_test->setAdaptionMode(boost::logic::indeterminate));
			BOOST_CHECK_MESSAGE (
					boost::logic::indeterminate(p_test->getAdaptionMode())
					,  "\n"
					<< "p_test->getAdaptionMode() = " << p_test->getAdaptionMode() << "\n"
					<< "required value            = boost::logic::indeterminate\n"
			);
		}

		//------------------------------------------------------------------------------

		{ // Check the effect of the adaption mode settings
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();
			p_test->setAdaptionProbability(0.5);

			const std::size_t nTests = 10000;

			// false: There should never be adaptions, independent of the adaption probability
			BOOST_CHECK_NO_THROW (p_test->setAdaptionMode(false));
			T currentValue = T(0);
			T oldValue = currentValue;
			for(std::size_t i=0; i<nTests; i++) {
				p_test->adapt(currentValue, T(1));
				BOOST_CHECK_MESSAGE (
						currentValue == oldValue
						,  "\n"
						<< "Values differ, when they shouldn't:"
						<< "currentValue = " << currentValue << "\n"
						<< "oldValue     = " << oldValue << "\n"
						<< "iteration    = " << i << "\n"
				);
			}

			// true: Adaptions should happen always, independent of the adaption probability
			BOOST_CHECK_NO_THROW (p_test->setAdaptionMode(true));
			currentValue = T(0);
			oldValue = currentValue;
			for(std::size_t i=0; i<nTests; i++) {
				p_test->adapt(currentValue, T(1));
				BOOST_CHECK_MESSAGE (
						currentValue != oldValue
						,  "\n"
						<< "Values are identical when they shouldn't be:" << "\n"
						<< "currentValue = " << currentValue << "\n"
						<< "oldValue     = " << oldValue << "\n"
						<< "iteration    = " << i << "\n"
						<< (this->printDiagnostics()).c_str()
				);
				oldValue = currentValue;
			}

			// boost::logic::indeterminate: Adaptions should happen with a certain adaption probability
			// No tests -- we already know that this works
		}

		//------------------------------------------------------------------------------

		{ // Test of GAdaptorT<T>::set/getAdaptAdaptionProbability()
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// The adaption probability should have been cloned
			BOOST_CHECK_MESSAGE(
					p_test->getAdaptAdaptionProbability() == this->getAdaptAdaptionProbability()
					,  "\n"
					<< "p_test->getAdaptAdaptionProbability() = " << p_test->getAdaptAdaptionProbability() << "\n"
					<< "this->getAdaptAdaptionProbability() = " << this->getAdaptAdaptionProbability() << "\n"
			);

			// Set the adaption probability to a sensible value and check the new setting
			double testAdProb = 0.5;
			BOOST_CHECK_NO_THROW(
					p_test->setAdaptAdaptionProbability(testAdProb);
			);
			BOOST_CHECK_MESSAGE(
					p_test->getAdaptAdaptionProbability() == testAdProb
					,  "\n"
					<< "p_test->getAdaptAdaptionProbability() = " << p_test->getAdaptAdaptionProbability() << "\n"
					<< "testAdProb = " << testAdProb << "\n"
			);
		}

		//------------------------------------------------------------------------------

		{ // Test retrieval and setting of the adaption threshold and whether the adaptionCounter behaves nicely
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Make sure we have the right adaption mode
			p_test->setAdaptionMode(boost::logic::indeterminate);
			// Make sure we always adapt
			p_test->setAdaptionProbability(1.0);

			// The value that will be adapted
			T testVal = T(0);
			T oldTestVal = T(0);

			// The old adaption counter
			boost::uint32_t oldAdaptionCounter=p_test->getAdaptionCounter();

			// Set the adaption threshold to a specific value
			for(boost::uint32_t adThr=10; adThr>0; adThr--) {
				// Just make sure our logic is right and we stay in the right window
				BOOST_CHECK(adThr<=10);

				BOOST_CHECK_NO_THROW(p_test->setAdaptionThreshold(adThr));
				BOOST_CHECK_MESSAGE(
						p_test->getAdaptionThreshold() == adThr
						,  "\n"
						<< "p_test->getAdaptionThreshold() = " << p_test->getAdaptionThreshold() << "\n"
						<< "adThr = " << adThr << "\n"
				);

				// Check that the adaption counter does not exceed the threshold by
				// adapting a value a number of times > adThr
				for(boost::uint32_t adCnt=0; adCnt<3*adThr; adCnt++) {
					// Do the actual adaption
				   if(p_test->adapt(testVal, T(1))) {
                  // Check that testVal has indeed been adapted
                  BOOST_CHECK_MESSAGE(
                        testVal != oldTestVal
                        ,  "\n"
                        << "testVal = " << testVal << "\n"
                        << "oldTestVal = " << oldTestVal << "\n"
                        << "adThr = " << adThr << "\n"
                        << "adCnt = " << adCnt << "\n"
                  );
                  oldTestVal = testVal;

                  // Check that the adaption counter has changed at all, as it should
                  // for adaption thresholds > 1
                  if(adThr > 1) {
                     BOOST_CHECK_MESSAGE(
                           p_test->getAdaptionCounter() != oldAdaptionCounter
                           ,  "\n"
                           << "p_test->getAdaptionCounter() = " << p_test->getAdaptionCounter() << "\n"
                           << "oldAdaptionCounter = " << oldAdaptionCounter << "\n"
                           << "adThr = " << adThr << "\n"
                           << "adCnt = " << adCnt << "\n"
                     );
                     oldAdaptionCounter = p_test->getAdaptionCounter();
                  }

                  // Check that the adaption counter is behaving nicely
                  BOOST_CHECK_MESSAGE(
                        p_test->getAdaptionCounter() < adThr
                        , "\n"
                        << "p_test->getAdaptionCounter() = " << p_test->getAdaptionCounter() << "\n"
                        << "adThr = " << adThr << "\n"
                        << "adCnt = " << adCnt << "\n"
                  );
				   }
				}
			}
		}

		//------------------------------------------------------------------------------

		{ // Test that customAdaptions() in derived classes changes a test value on every call
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			std::size_t nTests = 10000;

			T testVal = T(0);
			T oldTestVal = T(0);
			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(p_test->customAdaptions(testVal, T(1)));
				BOOST_CHECK_MESSAGE(
               testVal != oldTestVal
               ,  "\n"
               << "Found identical values after adaption took place" << "\n"
               << "testVal = " << testVal << "\n"
               << "oldTestVal = " << oldTestVal << "\n"
               << "iteration = " << i << "\n"
				);
				oldTestVal = testVal;
			}
		}

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GAdaptorT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes.
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GObject::specificTestsFailuresExpected_GUnitTests();

		//------------------------------------------------------------------------------

		{ // Test of GAdaptorT<T>::setAdaptionProbability(): Setting a value < 0. should throw
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Setting a probability < 0 should throw
			BOOST_CHECK_THROW(
					p_test->setAdaptionProbability(-1.);
					, Gem::Common::gemfony_error_condition
			);
		}

		//------------------------------------------------------------------------------

		{ // Test of GAdaptorT<T>::setAdaptionProbability(): Setting a value > 1. should throw
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Setting a probability > 1 should throw
			BOOST_CHECK_THROW(
					p_test->setAdaptionProbability(2.);
					, Gem::Common::gemfony_error_condition
			);
		}

		//------------------------------------------------------------------------------

		{ // Test of GAdaptorT<T>::setAdaptAdaptionProbability(): Setting a value < 0. should throw
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Setting a probability < 0 should throw
			BOOST_CHECK_THROW(
					p_test->setAdaptAdaptionProbability(-1.);
					, Gem::Common::gemfony_error_condition
			);
		}

		//------------------------------------------------------------------------------

		{ // Test of GAdaptorT<T>::setAdaptAdaptionProbability(): Setting a value > 1. should throw
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Setting a probability > 1 should throw
			BOOST_CHECK_THROW(
					p_test->setAdaptAdaptionProbability(2.);
					, Gem::Common::gemfony_error_condition
			);
		}

		//------------------------------------------------------------------------------

#ifdef DEBUG
		{ // Check that assigning a NULL pointer for the random number generator throws in DEBUG mode
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Assigning a NULL pointer should throw
			BOOST_CHECK_THROW(
					p_test->assignGRandomPointer(NULL);
					, Gem::Common::gemfony_error_condition
			);
		}
#endif /* DEBUG */

		//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAdaptorT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/** @brief Mark this class as abstract. This is the content of
 * BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) */

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::Geneva::GAdaptorT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::Geneva::GAdaptorT<T> > : public boost::true_type {};
	}
}

/******************************************************************************/

#endif /* GADAPTORT_HPP_ */
