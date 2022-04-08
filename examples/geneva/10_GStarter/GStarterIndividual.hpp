/**
 * @file GStarterIndividual.hpp
 */

/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <cmath>
#include <memory>
#include <sstream>
#include <vector>
#include <tuple>

// Boost header files go here

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "common/GParserBuilder.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum class targetFunction : Gem::Common::ENUMBASETYPE {
	 PARABOLA=0
	 , NOISYPARABOLA=1
};

// Make sure targetFunction can be streamed
/** @brief Puts a Gem::Geneva::targetFunction into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::targetFunction&);

/** @brief Reads a Gem::Geneva::targetFunction from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::targetFunction&);

/******************************************************************************/
// A number of default settings for the factory
const double GSI_DEF_ADPROB = 1.0;
const double GSI_DEF_SIGMA = 0.025;
const double GSI_DEF_SIGMASIGMA = 0.2;
const double GSI_DEF_MINSIGMA = 0.001;
const double GSI_DEF_MAXSIGMA = 1;
const targetFunction GO_DEF_TARGETFUNCTION = targetFunction::PARABOLA;

/******************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GStarterIndividual : public GParameterSet
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<class Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		 & BOOST_SERIALIZATION_NVP(m_targetFunction);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 GStarterIndividual();
	 /** @brief A constructor that receives all arguments */
	 GStarterIndividual(
		 const std::size_t&
		 , const std::vector<double>&
		 , const std::vector<double>&
		 , const std::vector<double>&
		 , const double&
		 , const double&
		 , const double&
		 , const double&
		 , const double&
	 );
	 /** @brief A standard copy constructor */
	 GStarterIndividual(const GStarterIndividual&);
	 /** @brief The standard destructor */
	 ~GStarterIndividual() override;

	 /** @brief Adds local configuration options to a GParserBuilder object */
         virtual void addConfigurationOptions(Gem::Common::GParserBuilder&) final;

	 /** @brief Allows to set the demo function */
	 void setTargetFunction(targetFunction);
	 /** @brief Allows to retrieve the current demo function */
	 [[nodiscard]] targetFunction getTargetFunction() const;

	 /** @brief Retrieves the average value of the sigma used in local Gauss adaptors */
	 [[nodiscard]] double getAverageSigma() const;

	 /** @brief Emit information about this individual */
	 std::string print();

	 /***************************************************************************/
	 /**
	  * This function is used to unify the setup from within the constructor
	  * and factory.
	  */
	 static void addContent(
		 GStarterIndividual& p
		 , const std::size_t& prod_id
		 , const std::vector<double>& startValues
		 , const std::vector<double>& lowerBoundaries
		 , const std::vector<double>& upperBoundaries
		 , const double& sigma
		 , const double& sigmaSigma
		 , const double& minSigma
		 , const double& maxSigma
		 , const double& adProb
	 ) {
		 // Some error checking
#ifdef DEBUG
		 // Check whether values have been provided
		 if(startValues.empty()) {
			 glogger
				 << "In GStarterIndividual::addContent(): Error!" << std::endl
				 << "No parameters given" << std::endl
				 << GTERMINATION;
		 }

		 // Check whether all sizes match
		 if(startValues.size() != lowerBoundaries.size() || startValues.size() != upperBoundaries.size()) {
			 glogger
				 << "In GStarterIndividual::addContent(): Error!" << std::endl
				 << "Invalid sizes" << startValues.size() << " / " << lowerBoundaries.size() << " / " << upperBoundaries.size() << std::endl
				 << GTERMINATION;
		 }

		 // Check that start values and boundaries have valid values
		 for(std::size_t i=0; i<startValues.size(); i++) {
			 Gem::Common::checkValueRange( // We expect the start value to be in the range [lower, upper[
				 startValues.at(i)
				 , lowerBoundaries.at(i)
				 , upperBoundaries.at(i)
				 , false // closed lower boundary
				 , true  // open upper boundary
			 );
		 }

#endif /* DEBUG */

		 // Add the required number of GConstrainedDoubleObject objects to the individual
		 for(std::size_t i=0; i<startValues.size(); i++) {
			 std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr;
			 if(Gem::Common::GFACTTORYFIRSTID == prod_id) { // First individual, initialization with standard values
				 gcdo_ptr = std::make_shared<GConstrainedDoubleObject> (

						 startValues.at(i)
						 , lowerBoundaries.at(i)
						 , upperBoundaries.at(i)

				 );
			 } else { // Random initialization for all other individuals
				 gcdo_ptr = std::make_shared<GConstrainedDoubleObject> (

						 lowerBoundaries.at(i)
						 , upperBoundaries.at(i)

				 );
			 }

			 std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(
				 new GDoubleGaussAdaptor(
					 sigma
					 , sigmaSigma
					 , minSigma
					 , maxSigma
				 )
			 );

			 gdga_ptr->setAdaptionProbability(adProb);
			 gcdo_ptr->addAdaptor(gdga_ptr);

			 p.push_back(gcdo_ptr);
		 }
	 }

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another GStarterIndividual */
	 void load_(const GObject*) final;

	/** @brief Allow access to this classes compare_ function */
	friend void Gem::Common::compare_base_t<GStarterIndividual>(
		GStarterIndividual const &
		, GStarterIndividual const &
		, Gem::Common::GToken &
	);

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	void compare_(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const final;

	/** @brief The actual value calculation takes place here */
	double fitnessCalculation() final;

	/** @brief Applies modifications to this object. */
	bool modify_GUnitTests_() override;
	/** @brief Performs self tests that are expected to succeed. */
	void specificTestsNoFailureExpected_GUnitTests_() override;
	/** @brief Performs self tests that are expected to fail. */
	void specificTestsFailuresExpected_GUnitTests_() override;

private:
	/***************************************************************************/

	targetFunction m_targetFunction = GO_DEF_TARGETFUNCTION; ///< Specifies which demo function should be used

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 [[nodiscard]] GObject* clone_() const final;

	 /***************************************************************************/
	 /** @brief A simple n-dimensional parabola */
	 [[nodiscard]] double parabola(const std::vector<double>& parVec) const;
	 /** @brief A "noisy" parabola */
	 [[nodiscard]] double noisyParabola(const std::vector<double>& parVec) const;

	 /***************************************************************************/
};

