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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <algorithm> // for std::sort
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <list>
#include <sstream>
#include <tuple>
#include <vector>

// Boost header files go here
#include <boost/algorithm/string/trim.hpp>

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GFactoryT.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_GradientDescent_PersonalityTraits.hpp"
#include "geneva/G_OptimizationAlgorithm_SwarmAlgorithm_PersonalityTraits.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * This individual takes a vector of 2D double-tuples and calculates the
 * root-square deviation from the line defined by its two parameters
 */
class GLineFitIndividual // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Geneva::GParameterSet {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet) &
            BOOST_SERIALIZATION_NVP(dataPoints_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GLineFitIndividual(const std::vector<std::tuple<double, double>> &);
    /** @brief The copy constructor */
    GLineFitIndividual(const GLineFitIndividual &);

    /** @brief The standard destructor */
    virtual ~GLineFitIndividual();

    /** @brief Retrieves the tuple (a,b) of the line represented by this object */
    std::tuple<double, double> getLine() const;

protected:
    /** @brief Loads the data of another GLineFitIndividual */
    virtual void load_(const GObject *) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GLineFitIndividual>(
        GLineFitIndividual const &,
        GLineFitIndividual const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    virtual void compare_(
        const GObject & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /** @brief The actual fitness calculation takes place here. */
    virtual double fitnessCalculation() final;

    /** @brief Applies modifications to this object. */
    virtual bool modify_GUnitTests_();
    /** @brief Performs self tests that are expected to succeed. */
    virtual void specificTestsNoFailureExpected_GUnitTests_();
    /** @brief Performs self tests that are expected to fail. */
    virtual void specificTestsFailuresExpected_GUnitTests_();

private:
    /** @brief Creates a deep clone of this object */
    virtual GObject *clone_() const final;

    /** @brief The default constructor -- private, as it is only needed for (de-)serialization purposes */
    GLineFitIndividual();

    std::vector<std::tuple<double, double>>
        dataPoints_; ///< Holds the data points used for the fit procedure
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GLineFitIndividual objects
 */
class GLineFitIndividualFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GFactoryT<GParameterSet> {
public:
    /** @brief The standard constructor */
    GLineFitIndividualFactory(
        const std::vector<std::tuple<double, double>> &,
        std::filesystem::path const &
    );

    /** @brief The destructor */
    virtual ~GLineFitIndividualFactory();

protected:
    /** @brief Allows to describe local configuration options in derived classes */
    virtual void describeLocalOptions_(Gem::Common::GParserBuilder &);

    /** @brief Allows to act on the configuration options received from the configuration file */
    virtual void postProcess_(std::shared_ptr<GParameterSet> &);

private:
    /** @brief The default constructor. Only needed for (de-)serialization purposes */
    GLineFitIndividualFactory() = default;

    /** @brief Creates individuals of this type */
    virtual std::shared_ptr<GParameterSet>
    getObject_(Gem::Common::GParserBuilder &, const std::size_t &);

    std::vector<std::tuple<double, double>> dataPoints_; ///< Holds data points for the fit
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GLineFitIndividual) // NOLINT
