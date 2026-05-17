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
#include <chrono>
#include <filesystem>
#include <string>

// Boost header files go here

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GFactoryT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "geneva/GPluggableOptimizationMonitors.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for optimization algorithms.
 */
template <typename oa_type>
class G_OptimizationAlgorithm_FactoryT // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GFactoryT<oa_type> {
public:
    /***************************************************************************/
    // Let the audience know what type of algorithm will be produced
    using pType = oa_type;

    /***************************************************************************/
    // Deleted default constructor
    G_OptimizationAlgorithm_FactoryT() = delete;

    /***************************************************************************/
    /**
	  * Initialization with the name of a config file
	  */
    explicit G_OptimizationAlgorithm_FactoryT(std::filesystem::path const &config_file)
      : Gem::Common::GFactoryT<oa_type>(config_file) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * A constructor which adds a content creation function
	  */
    G_OptimizationAlgorithm_FactoryT(
        std::filesystem::path const &config_file,
        std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> content_creator_ptr
    )
      : Gem::Common::GFactoryT<oa_type>(config_file)
      , contentCreatorPtr_(content_creator_ptr) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The copy constructor
	  */
    G_OptimizationAlgorithm_FactoryT(const G_OptimizationAlgorithm_FactoryT<oa_type> &cp)
      : Gem::Common::GFactoryT<oa_type>(cp)
      , maxIterationCL_(cp.maxIterationCL_)
      , maxStallIterationCL_(cp.maxStallIterationCL_)
      , maxSecondsCL_(cp.maxSecondsCL_) {
        if(cp.contentCreatorPtr_) {
            if(contentCreatorPtr_) {
                contentCreatorPtr_->load(cp.contentCreatorPtr_);
            }
            else {
                contentCreatorPtr_ = cp.contentCreatorPtr_->clone();
            }
        }
        else {
            contentCreatorPtr_.reset();
        }
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~G_OptimizationAlgorithm_FactoryT() override = default;

    /***************************************************************************/
    /**
	  * Adds local command line options to a boost::program_options::options_description object.
	  * These are options common to all implemented algorithms. The command line parameter,
	  * however, needs to be specific to a given algorithm, so we can select which algorithm
	  * should receive which option. This happens with the help of the small mnemonic assigned
	  * to each algorithm (e.g. "ea" for evolutionary algorithms). In order not to "clutter"
	  * the output, some options are hidden and will only be shown upon explicit request by
	  * the user
	  *
	  * @param visible Command line options that should always be visible
	  * @param hidden Command line options that should only be visible upon request
	  */
    virtual void addCLOptions(
        boost::program_options::options_description & /*visible*/
        ,
        boost::program_options::options_description &hidden
    ) {
        namespace po = boost::program_options;

        hidden.add_options()(
			 (this->getMnemonic() + std::string("MaxIterations")).c_str()
			 , po::value<std::int32_t>(&maxIterationCL_)->default_value(-1)
			 , (std::string("\t[G_OptimizationAlgorithm_FactoryT / ") + this->getMnemonic() +
				 "] The maximum allowed number of iterations or 0 to disable limit").c_str()
		 )(
			 (this->getMnemonic() + std::string("MaxStallIterations")).c_str()
			 , po::value<std::int32_t>(&maxStallIterationCL_)->default_value(-1)
			 , (std::string("\t[G_OptimizationAlgorithm_FactoryT / ") + this->getMnemonic() +
				 "] The maximum allowed number of stalled iterations or 0 to disable limit").c_str()
		 )(
			 (this->getMnemonic() + std::string("MaxSeconds")).c_str()
			 , po::value<std::int32_t>(&maxSecondsCL_)->default_value(-1)
			 , (std::string("\t[G_OptimizationAlgorithm_FactoryT / ") + this->getMnemonic() +
				 "] The maximum allowed duration in seconds or 0 to disable limit").c_str()
		 );
    }

    /***************************************************************************/
    /**
	  * Triggers the creation of objects of the desired type and converts them
	  * to a given target type. Will throw if conversion is unsuccessful.
	  *
	  * @return A converted copy of the desired production type
	  */
    template <typename target_type>
    std::shared_ptr<target_type> get() {
        return Gem::Common::convertSmartPointer<oa_type, target_type>(
            Gem::Common::GFactoryT<oa_type>::get()
        );
    }

    /***************************************************************************/
    /**
	  * Allows to register a content creator
	  */
    void registerContentCreator(std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> cc_ptr) {
        if(not cc_ptr) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GOptiomizationAlgorithmFactoryT<T>::registerContentCreator(): Error!"
                << '\n'
                << "Tried to register an empty pointer" << '\n'
            );
        }

        contentCreatorPtr_ = cc_ptr;
    }

    /***************************************************************************/
    /**
	  * Allows to register a pluggable optimization monitor
	  */
    void registerPluggableOM(std::shared_ptr<GBasePluggableOM> pluggable_om) {
        if(pluggable_om) {
            pluggableOM_ = pluggable_om;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In G_OptimizationAlgorithm_FactoryT<>::registerPluggableOM(): Tried to "
                   "register empty pluggable optimization monitor"
                << '\n'
            );
        }
    }

    /***************************************************************************/
    /**
	  * Allows to reset the local pluggable optimization monitor
	  */
    void resetPluggableOM() {
        pluggableOM_.reset();
    }

    /***************************************************************************/
    /**
	  * Gives access to the mnemonics / nickname describing an algorithm
	  */
    virtual std::string getMnemonic() const = 0;

    /***************************************************************************/
    /**
	  * Gives access to a clear-text description of an algorithm
	  */
    virtual std::string getAlgorithmName() const = 0;

    /***************************************************************************/
    /**
	  * Allows to manually set the maximum number of iterations as is usually specified on the command line
	  */
    void setMaxIterationCL(std::uint32_t max_iteration_cl) {
        maxIterationCL_ = Gem::Common::narrow_cast<std::int32_t>(max_iteration_cl);
    }

    /***************************************************************************/
    /**
	  * Allows to check whether the maximum number of iterations was set on the command line or using the manual function
	  */
    bool maxIterationsCLSet() const {
        if(maxIterationCL_ >= 0) {
            return true;
        }
        else {
            return false;
        }
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the maximum number of iterations as set on the command line
	  */
    std::uint32_t getMaxIterationCL() const {
        if(maxIterationCL_ >= 0) {
            return Gem::Common::narrow_cast<std::uint32_t>(maxIterationCL_);
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In G_OptimizationAlgorithm_Base<>::getMaxIterationCL(): Error!" << '\n'
                << "maxIterationCL_ wasn't set" << '\n'
            );

            // Make the compiler happy
            return static_cast<std::uint32_t>(0);
        }
    }

    /***************************************************************************/
    /**
	  * Allows to manually set the maximum number of stall iterations as is usually specified on the command line
	  */
    void setMaxStallIterationCL(std::uint32_t max_stall_iteration_cl) {
        maxStallIterationCL_ = Gem::Common::narrow_cast<std::int32_t>(max_stall_iteration_cl);
    }

    /***************************************************************************/
    /**
	  * Allows to check whether the maximum number of stall iterations was set on the command line or using the manual function
	  */
    bool maxStallIterationsCLSet() const {
        if(maxStallIterationCL_ >= 0) {
            return true;
        }
        else {
            return false;
        }
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the maximum number of stall iterations as set on the command line
	  */
    std::uint32_t getMaxStallIterationCL() const {
        if(maxStallIterationCL_ >= 0) {
            return Gem::Common::narrow_cast<std::uint32_t>(maxStallIterationCL_);
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In G_OptimizationAlgorithm_Base<>::getMaxStallIterationCL(): Error!"
                << '\n'
                << "maxStallIterationCL_ wasn't set" << '\n'
            );

            // Make the compiler happy
            return static_cast<std::uint32_t>(0);
        }
    }

    /***************************************************************************/
    /**
	  * Allows to manually set the maximum number of seconds for a run as is usually specified on the command line
	  */
    void setMaxSecondsCL(std::uint32_t max_seconds_cl) {
        maxSecondsCL_ = Gem::Common::narrow_cast<std::int32_t>(max_seconds_cl);
    }

    /***************************************************************************/
    /**
	  * Allows to check whether the maximum number of seconds was set on the command line or using the manual function
	  */
    bool maxSecondsCLSet() const {
        if(maxSecondsCL_ >= 0) {
            return true;
        }
        else {
            return false;
        }
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the maximum number of seconds as set on the command line
	  */
    std::chrono::duration<double> getMaxTimeCL() const {
        if(maxSecondsCL_ >= 0) {
            std::chrono::duration<double> max_duration =
                std::chrono::seconds(Gem::Common::narrow_cast<long>(maxSecondsCL_));
            return max_duration;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In G_OptimizationAlgorithm_Base<>::getMaxTimeCL(): Error!" << '\n'
                << "maxSecondsCL_ wasn't set" << '\n'
            );

            // Make the compiler happy
            return std::chrono::seconds(0);
        }
    }

