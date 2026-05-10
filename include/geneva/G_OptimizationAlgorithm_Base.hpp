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
#include <ctime>
#include <iostream>
#include <type_traits>
#include <utility>

// Boost header files go here

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "common/GPtrVectorT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GParameterSetFixedSizePriorityQueue.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/G_Interface_OptimizerT.hpp"
#include "geneva/GenevaHelperFunctions.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/*
 * This is a collection of simple pluggable modules suitable for emitting certain specialized
 * information from within optimization algorithms. They can be plugged into G_OptimizationAlgorithm_Base
 * derivatives. A requirement is that they implement a private function "informationFunction_"
 * according to the API of GBasePluggableOM .
 */

// Forward declaration
class G_OptimizationAlgorithm_Base;

/*******************************************************************************/
/////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************/
/**
 * The base class of all pluggable optimization monitors
 */
class GBasePluggableOM : public GObject {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject) &
            BOOST_SERIALIZATION_NVP(useRawEvaluation_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GBasePluggableOM() = default;
    GBasePluggableOM(GBasePluggableOM const &cp) = default;
    GBasePluggableOM(GBasePluggableOM &&cp) = default;

    ~GBasePluggableOM() override = default;

    GBasePluggableOM &operator=(GBasePluggableOM const &) = default;
    GBasePluggableOM &operator=(GBasePluggableOM &&) = default;

    /***************************************************************************/
    /** @brief Access tp information about the current iteration */
    void informationFunction(infoMode, G_OptimizationAlgorithm_Base const *const);

    /** @brief Allows to set the useRawEvaluation_ variable */
    void setUseRawEvaluation(bool useRaw);

    /** @brief Allows to retrieve the value of the useRawEvaluation_ variable */
    bool getUseRawEvaluation() const;

protected:
    /************************************************************************/
    /** @brief Loads the data of another object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBasePluggableOM>(
        GBasePluggableOM const &,
        GBasePluggableOM const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes
 */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/
    // Data

    bool useRawEvaluation_ =
        false; ///< Specifies whether the true (unmodified) evaluation should be used

private:
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override = 0;

    /** @brief Overload this function in derived classes, specifying actions for initialization, the optimization cycles and finalization. */
    virtual void
    informationFunction_(infoMode, G_OptimizationAlgorithm_Base const *const) = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements basic operations found in iteration-based optimization algorithms.
 * E.g., one might want to stop the optimization after a given number of cycles, or after
 * a given amount of time. The class also defines the interface functions common to these
 * algorithms, such as a general call to "optimize()".
 */
class G_OptimizationAlgorithm_Base // NOLINT(cppcoreguidelines-special-member-functions)
  : public GObject
  , public Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>
  , public G_Interface_OptimizerT<G_OptimizationAlgorithm_Base> {
private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void load(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        std::string cpDir{};

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject) &
            make_nvp(
                "GStdPtrVectorInterfaceT_T",
                boost::serialization::base_object<
                    Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>>(*this)
            ) &
            BOOST_SERIALIZATION_NVP(iteration_) & BOOST_SERIALIZATION_NVP(offset_) &
            BOOST_SERIALIZATION_NVP(maxIteration_) & BOOST_SERIALIZATION_NVP(minIteration_) &
            BOOST_SERIALIZATION_NVP(maxStallIteration_) &
            BOOST_SERIALIZATION_NVP(reportIteration_) &
            BOOST_SERIALIZATION_NVP(nRecordbestGlobalIndividuals_) &
            BOOST_SERIALIZATION_NVP(bestGlobalIndividuals_pq_) &
            BOOST_SERIALIZATION_NVP(defaultPopulationSize_) &
            BOOST_SERIALIZATION_NVP(bestKnownPrimaryFitness_) &
            BOOST_SERIALIZATION_NVP(bestCurrentPrimaryFitness_) &
            BOOST_SERIALIZATION_NVP(stallCounter_) &
            BOOST_SERIALIZATION_NVP(stallCounterThreshold_) &
            BOOST_SERIALIZATION_NVP(cp_interval_) & BOOST_SERIALIZATION_NVP(cp_base_name_) &
            BOOST_SERIALIZATION_NVP(cpDir) & BOOST_SERIALIZATION_NVP(cp_last_) &
            BOOST_SERIALIZATION_NVP(cp_remove_) &
            BOOST_SERIALIZATION_NVP(cp_serialization_mode_) &
            BOOST_SERIALIZATION_NVP(qualityThreshold_) &
            BOOST_SERIALIZATION_NVP(hasQualityThreshold_) &
            BOOST_SERIALIZATION_NVP(maxDuration_) & BOOST_SERIALIZATION_NVP(minDuration_) &
            BOOST_SERIALIZATION_NVP(terminationFile_) &
            BOOST_SERIALIZATION_NVP(terminateOnFileModification_) &
            BOOST_SERIALIZATION_NVP(emitTerminationReason_) & BOOST_SERIALIZATION_NVP(halted_) &
            BOOST_SERIALIZATION_NVP(worstKnownValids_cnt_) &
            BOOST_SERIALIZATION_NVP(pluggable_monitors_cnt_) &
            BOOST_SERIALIZATION_NVP(executor_ptr_) & BOOST_SERIALIZATION_NVP(default_execMode_) &
            BOOST_SERIALIZATION_NVP(default_executor_config_);

        // Transfer the string to the path
        cp_directory_path_ = std::filesystem::path(cpDir);
    }

    template <typename Archive>
    void save(Archive &ar, const unsigned int) const {
        using boost::serialization::make_nvp;

        // Transfer the path to the string
        std::string cpDir =
            cp_directory_path_.string(); // NOLINT(cppcoreguidelines-init-variables)

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject) &
            make_nvp(
                "GStdPtrVectorInterfaceT_T",
                boost::serialization::base_object<
                    Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>>(*this)
            ) &
            BOOST_SERIALIZATION_NVP(iteration_) & BOOST_SERIALIZATION_NVP(offset_) &
            BOOST_SERIALIZATION_NVP(maxIteration_) & BOOST_SERIALIZATION_NVP(minIteration_) &
            BOOST_SERIALIZATION_NVP(maxStallIteration_) &
            BOOST_SERIALIZATION_NVP(reportIteration_) &
            BOOST_SERIALIZATION_NVP(nRecordbestGlobalIndividuals_) &
            BOOST_SERIALIZATION_NVP(bestGlobalIndividuals_pq_) &
            BOOST_SERIALIZATION_NVP(defaultPopulationSize_) &
            BOOST_SERIALIZATION_NVP(bestKnownPrimaryFitness_) &
            BOOST_SERIALIZATION_NVP(bestCurrentPrimaryFitness_) &
            BOOST_SERIALIZATION_NVP(stallCounter_) &
            BOOST_SERIALIZATION_NVP(stallCounterThreshold_) &
            BOOST_SERIALIZATION_NVP(cp_interval_) & BOOST_SERIALIZATION_NVP(cp_base_name_) &
            BOOST_SERIALIZATION_NVP(cpDir) & BOOST_SERIALIZATION_NVP(cp_last_) &
            BOOST_SERIALIZATION_NVP(cp_remove_) &
            BOOST_SERIALIZATION_NVP(cp_serialization_mode_) &
            BOOST_SERIALIZATION_NVP(qualityThreshold_) &
            BOOST_SERIALIZATION_NVP(hasQualityThreshold_) &
            BOOST_SERIALIZATION_NVP(maxDuration_) & BOOST_SERIALIZATION_NVP(minDuration_) &
            BOOST_SERIALIZATION_NVP(terminationFile_) &
            BOOST_SERIALIZATION_NVP(terminateOnFileModification_) &
            BOOST_SERIALIZATION_NVP(emitTerminationReason_) & BOOST_SERIALIZATION_NVP(halted_) &
            BOOST_SERIALIZATION_NVP(worstKnownValids_cnt_) &
            BOOST_SERIALIZATION_NVP(pluggable_monitors_cnt_) &
            BOOST_SERIALIZATION_NVP(executor_ptr_) & BOOST_SERIALIZATION_NVP(default_execMode_) &
            BOOST_SERIALIZATION_NVP(default_executor_config_);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The copy constructor */
    G_OptimizationAlgorithm_Base(G_OptimizationAlgorithm_Base const &cp);

    /***************************************************************************/
    // Defaulted functions

    G_OptimizationAlgorithm_Base() = default;
    ~G_OptimizationAlgorithm_Base() override = default;

    /***************************************************************************/

    /** @brief Performs the necessary administratory work of doing check-pointing */
    void checkpoint(bool is_better) const;

    /** @brief Loads the state of the class from disc */
    void loadCheckpoint(std::filesystem::path const &cpFile);

    /** @brief Checks whether the optimization process has been halted */
    bool halted() const;

    /** @brief Allows to set the number of generations after which a checkpoint should be written */
    void setCheckpointInterval(std::int32_t cpInterval);
    /** @brief Allows to retrieve the number of generations after which a checkpoint should be written */
    std::int32_t getCheckpointInterval() const;

    /** @brief Allows to set the base name of the checkpoint file and the directory where it should be stored. */
    void setCheckpointBaseName(std::string cpDirectory, std::string cpBaseName);
    /** @brief Allows to retrieve the base name of the checkpoint file. */
    std::string getCheckpointBaseName() const;
    /** @brief Allows to retrieve the directory where checkpoint files should be stored */
    std::string getCheckpointDirectory() const;
    /** @brief Allows to retrieve the directory where checkpoint files should be stored */
    std::filesystem::path getCheckpointDirectoryPath() const;
    /** @brief Determines whether checkpointing should be done in Text-, XML- or Binary-mode */
    void setCheckpointSerializationMode(Gem::Common::serializationMode cpSerMode);
    /** @brief Retrieves the current checkpointing serialization mode */
    Gem::Common::serializationMode getCheckpointSerializationMode() const;
    /** @brief Allows to set the cp_overwrite_ flag */
    void setRemoveCheckpointFiles(bool cp_remove);
    /** @brief Allows to check whether checkpoint files will be removed */
    bool checkpointFilesAreRemoved() const;

    /** @brief Resets the class to the state before the optimize call. */
    void resetToOptimizationStart();

    /** @brief Adds a new executor to the class, replacing the default executor */
    void registerExecutor(
        std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> executor_ptr,
        std::filesystem::path const &executorConfigFile
    );
    /** @brief Adds a new executor to the class, using the chosen execution mode */
    void registerExecutor(execMode e, std::filesystem::path const &executorConfigFile);

    /******************************************************************************/
    /**
      * Gives access to the current executor, converted to a given target type.
      * The executor is internally stored via its base class, so we need to
      * convert it to its final type in order to configure it via its API. The
      * function is only accessible when converting to a derived class of GBaseExecutorT.
      * You need to take care yourself that the stored class matches the one you
      * are converting to. The function will throw (via dynamic_pointer_cast), if
      * this is not the case.
      */
    template <typename target_type>
    std::shared_ptr<target_type> getExecutor(
        typename std::enable_if<
            std::is_base_of<Gem::Courtier::GBaseExecutorT<GParameterSet>, target_type>::value>::type
            *dummy = nullptr
    ) {
        return std::dynamic_pointer_cast<target_type>(executor_ptr_);
    }

    /******************************************************************************/

    /** @brief Emits information specific to this class */
    void informationUpdate(const infoMode &im);

    /** @brief Checks whether a better solution was found. */
    bool progress() const;

    /** @brief Allows to register a pluggable optimization monitor. */
    void registerPluggableOM(std::shared_ptr<GBasePluggableOM> pluggableOM);
    /** @brief Allows to reset the local pluggable optimization monitors */
    void resetPluggableOM();
    /** @brief Allows to check whether pluggable optimization monitors were registered */
    bool hasPluggableOptimizationMonitors() const;

    /** @brief Retrieves the default population size */
    std::size_t getDefaultPopulationSize() const;
    /** @brief Retrieve the current population size */
    std::size_t getPopulationSize() const;

    /** @brief Set the number of iterations after which the optimization should be stopped */
    void setMaxIteration(std::uint32_t maxIteration);
    /** @brief Retrieve the number of iterations after which optimization should be stopped */
    std::uint32_t getMaxIteration() const;

    /** @brief Sets the minimum number of iterations */
    void setMinIteration(std::uint32_t minIteration);
    /** @brief Retrieves the currently set minimum number of iterations */
    std::uint32_t getMinIteration() const;

    /** @brief Sets the maximum number of iterations allowed without improvement of the best individual */
    void setMaxStallIteration(std::uint32_t maxStallIteration);
    /** @brief Retrieves the maximum number of generations allowed in an optimization run without improvement of the best individual. */
    std::uint32_t getMaxStallIteration() const;

    /** @brief Sets the maximum allowed processing time */
    void setMaxTime(std::chrono::duration<double> maxDuration);
    /** @brief Retrieves the value of the maxDuration_ parameter. */
    std::chrono::duration<double> getMaxTime() const;

    /** @brief Sets the minimum required processing time */
    void setMinTime(std::chrono::duration<double> minDuration);
    /** @brief Retrieves the value of the minDuration_ parameter */
    std::chrono::duration<double> getMinTime() const;

    /** @brief Sets a quality threshold beyond which optimization is expected to stop */
    void setQualityThreshold(double qualityThreshold, bool hasQualityThreshold);
    /** @brief Retrieves the current value of the quality threshold and also indicates whether the threshold is active */
    double getQualityThreshold(bool &hasQualityThreshold) const;

    /** @brief Sets the name of a "termination file" */
    void
    setTerminationFile(std::string terminationFile, bool terminateOnFileModification);
    /** @brief Retrieves the current name of the termination file and also indicates whether the "touched halt" is active */
    std::string getTerminationFile(bool &terminateOnFileModification) const;

    /** @brief Removes the quality threshold */
    void resetQualityThreshold();
    /** @brief Checks whether a quality threshold has been set */
    bool hasQualityThreshold() const;

    /** @brief Returns the current offset used to calculate the current iteration */
    std::uint32_t getStartIteration() const;

    /** @brief Sets the number of iterations after which the algorithm should report about its inner state. */
    void setReportIteration(std::uint32_t iter);
    /** @brief Returns the number of iterations after which the algorithm should report about its inner state. */
    std::uint32_t getReportIteration() const;

    /** @brief Retrieves the current number of failed optimization attempts */
    std::uint32_t getStallCounter() const;

    /** @brief Allows to set the number of iterations without improvement, after which individuals are asked to update their internal data structures */
    void setStallCounterThreshold(std::uint32_t stallCounterThreshold);
    /** @brief Allows to retrieve the number of iterations without improvement, after which individuals are asked to update their internal data structures */
    std::uint32_t getStallCounterThreshold() const;

    /** @brief Retrieve the best value found in the entire optimization run so far */
    std::tuple<double, double> getBestKnownPrimaryFitness() const;
    /** @brief Retrieves the best value found in the current iteration */
    std::tuple<double, double> getBestCurrentPrimaryFitness() const;

    /** @brief Specifies whether information about termination reasons should be emitted */
    void setEmitTerminationReason(bool emitTerminatioReason = true);
    /** @brief Retrieves information on whether information about termination reasons should be emitted */
    bool getEmitTerminationReason() const;

    /******************************************************************************/
    /**
     * This function converts an individual at a given position to the derived
     * type and returns it. In DEBUG mode, the function will check whether the
     * requested position exists.
     *
     * @param pos The position in our data array that shall be converted
     * @return A converted version of the GParameterSet object, as required by the user
     */
    template <typename target_type>
    std::shared_ptr<target_type> individual_cast(std::size_t pos) const {
#ifdef DEBUG
        if(pos >= this->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In G_OptimizationAlgorithm_Base::individual_cast<>() : Error" << std::endl
                << "Tried to access position " << pos << " which is >= array size " << this->size()
                << std::endl
            );
        }
#endif /* DEBUG */

        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<GParameterSet, target_type>(this->at(pos));
    }

    /***************************************************************************/

    /** @brief Retrieve the number of processable items in the current iteration. */
    std::size_t getNProcessableItems() const;

    /** @brief If individuals have been stored in this population, they are added to the priority queue. */
    void addCleanStoredBests(GParameterSetFixedSizePriorityQueue &bestIndividuals);

    /** @brief Helper function that determines whether we are currently inside of the first iteration */
    bool inFirstIteration() const;
    /** @brief Helper function that determines whether we are after the first iteration */
    bool afterFirstIteration() const;

    /** @brief Checks whether a checkpoint-file has the same "personality" as our own algorithm */
    bool cp_personality_fits(const std::filesystem::path &p) const;