/** @brief Allows to output a GStarterIndividual or convert it to a string using boost::lexical_cast */
std::ostream& operator<<(std::ostream&, std::shared_ptr<GStarterIndividual>);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GStarterIndividual objects
 */
class GStarterIndividualFactory
	: public Gem::Common::GFactoryT<GParameterSet>
{
public:
	 /** @brief The standard constructor */
	 explicit GStarterIndividualFactory(std::filesystem::path const&);
	 /** @brief The destructor */
	 ~GStarterIndividualFactory() override = default;

protected:
	 /** @brief Allows to describe local configuration options in derived classes */
	 void describeLocalOptions_(Gem::Common::GParserBuilder&) override;
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 void postProcess_(std::shared_ptr<GParameterSet>&) override;

private:
	 /** @brief The default constructor. Only needed for (de-)serialization purposes */
	 GStarterIndividualFactory() = default;
     /** @brief Creates individuals of this type */
     std::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&) override;

	 double m_adProb = GSI_DEF_ADPROB; ///< Probability for a parameter to be mutated
	 double m_sigma = GSI_DEF_SIGMA; ///< Step-width
	 double m_sigmaSigma = GSI_DEF_SIGMASIGMA; ///< Speed of sigma_-adaption
	 double m_minSigma = GSI_DEF_MINSIGMA; ///< Minimum allowed sigma value
	 double m_maxSigma = GSI_DEF_MAXSIGMA; ///< Maximum allowed sigma value

	 std::vector<double> m_startValues; ///< Start values for all parameters
	 std::vector<double> m_lowerBoundaries; ///< Lower boundaries for all parameters
	 std::vector<double> m_upperBoundaries; ///< Upper boundaroes for all parameters
};

/******************************************************************************/

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GStarterIndividual)