protected:
    /***************************************************************************/
    /**
      * Triggers the creation of objects of the desired type with the preset
      * parallelization mode.
      *
      * @return An object of the desired algorithm type
      */
    std::shared_ptr<oa_type> get_() override {
        // Retrieve a work item using the methods implemented in our parent class
        std::shared_ptr<oa_type> p_alg = Gem::Common::GFactoryT<oa_type>::get_();

        // If we have been given a factory function for individuals, fill the object with data
        if(contentCreatorPtr_) { // Has a content creation object been registered ? If so, add individuals to the population
            for(std::size_t ind = 0; ind < p_alg->getDefaultPopulationSize(); ind++) {
                std::shared_ptr<GParameterSet> p_ind = (*contentCreatorPtr_)();
                if(not p_ind) { // No valid item received, the factory has run empty
                    break;
                }
                else {
                    p_alg->push_back(p_ind);
                }
            }
        }

        // Check if any pluggable optimization monitor was registered. If so,
        // load it into the optimization algorithm
        if(pluggableOM_) {
            p_alg->registerPluggableOM(pluggableOM_);
        }

        // Return the filled object to the audience
        return p_alg;
    }

    /***************************************************************************/
    /**
	  * Allows to describe configuration options
	  *
	  * @param gpb A reference to the parser-builder
	  */
    void describeLocalOptions_(Gem::Common::GParserBuilder &gpb) override {
        /* nothing */
    }

    /***************************************************************************/
    /**
	  * Allows to act on the configuration options received from the configuration file or from the command line
	  */
    void postProcess_(std::shared_ptr<oa_type> &p) override {
        // Set local options

        // The maximum allowed number of iterations
        if(this->maxIterationsCLSet()) {
            p->oa_type::setMaxIteration(this->getMaxIterationCL());
        }

        // The maximum number of stalls until operation stops
        if(this->maxStallIterationsCLSet()) {
            p->oa_type::setMaxStallIteration(this->getMaxStallIterationCL());
        }

        // The maximum amount of time until operation stops
        if(this->maxSecondsCLSet()) {
            p->oa_type::setMaxTime(this->getMaxTimeCL());
        }
    }

    /***************************************************************************/

    std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>>
        contentCreatorPtr_; ///< Holds an object capable of producing objects of the desired type
    std::shared_ptr<GBasePluggableOM>
        pluggableOM_; // A user-defined means for information retrieval

private:
    /***************************************************************************/
    /** @brief Creates individuals of this type */
    std::shared_ptr<oa_type>
    getObject_(Gem::Common::GParserBuilder &, const std::size_t &) override = 0;

    /***************************************************************************/

    std::int32_t maxIterationCL_ =
        -1; ///< The maximum number of iterations. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
    std::int32_t maxStallIterationCL_ =
        -1; ///< The maximum number of generations without improvement, after which optimization is stopped. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
    std::int32_t maxSecondsCL_ =
        -1; ///< The maximum number of seconds for the optimization to run. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

// Phase-2b compile shim — removed in NS Phase 3 (reference migration).
namespace Gem::Geneva {
using OptimizationAlgorithms::G_OptimizationAlgorithm_FactoryT;
} // namespace Gem::Geneva