protected:
    /***************************************************************************/
    // Some data

    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>
        gr_; ///< A random number generator. Note that the actual calculation is done in a random number proxy / factory
    std::uniform_real_distribution<double>
        uniform_real_distribution_; ///< Access to uniformly distributed double random values

    /***************************************************************************/
    // Overridden or virtual protected functions

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;
    /** @brief Loads the data of another GOptimizationAlgorithm object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<G_OptimizationAlgorithm_Base>(
        G_OptimizationAlgorithm_Base const &,
        G_OptimizationAlgorithm_Base const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Resets the class to the state before the optimize call. */
    virtual void resetToOptimizationStart_();

    /** @brief Initialization code to be run before the optimization cycle */
    virtual void init();
    /** @brief Finalization code to be run after the optimization cycle */
    virtual void finalize();

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

    /** @brief Delegation of work to be performed to the private executor object */
    Gem::Courtier::executor_status_t workOn(
        std::vector<std::shared_ptr<GParameterSet>> &workItems,
        bool resubmitUnprocessed = false,
        const std::string &caller = std::string()
    );
    /** @brief Retrieves a vector of old work items after job submission */
    std::vector<std::shared_ptr<GParameterSet>> getOldWorkItems();

    /** @brief Saves the state of the class to disc */
    void saveCheckpoint(std::filesystem::path const &outputFile) const;

    /** @brief Extracts the short name of the optimization algorithm */
    std::string extractOptAlgFromPath(const std::filesystem::path &p) const;

    /** @brief Allows to set the personality type of the individuals */
    void setIndividualPersonalities();
    /** @brief Resets the individual's personality types */
    void resetIndividualPersonalities();

    /** @brief Sets the default size of the population */
    void setDefaultPopulationSize(std::size_t defPopSize);

    // NB: protected, as a derived function may fall back to this function, cmp EA in non-pareto mode
    /** @brief Adds the individuals of this iteration to a priority queue. */
    virtual void
    updateGlobalBestsPQ_(GParameterSetFixedSizePriorityQueue &bestIndividuals);
    /** @brief Adds the individuals of this iteration to a priority queue. */
    virtual void
    updateIterationBestsPQ_(GParameterSetFixedSizePriorityQueue &bestIndividuals);

    /** @brief Set the number of "best" individuals to be recorded in each iteration */
    void setNRecordBestIndividuals(std::size_t nRecordBestIndividuals);
    /** @brief Retrieve the number of best individuals to be recorded in each iteration */
    std::size_t getNRecordBestIndividuals() const;

    /** @brief Allows derived classes to reset the stall counter. */
    void resetStallCounter();

    /** @brief Lets individuals know about the current iteration of the optimization cycle. */
    void markIteration();

    /** @brief Let individuals know the number of stalls encountered so far */
    void markNStalls();

