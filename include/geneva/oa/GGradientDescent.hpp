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

// Standard headers go here
#include <memory>
#include <tuple>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "geneva/oa/GBase.hpp"
#include "geneva/oa/GGradientDescent_PersonalityTraits.hpp"

#ifdef GEM_TESTING

#include "geneva/individuals/GTestIndividual1.hpp"

#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {

/**
 * The default number of simultaneous starting points for the gradient descent
 */
constexpr std::size_t DEFAULTGDSTARTINGPOINTS = 1;
constexpr double DEFAULTFINITESTEP = 0.001;
constexpr double DEFAULTSTEPSIZE = 0.1;

/******************************************************************************/
/**
 * The GGradientDescent class implements a steepest descent algorithm. It is possible
 * to search for optima starting from several positions simultaneously. This class limits
 * itself to evaluation through the Courtier framework, i.e. all evaluation of individuals
 * is delegated to the Broker (which may in turn use other means, such as threads or
 * networked execution for the evaluation step).
 */
class GGradientDescent // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBase {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBase",
            boost::serialization::base_object<GBase>(*this)
        ) & BOOST_SERIALIZATION_NVP(n_starting_points_) &
            BOOST_SERIALIZATION_NVP(n_fp_parms_first_) & BOOST_SERIALIZATION_NVP(finite_step_) &
            BOOST_SERIALIZATION_NVP(step_size_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GGradientDescent();
    /** @brief Initialization with the number of starting points and the size of the finite step */
    GGradientDescent(const std::size_t &, const double &, const double &);
    /** @brief A standard copy constructor */
    GGradientDescent(const GGradientDescent &) = default;
    /** @brief The destructor */
    ~GGradientDescent() override = default;

    /** @brief Retrieves the number of starting points of the algorithm */
    std::size_t getNStartingPoints() const;
    /** @brief Allows to set the number of starting points for the gradient descent */
    void setNStartingPoints(std::size_t);

    /** @brief Set the size of the finite step of the adaption process */
    void setFiniteStep(double);
    /** @brief Retrieve the size of the finite step of the adaption process */
    double getFiniteStep() const;

    /** @brief Sets a multiplier for the adaption process */
    void setStepSize(double);
    /** @brief Retrieves the current step size */
    double getStepSize() const;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

    /** @brief Single declaration of this class'es serialized local data members */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("n_starting_points_", n_starting_points_),
            Gem::Common::make_member("n_fp_parms_first_", n_fp_parms_first_),
            Gem::Common::make_member("finite_step_", finite_step_),
            Gem::Common::make_member("step_size_", step_size_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("n_starting_points_", n_starting_points_),
            Gem::Common::make_member("n_fp_parms_first_", n_fp_parms_first_),
            Gem::Common::make_member("finite_step_", finite_step_),
            Gem::Common::make_member("step_size_", step_size_)
        );
    }

    /** @brief Loads the data of another population */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GGradientDescent>(
        GGradientDescent const &,
        GGradientDescent const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
    void resetToOptimizationStart_() override;

    /** @brief Does some preparatory work before the optimization starts */
    void init() override;
    /** @brief Does any necessary finalization work */
    void finalize() override;

    /** @brief Updates the individual parameters of children */
    virtual void updateChildParameters();

    /** @brief Performs a step of the parent individuals */
    virtual void updateParentIndividuals();

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

private:
    /***************************************************************************/
    // Virtual or overridden private functions

    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
    std::tuple<double, double> cycleLogic_() override;
    /** @brief Triggers fitness calculation of a number of individuals */
    void runFitnessCalculation_() override;

    /** @brief Returns information about the type of optimization algorithm */
    std::string getAlgorithmPersonalityType_() const override;
    /** @brief Returns the name of this optimization algorithm */
    std::string getAlgorithmName_() const override;

    /** @brief Retrieves the number of processable items for the current iteration */
    std::size_t getNProcessableItems_() const override;

    /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;
    /** @brief Resizes the population to the desired level and does some error checks */
    void adjustPopulation_() override;

    /** @brief Gives individuals an opportunity to update their internal structures */
    void actOnStalls_() override;

    /***************************************************************************/
    // Data

    std::size_t n_starting_points_ =
        DEFAULTGDSTARTINGPOINTS;    ///< The number of starting positions in the parameter space
    std::size_t n_fp_parms_first_ = 0; ///< The amount of floating point values in the first individual

    double finite_step_ =
        DEFAULTFINITESTEP; ///< The size of the incremental adaption of the feature vector
    double step_size_ = DEFAULTSTEPSIZE; ///< A multiplicative factor for the adaption
    long double step_ratio_ =
        (DEFAULTSTEPSIZE /
         DEFAULTFINITESTEP); ///< The ratio of step_size_ and finite_step_. NOTE: long double; Will be recalculated in init()

    std::vector<double>
        dbl_lower_parameter_boundaries_; ///< Holds lower boundaries of double parameters; Will be extracted in init()
    std::vector<double>
        dbl_upper_parameter_boundaries_; ///< Holds upper boundaries of double parameters; Will be extracted in init()
    std::vector<double>
        adjusted_finite_step_; ///< A step-size normalized to each parameter range; Will be recalculated in init()

    /** @brief Lets individuals know about their position in the population */
    void markIndividualPositions();
    /** @brief Recomputes step_ratio_ and adjusted_finite_step_ from the raw inputs */
    void updateDerivedQuantities();
};

/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GGradientDescent) // NOLINT

