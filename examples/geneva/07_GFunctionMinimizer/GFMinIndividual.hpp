/**
 * @file GFMinIndividual.hpp
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
#include <sstream>
#include <vector>
#include <tuple>

// Boost header files go here

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum class targetFunction : Gem::Common::ENUMBASETYPE {
	 GFM_PARABOLA=0
	 , GFM_NOISYPARABOLA=1
};

// Make sure targetFunction can be streamed
/** @brief Puts a Gem::Geneva::targetFunction into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::targetFunction&);

/** @brief Reads a Gem::Geneva::targetFunction from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::targetFunction&);

/******************************************************************************/
// A number of default settings for the factory
const double GFI_DEF_ADPROB = 1.0;
const double GFI_DEF_SIGMA = 0.025;
const double GFI_DEF_SIGMASIGMA = 0.2;
const double GFI_DEF_MINSIGMA = 0.001;
const double GFI_DEF_MAXSIGMA = 1;
const std::size_t GFI_DEF_PARDIM = 2;
const double GFI_DEF_MINVAR = -10.;
const double GFI_DEF_MAXVAR = 10.;
const targetFunction GO_DEF_TARGETFUNCTION = targetFunction::GFM_PARABOLA;

/******************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GFMinIndividual
	: public GParameterSet
{
	 /////////////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<class Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		 & BOOST_SERIALIZATION_NVP(targetFunction_);
	 }

	 /////////////////////////////////////////////////////////////////////////////

public:
	 /** @brief The default constructor */
	 GFMinIndividual();
	 /** @brief A standard copy constructor */
	 GFMinIndividual(const GFMinIndividual&);
	 /** @brief The standard destructor */
	 virtual ~GFMinIndividual();

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 virtual void addConfigurationOptions(Gem::Common::GParserBuilder&) final;

	 /** @brief Allows to set the demo function */
	 void setTargetFunction(targetFunction);
	 /** @brief Allows to retrieve the current demo function */
	 targetFunction getTargetFunction() const;

	 /** @brief Retrieves the average value of the sigma used in Gauss adaptors */
	 double getAverageSigma() const;

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another GFMinIndividual */
	 virtual void load_(const GObject*) final;

	 /** @brief The actual value calculation takes place here */
	 virtual double fitnessCalculation() final;

	 /***************************************************************************/

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 virtual GObject* clone_() const final;

	 /***************************************************************************/
	 targetFunction targetFunction_ = GO_DEF_TARGETFUNCTION; ///< Specifies which demo function should be used

	 /***************************************************************************/
	 /** @brief A simple n-dimensional parabola */
	 double parabola(const std::vector<double>& parVec) const;
	 /** @brief A "noisy" parabola */
	 double noisyParabola(const std::vector<double>& parVec) const;

	 /***************************************************************************/
};

/******************************************************************************/
/**
 * Provides an easy way to print the individual's content
 */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::GFMinIndividual&);
std::ostream& operator<<(std::ostream&, std::shared_ptr<Gem::Geneva::GFMinIndividual>);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFMinIndividual objects
 */
class GFMinIndividualFactory
	: public Gem::Common::GFactoryT<GParameterSet>
{
public:
	 /** @brief The standard constructor */
	 explicit GFMinIndividualFactory(std::filesystem::path const&);
	 /** @brief The destructor */
	 virtual ~GFMinIndividualFactory();

protected:
	 /** @brief Creates individuals of this type */
	 virtual std::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	 /** @brief Allows to describe local configuration options in derived classes */
	 virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 virtual void postProcess_(std::shared_ptr<GParameterSet>&);

private:
	 /** @brief The default constructor. Only needed for (de-)serialization purposes */
	 GFMinIndividualFactory() = default;

	 double adProb_;
	 double sigma_;
	 double sigmaSigma_;
	 double minSigma_;
	 double maxSigma_;
	 std::size_t parDim_;
	 double minVar_;
	 double maxVar_;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFMinIndividual)