private:
    /***************************************************************************/
    // Overloaded or virtual base functions

    /** @brief This function encapsulates some common functionality of iteration-based optimization algorithms. */
    G_OptimizationAlgorithm_Base const *optimize_(std::uint32_t offset) final;
    /** @brief Emits a name for this class / object; this can be a long name with spaces */
    std::string name_() const override = 0;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override = 0;

    /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
    void runFitnessCalculation_() override = 0;
    /** @brief The actual business logic to be performed during each iteration */
    virtual std::tuple<double, double> cycleLogic_() = 0;

    /** @brief Retrieve the current iteration of the optimization run */
    std::uint32_t getIteration_() const override;

    /** @brief Retrieves the best individual found up to now */
    std::shared_ptr<GParameterSet> getBestGlobalIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found */
    std::vector<std::shared_ptr<GParameterSet>>
    getBestGlobalIndividuals_() const final;

    /** @brief Retrieves the best individual found in the iteration */
    std::shared_ptr<GParameterSet> getBestIterationIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found in the */
    std::vector<std::shared_ptr<GParameterSet>>
    getBestIterationIndividuals_() const final;

    /** @brief Retrieve the number of processable items in the current iteration. */
    virtual std::size_t getNProcessableItems_() const;

    /** @brief Retrieve a personality trait object belonging to this algorithm */
    virtual std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const = 0;

    /** @brief Resizes the population to the desired level and does some error checks */
    virtual void adjustPopulation_() = 0;

    /** @brief Gives derived classes an opportunity to update their internal structures. */
    virtual void actOnStalls_() = 0;

    /***************************************************************************/

    /** @brief Update the stall counter. */
    void updateStallCounter(const std::tuple<double, double> &bestEval);

    /** @brief This function returns true once a given time has passed */
    bool timedHalt(const std::chrono::system_clock::time_point &currentTime) const;
    /** @brief This function checks whether a minimum amount of time has passed */
    bool minTimePassed(const std::chrono::system_clock::time_point &currentTime) const;

    /** @brief This function returns true once the quality has passed a given threshold */
    bool qualityHalt() const;

    /** @brief This function returns true once a given number of stalls has been exceeded in a row */
    bool stallHalt() const;

    /** @brief This function returns true once a maximum number of iterations has been exceeded */
    bool iterationHalt() const;
    /** @brief This function returns true when the minimum number of iterations has been passed. */
    bool minIterationPassed() const;

    /** @brief This function returns true if a SIGHUP / CTRL_CLOSE_EVENT signal was sent */
    bool sigHupHalt() const;

    /** @brief Triggers termination of the optimization run, when a file with a user-defined file is modified */
    bool touchHalt() const;

    /** @brief A wrapper for customHalt_ that allows to emit the termination reason */
    bool customHalt() const;
    /** @brief Custom setting of halt criteria */
    virtual bool customHalt_() const;

    /** @brief This function checks whether a halt criterion has been reached. */
    bool halt() const;

    /** @brief Check whether the max-iteration halt is set */
    bool maxIterationHaltset() const;
    /** @brief Check whether a halt criterion based on the number of stalls has been set */
    bool stallHaltSet() const;

    /** @brief Check whether the maxDuration-halt criterion has been set */
    bool maxDurationHaltSet() const;

    /** @brief Check whether the quality-threshold halt-criterion has been set */
    bool qualityThresholdHaltSet() const;
    /** @brief Marks the globally best known fitness in all individuals */
    void markBestFitness();

    /** @brief Indicates whether the stallCounterThreshold_ has been exceeded */
    bool stallCounterThresholdExceeded() const;

    /** @brief Retrieves an executor for the given execution mode */
    std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>
    createExecutor(const execMode &e);

    /***************************************************************************/
    // Data

    std::uint32_t iteration_ = 0; ///< The current iteration
    std::uint32_t offset_ =
        DEFAULTOFFSET; ///< An iteration offset which can be used, if the optimization starts from a checkpoint file
    std::uint32_t minIteration_ = DEFAULTMINIT; ///< The minimum number of iterations
    std::uint32_t maxIteration_ = DEFAULTMAXIT; ///< The maximum number of iterations
    std::uint32_t maxStallIteration_ =
        DEFAULTMAXSTALLIT; ///< The maximum number of generations without improvement, after which optimization is stopped
    std::uint32_t reportIteration_ =
        DEFAULTREPORTITER; ///< The number of generations after which a report should be issued

    std::size_t nRecordbestGlobalIndividuals_ =
        DEFNRECORDBESTINDIVIDUALS; ///< Indicates the number of best individuals to be recorded/updated in each iteration
    GParameterSetFixedSizePriorityQueue bestGlobalIndividuals_pq_{
        nRecordbestGlobalIndividuals_
    }; ///< A priority queue with the best individuals found so far
    GParameterSetFixedSizePriorityQueue bestIterationIndividuals_pq_{
        nRecordbestGlobalIndividuals_
    }; ///< A priority queue with the best individuals of a given iteration; unlimited size so all individuals of an iteration fit in

    std::size_t defaultPopulationSize_ =
        DEFAULTPOPULATIONSIZE; ///< The nominal size of the population
    std::tuple<double, double> bestKnownPrimaryFitness_ =
        std::tuple<double, double>(0., 0.); ///< Records the best primary fitness found so far
    std::tuple<double, double> bestCurrentPrimaryFitness_ = std::tuple<double, double>(
        0.,
        0.
    ); ///< Records the best fitness found in the current iteration

    std::uint32_t stallCounter_ = 0; ///< Counts the number of iterations without improvement
    std::uint32_t stallCounterThreshold_ =
        DEFAULTSTALLCOUNTERTHRESHOLD; ///< The number of stalls after which individuals are asked to update their internal data structures

    std::int32_t cp_interval_ =
        DEFAULTCHECKPOINTIT; ///< Number of iterations after which a checkpoint should be written. -1 means: Write whenever an improvement was encountered
    std::string cp_base_name_ = DEFAULTCPBASENAME; ///< The base name of the checkpoint file
    std::filesystem::path cp_directory_path_ =
        std::filesystem::path(DEFAULTCPDIR); ///< Path object to store the directory
    mutable std::string cp_last_ = "empty"; ///< The name of the last saved checkpoint
    bool cp_remove_ = true; ///< Whether checkpoint files should be overwritten or kept
    Gem::Common::serializationMode cp_serialization_mode_ =
        DEFAULTCPSERMODE; ///< Determines whether check-pointing should be done in text-, XML, or binary mode
    double qualityThreshold_ =
        DEFAULTQUALITYTHRESHOLD; ///< A threshold beyond which optimization is expected to stop
    bool hasQualityThreshold_ = false; ///< Specifies whether a qualityThreshold has been set
    std::chrono::duration<double> maxDuration_ = Gem::Common::duration_from_string(
        DEFAULTDURATION
    ); ///< Maximum time-frame for the optimization
    std::chrono::duration<double> minDuration_ = Gem::Common::duration_from_string(
        DEFAULTMINDURATION
    ); ///< Minimum time-frame for the optimization
    mutable std::chrono::system_clock::time_point
        startTime_; ///< Used to store the start time of the optimization. Declared mutable so the halt criteria can be const
    mutable std::filesystem::file_time_type
        file_startTime_; ///< Used for the touchHalt-feature, as system_clock file_time may not be comparable
    std::string terminationFile_ =
        DEFAULTTERMINATIONFILE; ///< The name of a file which, when modified after the start of the optimization run, will cause termination of the run
    bool terminateOnFileModification_ = false;
    bool emitTerminationReason_ =
        DEFAULTEMITTERMINATIONREASON; ///< Specifies whether information about reasons for termination should be emitted
    std::atomic<bool> halted_{true}; ///< Set to true when halt() has returned "true"
    std::vector<std::tuple<double, double>>
        worstKnownValids_cnt_; ///< Stores the worst known valid evaluations up to the current iteration (first entry: raw, second: tranformed)
    std::vector<std::shared_ptr<GBasePluggableOM>>
        pluggable_monitors_cnt_; ///< A collection of monitors

    std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>
        executor_ptr_; ///< Holds the current executor for this algorithm
    execMode default_execMode_ = execMode::
        BROKER; ///< The default execution mode. Unless explicitöy requested by the user, we always go through the broker
    std::string default_executor_config_ =
        "./config/GBrokerExecutor.json"; ///< The default configuration file for the broker executor
};

/*******************************************************************************/
/////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************/

} /* namespace Gem::Geneva */

/******************************************************************************/
// Some serialization-related exports and declarations. Note that namespace
// specifiers are included in the macros, no need for an explicit namespace boost::serialization

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBasePluggableOM)                  // NOLINT
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::G_OptimizationAlgorithm_Base)      // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GBrokerExecutorT<Gem::Geneva::GParameterSet>) // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GSerialExecutorT<Gem::Geneva::GParameterSet>) // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GMTExecutorT<Gem::Geneva::GParameterSet>)     // NOLINT
/******************************************************************************/
