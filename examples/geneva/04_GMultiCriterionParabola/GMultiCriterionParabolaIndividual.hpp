/**
 * @file GMultiCriterionParabolaIndividual.hpp
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

// Boost header files go here

// Geneva header files go here
#include <geneva/GParameterSet.hpp>
#include <geneva/GConstrainedDoubleObject.hpp>
#include <common/GFactoryT.hpp>
#include <common/GParserBuilder.hpp>
#include <common/GCommonHelperFunctions.hpp>

namespace Gem {
namespace Geneva {

// The number of parameters
const std::size_t NPAR_MC = 3;

/******************************************************************************/
/**
 * This individual implements several, possibly conflicting evaluation
 * criteria, each implemented as a parabola with its own minimum
 */
class GMultiCriterionParabolaIndividual :public GParameterSet
{
	 friend class GMultiCriterionParabolaIndividualFactory;

	 /***************************************************************************/
	 /**
	  * This function triggers serialization of this class and its
	  * base classes.
	  */
	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	 }

	 /** @brief Make the class accessible to Boost.Serialization */
	 friend class boost::serialization::access;
	 /***************************************************************************/

public:
	 /** @brief The standard constructor */
	 GMultiCriterionParabolaIndividual(const std::size_t&);
	 /** @brief A standard copy constructor */
	 GMultiCriterionParabolaIndividual(const GMultiCriterionParabolaIndividual&);
	 /** @brief The destructor */
	 virtual ~GMultiCriterionParabolaIndividual();

	 /** @brief Assigns a number of minima to this object */
	 void setMinima(const std::vector<double>&);

protected:
	 /** @brief Loads the data of another GMultiCriterionParabolaIndividual */
	 virtual void load_(const GObject*) final;

	 /** @brief The actual fitness calculation takes place here. */
	 virtual double fitnessCalculation() final;

private:
	 /** @brief Creates a deep clone of this object */
	 virtual GObject* clone_() const final;

	 /** @brief The default constructor -- intentionally private*/
	 GMultiCriterionParabolaIndividual();
	 /** @brief Holds the minima needed for multi-criterion optimization */
	 std::vector<double> minima_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GMultiCriterionParabolaIndividual objects
 */
class GMultiCriterionParabolaIndividualFactory
	:public Gem::Common::GFactoryT<GParameterSet>
{
public:
	 /** @brief The standard constructor for this class */
	 GMultiCriterionParabolaIndividualFactory(std::filesystem::path const&);
	 /** @brief The destructor */
	 virtual ~GMultiCriterionParabolaIndividualFactory();

protected:
	 /** @brief Creates individuals of this type */
	 virtual std::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	 /** @brief Allows to describe local configuration options in derived classes */
	 virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 virtual void postProcess_(std::shared_ptr<GParameterSet>&);

private:
	 /** @brief The default constructor. Only needed for (de-)serialization purposes */
	 GMultiCriterionParabolaIndividualFactory() = default;

	 Gem::Common::GOneTimeRefParameterT<double> par_min_; ///< The lower boundary of the initialization range
	 Gem::Common::GOneTimeRefParameterT<double> par_max_; ///< The upper boundary of the initialization range
	 Gem::Common::GOneTimeRefParameterT<std::string> minima_string_; ///< The minima encoded as a string

	 std::vector<double> minima_; ///< The desired minima of the parabolas
	 std::size_t nPar_; ///< The number of parameters to be added to the individual
	 bool firstParsed_; ///< Set to false when the configuration files were parsed for the first time
};

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content
 */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::GMultiCriterionParabolaIndividual&);
std::ostream& operator<<(std::ostream&, std::shared_ptr<Gem::Geneva::GMultiCriterionParabolaIndividual>);

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMultiCriterionParabolaIndividual)

