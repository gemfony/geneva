/**
 * @file GAdaptorT.hpp
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

// Standard headers go here

// Boost headers go here

#ifndef GADAPTORT_HPP_
#define GADAPTORT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here

#include "common/GTriboolSerialization.hpp"
#include "hap/GRandomT.hpp"
#include "GObject.hpp"
#include "GObjectExpectationChecksT.hpp"
#include "GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************************/
/**
 * In Geneva, two mechanisms exist that let the user specify the
 * type of adaption he wants to have executed on collections of
 * items (basic types or any other types).  The most basic
 * possibility is for the user to overload the GIndividual::customAdaptions()
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
class GAdaptorT:
	public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
		   & BOOST_SERIALIZATION_NVP(adaptionCounter_)
		   & BOOST_SERIALIZATION_NVP(adaptionThreshold_)
		   & BOOST_SERIALIZATION_NVP(adProb_)
		   & BOOST_SERIALIZATION_NVP(adaptionMode_)
		   & BOOST_SERIALIZATION_NVP(adaptAdaptionProbability_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***********************************************************************************/
	/**
	 * Allows external callers to find out about the type stored in this object
	 */
	typedef T adaption_type;

	/***********************************************************************************/
	/**
	 * The default constructor.
	 */
	GAdaptorT()
		: GObject()
		, gr_local(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>())
		, gr(gr_local)
		, adaptionCounter_(0)
		, adaptionThreshold_(DEFAULTADAPTIONTHRESHOLD)
		, adProb_(DEFAULTADPROB)
		, adaptionMode_(DEFAULTADAPTIONMODE)
		, adaptAdaptionProbability_(DEFAULTADAPTADAPTIONPROB)
	{ /* nothing */ }

	/***********************************************************************************/
	/**
	 * This constructor allows to set the probability with which an adaption is indeed
	 * performed.
	 *
	 * @param adProb The likelihood for a an adaption to be actually carried out
	 */
	GAdaptorT(const double& adProb)
		: GObject()
		, gr_local(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>())
		, gr(gr_local)
		, adaptionCounter_(0)
		, adaptionThreshold_(DEFAULTADAPTIONTHRESHOLD)
		, adProb_(adProb)
		, adaptionMode_(DEFAULTADAPTIONMODE)
		, adaptAdaptionProbability_(DEFAULTADAPTADAPTIONPROB)
	{
		// Do some error checking
		if(adProb < 0. || adProb > 1.) {
			std::cerr << "In GAdaptorT<T>::GadaptorT(const double& prob):" << std::endl
					  << "Provided adaption probability is invalid: " << adProb << std::endl
					  << "Setting to the maximum allowed value 1." << std::endl;
			adProb_ = 1.;
		}
	}

	/***********************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp A copy of another GAdaptorT<T>
	 */
	GAdaptorT(const GAdaptorT<T>& cp)
		: GObject(cp)
		, gr_local(new Gem::Hap::GRandomT<Gem::Hap::RANDOMLOCAL>()) // We do *not* copy the other object's generator
		, gr(gr_local)
		, adaptionCounter_(cp.adaptionCounter_)
		, adaptionThreshold_(cp.adaptionThreshold_)
		, adProb_(cp.adProb_)
		, adaptionMode_(cp.adaptionMode_)
		, adaptAdaptionProbability_(cp.adaptAdaptionProbability_)
	{ /* nothing */ }

	/***********************************************************************************/
	/**
	 * The standard destructor. Gets rid of the local random number generator, unless
	 * an external generator has been assigned.
	 */
	virtual ~GAdaptorT() {
		if(gr_local) delete gr_local;
	}

	/***********************************************************************************/
	/**
	 * A standard assignment operator for GAdaptorT<T> objects,
	 *
	 * @param cp A copy of another GAdaptorT<T> object
	 */
	const GAdaptorT<T>& operator=(const GAdaptorT<T>& cp) {
		GAdaptorT<T>::load_(&cp);
		return *this;
	}

	/***********************************************************************************/
	/**
	 * Checks for equality with another GAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GAdaptorT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GAdaptorT<T>::operator==","cp", CE_SILENT);
	}

	/***********************************************************************************/
	/**
	 * Checks for inequality with another GAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GAdaptorT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GAdaptorT<T>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GAdaptorT<T>::operator!=","cp", CE_SILENT);
	}

	/***********************************************************************************/
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
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GAdaptorT<T>  *p_load = gobject_conversion<GAdaptorT<T> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, "GAdaptorT<T>", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", adaptionCounter_, p_load->adaptionCounter_, "adaptionCounter_", "p_load->adaptionCounter_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", adaptionThreshold_, p_load->adaptionThreshold_, "adaptionThreshold_", "p_load->adaptionThreshold_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", adProb_, p_load->adProb_, "adProb_", "p_load->adProb_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", adaptionMode_, p_load->adaptionMode_, "adaptionMode_", "p_load->adaptionMode_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GAdaptorT<T>", adaptAdaptionProbability_, p_load->adaptAdaptionProbability_, "adaptAdaptionProbability_", "p_load->adaptAdaptionProbability_", e , limit));

		return evaluateDiscrepancies("GAdaptorT<T>", caller, deviations, e);
	}

	/***********************************************************************************/
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

	/***********************************************************************************/
	/**
	 * Sets the adaption probability to a given value. This function will throw
	 * if the probability is not in the allowed range.
	 *
	 * @param probability The new value of the probability of adaptions taking place
	 */
	void setAdaptionProbability(const double& probability) {
		// Check the supplied probability value
		if(probability < 0. || probability > 1.) {
			raiseException(
					"In GAdaptorT<T>::setAdaptionProbability(const double&):" << std::endl
					<< "Bad probability value given: " << probability
			);
		}

		adProb_ = probability;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of valid probabilities is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * Checks for setting of invalid probabilities is tested in GAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	 * The effects on the probability of adaptions actually taking place are tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
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

	/***********************************************************************************/
	/**
	 * Sets the probability for the adaption of adaption parameters
	 *
	 * @param probability The new value of the probability of adaptions of adaption parameters
	 */
	void setAdaptAdaptionProbability(const double& probability) {
		// Check the supplied probability value
		if(probability < 0. || probability > 1.) {
			raiseException(
					"In GAdaptorT<T>::setAdaptAdaptionProbability(const double&) :" << std::endl
					<< "Bad probability value given: " << probability
			);
		}

		adaptAdaptionProbability_ = probability;
	}

	/* ----------------------------------------------------------------------------------
	 * Setting of valid probabilities is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * Checks for setting of invalid probabilities is tested in GAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
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

	/***********************************************************************************/
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

	/***********************************************************************************/
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

	/***********************************************************************************/
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

	/***********************************************************************************/
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

	/***********************************************************************************/
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

	/***********************************************************************************/
	/**
	 * Common interface for all adaptors to the adaption functionality. The user
	 * specifies the actual actions in the customAdaptions() function.
	 *
	 * @param val The value that needs to be adapted
	 */
	void adapt(T& val) {
		if(boost::logic::indeterminate(adaptionMode_)) { // The most likely case is indeterminate
			if(gr->uniform_01<double>() <= adProb_) { // Should we perform adaption
				adaptAdaption();
				customAdaptions(val);
			}
		}
		else if(adaptionMode_) { // always adapt
			adaptAdaption();
			customAdaptions(val);
		}

		// No need to test for adaptionMode_ == false as no action is needed in this case
	}

	/* ----------------------------------------------------------------------------------
	 * Adaption is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
	/**
	 * Assign a random number generator from another object.
	 *
	 * @param gr_cp A reference to another object's GRandomBase object derivative
	 */
	virtual void assignGRandomPointer(Gem::Hap::GRandomBase *gr_cp) {
#ifdef DEBUG
		if(!gr_cp) {
			raiseException(
					"In GAdaptorT<T>::assignGRandomPointer() :" << std::endl
					<< "Tried to assign NULL pointer"
			);
		}
#endif

		gr = gr_cp;
	}

	/* ----------------------------------------------------------------------------------
	 * - Assigning a random number generator is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * - Assigning a NULL pointer is tested in GAdaptorT<T>::specificTestsFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
	/**
	 * Re-connects the local random number generator to gr.
	 */
	virtual void resetGRandomPointer() {
		if(gr_local) gr = gr_local;
		else {
			raiseException(
					"In GAdaptorT<T>::resetGRandomPointer() :" << std::endl
					<< "Tried to assign NULL pointer"
			);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * - Resetting random number generator is tested in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * - throw is untested
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
	/**
	 * Checks whether the local random number generator is used. This is simply done
	 * by comparing the two pointers.
	 *
	 * @bool A boolean indicating whether the local random number generator is used
	 */
	virtual bool usesLocalRNG() const {
		return gr == gr_local;
	}

	/* ----------------------------------------------------------------------------------
	 * Used in GAdaptorT<T>::specificTestsNoFailuresExpected_GUnitTests()
	 * ----------------------------------------------------------------------------------
	 */

	/***********************************************************************************/
	/**
	 * Checks whether an assigned random number generator is used
	 *
	 * @return A boolean indicating whether an assigned random number generator is used
	 */
	virtual bool assignedRNGUsed() const {
		return gr != gr_local;
	}

protected:
	/***********************************************************************************/
    /**
     * A random number generator. This reference and the associated pointer is either
     * connected to a local random number generator assigned in the constructor, or
     * to a "factory" generator located in the surrounding GParameterSet object.
     */
	Gem::Hap::GRandomBase *gr_local;
	Gem::Hap::GRandomBase *gr;

	/***********************************************************************************/
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
	virtual void load_(const GObject *cp) {
		// Convert cp into local format
		const GAdaptorT<T> *p_load = gobject_conversion<GAdaptorT<T> >(cp);

		// Load the parent class'es data
		GObject::load_(cp);

		// Then our own data
		adaptionCounter_ = p_load->adaptionCounter_;
		adaptionThreshold_ = p_load->adaptionThreshold_;
		adProb_ = p_load->adProb_;
		adaptionMode_ = p_load->adaptionMode_;
		adaptAdaptionProbability_ = p_load->adaptAdaptionProbability_;
	}

	/***********************************************************************************/
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone_(void) const =0;

	/***********************************************************************************/
	/**
	 * 	This function is re-implemented by derived classes, if they wish to
	 *  implement special behavior upon a new adaption run. E.g., an internal
	 *  variable could be set to a new value.
	 */
	virtual void customAdaptAdaption() { /* nothing */ }

	/***********************************************************************************/
	/**
	 * This function helps tp adapt the adaption parameters, if certain conditions are met.
	 */
	void adaptAdaption() {
		// The adaption parameters are modified every adaptionThreshold_ number of adaptions.
		if(adaptionThreshold_) {
			if(++adaptionCounter_ >= adaptionThreshold_){
				adaptionCounter_ = 0;
				customAdaptAdaption();
			}
		} else if(adaptAdaptionProbability_) { // Do the same with probability settings
			if(gr->uniform_01<double>() <= adaptAdaptionProbability_) {
				customAdaptAdaption();
			}
		}
	}

	/***********************************************************************************/
	/** @brief Adaption of values as specified by the user */
	virtual void customAdaptions(T& val)=0;

private:
	/***********************************************************************************/
	boost::uint32_t adaptionCounter_; ///< A local counter
	boost::uint32_t adaptionThreshold_; ///< Specifies after how many adaptions the adaption itself should be adapted
	double adProb_; ///< internal representation of the adaption probability
	boost::logic::tribool adaptionMode_; ///< false == never adapt; indeterminate == adapt with adProb_ probability; true == always adapt
	double adaptAdaptionProbability_; ///< Influences the likelihood for the adaption of the adaption parameters

#ifdef GEM_TESTING
public:
	/***********************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
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
	}

	/***********************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
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

			T testVal = T(0);
			for(double prob=0.; prob<1.; prob+=0.01) {
				// Account for rounding problems
				if(prob > 1.) prob = 1.;

				BOOST_CHECK_NO_THROW(p_test->setAdaptionProbability(prob));
				BOOST_CHECK_NO_THROW(p_test->adapt(testVal));
			}
		}

		//------------------------------------------------------------------------------

		{ // Test of GAdaptorT<T>::setAdaptionProbability() regarding the effects on the likelihood for adaption of the variable
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Make sure the adaption probability is taken into account
			p_test->setAdaptionMode(boost::logic::indeterminate);

			const std::size_t nTests=100000;

			for(double prob=0.; prob<1.; prob+=0.1) {
				// Account for rounding problems
				if(prob > 1.) prob = 1.;

				std::size_t nChanged=0;

				T testVal = T(0);
				T prevTestVal = testVal;

				// Set the likelihood for adaption to "prob"
				p_test->setAdaptionProbability(prob);

				// Mutating a boolean value a number of times should now result in a certain number of changed values
				for(std::size_t i=0; i<nTests; i++) {
					p_test->adapt(testVal);
					if(testVal != prevTestVal) {
						nChanged++;
						prevTestVal=testVal;
					}
				}

				double changeProb = double(nChanged)/double(nTests);

				if(prob==0.) {
					BOOST_CHECK_MESSAGE(
								changeProb<0.0001
								,  "\n"
								<< "changeProb = " << changeProb << "\n"
								<< "prob = " << prob << "\n"
								<< "with allowed window = [" << 0. << " : " << 0.0001 << "]" << "\n"
					);
				}
				else {
					BOOST_CHECK_MESSAGE(
							changeProb>0.95*prob && changeProb<1.05*prob
							,  "\n"
							<< "changeProb = " << changeProb << "\n"
							<< "prob = " << prob << "\n"
							<< "with allowed window = [" << 0.95*prob << " : " << 1.05*prob << "]" << "\n"
					);
				}
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
				p_test->adapt(currentValue);
				BOOST_CHECK_MESSAGE (
						currentValue == oldValue
						,  "\n"
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
				p_test->adapt(currentValue);
				BOOST_CHECK_MESSAGE (
						currentValue != oldValue
						,  "\n"
						<< "currentValue = " << currentValue << "\n"
						<< "oldValue     = " << oldValue << "\n"
						<< "iteration    = " << i << "\n"
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
				assert(adThr >= 0 && adThr<=10);

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
					p_test->adapt(testVal);

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

		//------------------------------------------------------------------------------

		{ // Check adding and resetting of random number generators
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			// Always adapt
			p_test->setAdaptionMode(true);

			// A cloned adaptor should have a local random number generator, as it is default-constructed
			BOOST_CHECK(p_test->usesLocalRNG());

			// Some values that will be adapted
			T testVal = T(0);
			T oldTestVal = T(0);

			// Check that we have adaption powers when using a local random number generator
			for(std::size_t i=0; i<1000; i++) {
				p_test->adapt(testVal);
				BOOST_CHECK_MESSAGE(
						testVal != oldTestVal
						,  "(1)\n"
						<< "testVal = " << testVal << "\n"
						<< "oldTestVal = " << oldTestVal << "\n"
						<< "iteration = " << i << "\n"
				);
				oldTestVal = testVal;
			}

			// Assign a factory generator
			Gem::Hap::GRandomBase *gr_test = new Gem::Hap::GRandomT<Gem::Hap::RANDOMPROXY>();

			BOOST_CHECK_NO_THROW(p_test->assignGRandomPointer(gr_test));

			// Has the generator been assigned ?
			BOOST_CHECK(!p_test->usesLocalRNG());

			// Check that we have adaption powers when using the new random number generator
			testVal = T(0);
			oldTestVal = T(0);
			for(std::size_t i=0; i<1000; i++) {
				BOOST_CHECK_NO_THROW(p_test->adapt(testVal));
				BOOST_CHECK_MESSAGE(
						testVal != oldTestVal
						,  "(2)\n"
						<< "testVal = " << testVal << "\n"
						<< "oldTestVal = " << oldTestVal << "\n"
						<< "iteration = " << i << "\n"
				);
				oldTestVal = testVal;
			}

			// Make sure we use the local generator again
			BOOST_CHECK_NO_THROW(p_test->resetGRandomPointer());

			// Get rid of the test generator
			delete gr_test;

			// We should now be using a local random number generator again
			BOOST_CHECK(p_test->usesLocalRNG());

			// Check that we have adaption powers when using the local random number generator again
			testVal = T(0);
			oldTestVal = T(0);
			for(std::size_t i=0; i<1000; i++) {
				p_test->adapt(testVal);
				BOOST_CHECK_MESSAGE(
						testVal != oldTestVal
						,  "(3)\n"
						<< "testVal = " << testVal << "\n"
						<< "oldTestVal = " << oldTestVal << "\n"
						<< "iteration = " << i << "\n"
				);
				oldTestVal = testVal;
			}
		}

		//------------------------------------------------------------------------------

		{ // Test that customAdaptions() in derived classes changes a test value on every call
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			std::size_t nTests = 10000;

			T testVal = T(0);
			T oldTestVal = T(0);
			for(std::size_t i=0; i<nTests; i++) {
				BOOST_CHECK_NO_THROW(p_test->customAdaptions(testVal));
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
	}

	/***********************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes.
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
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


		{ // Check that resetting the random number generator throws if gr_local is NULL
			boost::shared_ptr<GAdaptorT<T> > p_test = this->clone<GAdaptorT<T> >();

			p_test->gr_local = NULL;

			// Resetting the pointer should throw, if gr_local is NULL (which it technically
			// should never be able to become
			BOOST_CHECK_THROW(
					p_test->resetGRandomPointer();
					, Gem::Common::gemfony_error_condition
			);
		}

		//------------------------------------------------------------------------------
	}

#endif /* GEM_TESTING */
};

/******************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/********************************************************************************************/
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

/********************************************************************************************/

#endif /* GADAPTORT_HPP_ */
