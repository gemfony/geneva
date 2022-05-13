/**
 * @file GMPIEvaluatedIndividual.hpp
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

namespace Gem {
namespace Geneva {

/******************************************************************/
/**
 * This individual searches for the minimum of a 2-dimensional parabola.
 * It is part of an introductory example, used in the Geneva manual.
 */
class GMPIEvaluatedIndividual :public GParameterSet
{
	 /** @brief Make the class accessible to Boost.Serialization */
	 friend class boost::serialization::access;

	 /**************************************************************/
	 /**
	  * This function triggers serialization of this class and its
	  * base classes.
	  */
	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 // Serialize the base class
		 ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
		 // Add other variables here like this:
		 // ar & BOOST_SERIALIZATION_NVP(sampleVariable);
	 }
	 /**************************************************************/
public:
	 /** @brief The default constructor */
	 GMPIEvaluatedIndividual();
	 /** @brief A standard copy constructor */
	 GMPIEvaluatedIndividual(const GMPIEvaluatedIndividual&);
	 /** @brief The standard destructor */
	 virtual ~GMPIEvaluatedIndividual();

protected:
	 /** @brief Loads the data of another GMPIEvaluatedIndividual */
	 virtual void load_(const GObject*) final;

	 /** @brief The actual fitness calculation takes place here. */
	 virtual double fitnessCalculation() final;

private:
	 /** @brief Creates a deep clone of this object */
	 virtual GObject* clone_() const final;

	 const double M_PAR_MIN;
	 const double M_PAR_MAX;
};

/******************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMPIEvaluatedIndividual)

