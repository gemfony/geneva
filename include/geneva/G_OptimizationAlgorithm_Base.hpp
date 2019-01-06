/**
 * @file
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <ctime>
#include <chrono>
#include <type_traits>
#include <utility>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

// Geneva headers go here
#include "common/GPtrVectorT.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/G_Interface_OptimizerT.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GParameterSetFixedSizePriorityQueue.hpp"
#include "geneva/GenevaHelperFunctions.hpp"

namespace Gem {
namespace Geneva {

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
class GBasePluggableOM :
    public GObject
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
        & BOOST_SERIALIZATION_NVP(m_useRawEvaluation);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    // Defaulted constructors, destructor and assignment operators

    G_API_GENEVA GBasePluggableOM() = default;
    G_API_GENEVA GBasePluggableOM(GBasePluggableOM const & cp) = default;
    G_API_GENEVA GBasePluggableOM(GBasePluggableOM && cp) = default;

    G_API_GENEVA ~GBasePluggableOM() override = default;

    G_API_GENEVA GBasePluggableOM& operator=(GBasePluggableOM const&) = default;
    G_API_GENEVA GBasePluggableOM& operator=(GBasePluggableOM &&) = default;

    /***************************************************************************/
    /** @brief Access tp information about the current iteration */
    G_API_GENEVA void informationFunction(
        infoMode
        , G_OptimizationAlgorithm_Base const * const
    );

    /** @brief Allows to set the m_useRawEvaluation variable */
    void G_API_GENEVA setUseRawEvaluation(bool useRaw);

    /** @brief Allows to retrieve the value of the m_useRawEvaluation variable */
    bool G_API_GENEVA getUseRawEvaluation() const;

protected:
    /************************************************************************/
    /** @brief Loads the data of another object */
    G_API_GENEVA void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBasePluggableOM>(
        GBasePluggableOM const &
        , GBasePluggableOM const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const GObject &cp
        , const Gem::Common::expectation &e
        , const double &limit
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes
 */
    G_API_GENEVA bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/
    // Data

    bool m_useRawEvaluation = false; ///< Specifies whether the true (unmodified) evaluation should be used

private:
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override = 0;

    /** @brief Overload this function in derived classes, specifying actions for initialization, the optimization cycles and finalization. */
    virtual G_API_GENEVA void informationFunction_(
        infoMode
        , G_OptimizationAlgorithm_Base const * const
    ) BASE = 0;
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
class G_OptimizationAlgorithm_Base
    : public GObject
    , public Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>
    , public G_Interface_OptimizerT<G_OptimizationAlgorithm_Base>
{
private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void load(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        std::string cpDir{};

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
        & make_nvp(
            "GStdPtrVectorInterfaceT_T"
            , boost::serialization::base_object<Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>>(*this))
        & BOOST_SERIALIZATION_NVP(m_iteration)
        & BOOST_SERIALIZATION_NVP(m_offset)
        & BOOST_SERIALIZATION_NVP(m_maxIteration)
        & BOOST_SERIALIZATION_NVP(m_minIteration)
        & BOOST_SERIALIZATION_NVP(m_maxStallIteration)
        & BOOST_SERIALIZATION_NVP(m_reportIteration)
        & BOOST_SERIALIZATION_NVP(m_nRecordbestGlobalIndividuals)
        & BOOST_SERIALIZATION_NVP(m_bestGlobalIndividuals_pq)
        & BOOST_SERIALIZATION_NVP(m_defaultPopulationSize)
        & BOOST_SERIALIZATION_NVP(m_bestKnownPrimaryFitness)
        & BOOST_SERIALIZATION_NVP(m_bestCurrentPrimaryFitness)
        & BOOST_SERIALIZATION_NVP(m_stallCounter)
        & BOOST_SERIALIZATION_NVP(m_stallCounterThreshold)
        & BOOST_SERIALIZATION_NVP(m_cp_interval)
        & BOOST_SERIALIZATION_NVP(m_cp_base_name)
        & BOOST_SERIALIZATION_NVP(cpDir)
        & BOOST_SERIALIZATION_NVP(m_cp_last)
        & BOOST_SERIALIZATION_NVP(m_cp_remove)
        & BOOST_SERIALIZATION_NVP(m_cp_serialization_mode)
        & BOOST_SERIALIZATION_NVP(m_qualityThreshold)
        & BOOST_SERIALIZATION_NVP(m_hasQualityThreshold)
        & BOOST_SERIALIZATION_NVP(m_maxDuration)
        & BOOST_SERIALIZATION_NVP(m_minDuration)
        & BOOST_SERIALIZATION_NVP(m_terminationFile)
        & BOOST_SERIALIZATION_NVP(m_terminateOnFileModification)
        & BOOST_SERIALIZATION_NVP(m_emitTerminationReason)
        & BOOST_SERIALIZATION_NVP(m_halted)
        & BOOST_SERIALIZATION_NVP(m_worstKnownValids_cnt)
        & BOOST_SERIALIZATION_NVP(m_pluggable_monitors_cnt)
        & BOOST_SERIALIZATION_NVP(m_executor_ptr)
        & BOOST_SERIALIZATION_NVP(m_default_execMode)
        & BOOST_SERIALIZATION_NVP(m_default_executor_config);

        // Transfer the string to the path
        m_cp_directory_path = boost::filesystem::path(cpDir);
    }

    template<typename Archive>
    void save(Archive &ar, const unsigned int) const {
        using boost::serialization::make_nvp;

        // Transfer the path to the string
        std::string cpDir = m_cp_directory_path.string();

        ar
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
        & make_nvp(
            "GStdPtrVectorInterfaceT_T"
            , boost::serialization::base_object<Gem::Common::GPtrVectorT<GParameterSet, Gem::Geneva::GObject>>(*this))
        & BOOST_SERIALIZATION_NVP(m_iteration)
        & BOOST_SERIALIZATION_NVP(m_offset)
        & BOOST_SERIALIZATION_NVP(m_maxIteration)
        & BOOST_SERIALIZATION_NVP(m_minIteration)
        & BOOST_SERIALIZATION_NVP(m_maxStallIteration)
        & BOOST_SERIALIZATION_NVP(m_reportIteration)
        & BOOST_SERIALIZATION_NVP(m_nRecordbestGlobalIndividuals)
        & BOOST_SERIALIZATION_NVP(m_bestGlobalIndividuals_pq)
        & BOOST_SERIALIZATION_NVP(m_defaultPopulationSize)
        & BOOST_SERIALIZATION_NVP(m_bestKnownPrimaryFitness)
        & BOOST_SERIALIZATION_NVP(m_bestCurrentPrimaryFitness)
        & BOOST_SERIALIZATION_NVP(m_stallCounter)
        & BOOST_SERIALIZATION_NVP(m_stallCounterThreshold)
        & BOOST_SERIALIZATION_NVP(m_cp_interval)
        & BOOST_SERIALIZATION_NVP(m_cp_base_name)
        & BOOST_SERIALIZATION_NVP(cpDir)
        & BOOST_SERIALIZATION_NVP(m_cp_last)
        & BOOST_SERIALIZATION_NVP(m_cp_remove)
        & BOOST_SERIALIZATION_NVP(m_cp_serialization_mode)
        & BOOST_SERIALIZATION_NVP(m_qualityThreshold)
        & BOOST_SERIALIZATION_NVP(m_hasQualityThreshold)
        & BOOST_SERIALIZATION_NVP(m_maxDuration)
        & BOOST_SERIALIZATION_NVP(m_minDuration)
        & BOOST_SERIALIZATION_NVP(m_terminationFile)
        & BOOST_SERIALIZATION_NVP(m_terminateOnFileModification)
        & BOOST_SERIALIZATION_NVP(m_emitTerminationReason)
        & BOOST_SERIALIZATION_NVP(m_halted)
        & BOOST_SERIALIZATION_NVP(m_worstKnownValids_cnt)
        & BOOST_SERIALIZATION_NVP(m_pluggable_monitors_cnt)
        & BOOST_SERIALIZATION_NVP(m_executor_ptr)
        & BOOST_SERIALIZATION_NVP(m_default_execMode)
        & BOOST_SERIALIZATION_NVP(m_default_executor_config);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The copy constructor */
    G_API_GENEVA G_OptimizationAlgorithm_Base(G_OptimizationAlgorithm_Base const & cp);

    /***************************************************************************/
    // Defaulted functions

    G_API_GENEVA G_OptimizationAlgorithm_Base() = default;
    G_API_GENEVA ~G_OptimizationAlgorithm_Base() override = default;

    /***************************************************************************/

    /** @brief Performs the necessary administratory work of doing check-pointing */
    G_API_GENEVA void checkpoint(bool is_better) const;

    /** @brief Loads the state of the class from disc */
    G_API_GENEVA void loadCheckpoint(bf::path const & cpFile);

    /** @brief Checks whether the optimization process has been halted */
    G_API_GENEVA bool halted() const;

    /** @brief Allows to set the number of generations after which a checkpoint should be written */
    G_API_GENEVA void setCheckpointInterval(std::int32_t cpInterval);
    /** @brief Allows to retrieve the number of generations after which a checkpoint should be written */
    G_API_GENEVA std::int32_t getCheckpointInterval() const;

    /** @brief Allows to set the base name of the checkpoint file and the directory where it should be stored. */
    G_API_GENEVA void setCheckpointBaseName(std::string cpDirectory, std::string cpBaseName);
    /** @brief Allows to retrieve the base name of the checkpoint file. */
    G_API_GENEVA std::string getCheckpointBaseName() const;
    /** @brief Allows to retrieve the directory where checkpoint files should be stored */
    G_API_GENEVA std::string getCheckpointDirectory() const;
    /** @brief Allows to retrieve the directory where checkpoint files should be stored */
    G_API_GENEVA bf::path getCheckpointDirectoryPath() const;
    /** @brief Determines whether checkpointing should be done in Text-, XML- or Binary-mode */
    G_API_GENEVA void setCheckpointSerializationMode(Gem::Common::serializationMode cpSerMode);
    /** @brief Retrieves the current checkpointing serialization mode */
    G_API_GENEVA Gem::Common::serializationMode getCheckpointSerializationMode() const;
    /** @brief Allows to set the m_cp_overwrite flag */
    G_API_GENEVA void setRemoveCheckpointFiles(bool cp_remove);
    /** @brief Allows to check whether checkpoint files will be removed */
    G_API_GENEVA bool checkpointFilesAreRemoved() const;

    /** @brief Resets the class to the state before the optimize call. */
    G_API_GENEVA void resetToOptimizationStart();

    /** @brief Adds a new executor to the class, replacing the default executor */
    G_API_GENEVA void registerExecutor(
        std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> executor_ptr
        , boost::filesystem::path const &executorConfigFile
    );
    /** @brief Adds a new executor to the class, using the chosen execution mode */
    G_API_GENEVA void registerExecutor(
        execMode e
        , boost::filesystem::path const &executorConfigFile
    );

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
    template<typename target_type>
    std::shared_ptr<target_type> getExecutor(
        typename std::enable_if<std::is_base_of<Gem::Courtier::GBaseExecutorT<GParameterSet>, target_type>::value>::type *dummy = nullptr
    ) {
        return std::dynamic_pointer_cast<target_type>(m_executor_ptr);
    }

    /******************************************************************************/

    /** @brief Emits information specific to this class */
    G_API_GENEVA void informationUpdate(const infoMode &im);

    /** @brief Checks whether a better solution was found. */
    G_API_GENEVA bool progress() const;

    /** @brief Allows to register a pluggable optimization monitor. */
    G_API_GENEVA void registerPluggableOM(
        std::shared_ptr<GBasePluggableOM> pluggableOM
    );
    /** @brief Allows to reset the local pluggable optimization monitors */
    G_API_GENEVA void resetPluggableOM();
    /** @brief Allows to check whether pluggable optimization monitors were registered */
    G_API_GENEVA bool hasPluggableOptimizationMonitors() const;

    /** @brief Retrieves the default population size */
    G_API_GENEVA std::size_t getDefaultPopulationSize() const;
    /** @brief Retrieve the current population size */
    G_API_GENEVA std::size_t getPopulationSize() const;

    /** @brief Set the number of iterations after which the optimization should be stopped */
    G_API_GENEVA void setMaxIteration(std::uint32_t maxIteration);
    /** @brief Retrieve the number of iterations after which optimization should be stopped */
    G_API_GENEVA std::uint32_t getMaxIteration() const;

    /** @brief Sets the minimum number of iterations */
    G_API_GENEVA void setMinIteration(std::uint32_t minIteration);
    /** @brief Retrieves the currently set minimum number of iterations */
    G_API_GENEVA std::uint32_t getMinIteration() const;

    /** @brief Sets the maximum number of iterations allowed without improvement of the best individual */
    G_API_GENEVA void setMaxStallIteration(std::uint32_t maxStallIteration);
    /** @brief Retrieves the maximum number of generations allowed in an optimization run without improvement of the best individual. */
    G_API_GENEVA std::uint32_t getMaxStallIteration() const;

    /** @brief Sets the maximum allowed processing time */
    G_API_GENEVA void setMaxTime(std::chrono::duration<double> maxDuration);
    /** @brief Retrieves the value of the maxDuration_ parameter. */
    G_API_GENEVA std::chrono::duration<double> getMaxTime() const;

    /** @brief Sets the minimum required processing time */
    G_API_GENEVA void setMinTime(std::chrono::duration<double> minDuration);
    /** @brief Retrieves the value of the minDuration_ parameter */
    G_API_GENEVA std::chrono::duration<double> getMinTime() const;

    /** @brief Sets a quality threshold beyond which optimization is expected to stop */
    G_API_GENEVA void setQualityThreshold(double qualityThreshold, bool hasQualityThreshold);
    /** @brief Retrieves the current value of the quality threshold and also indicates whether the threshold is active */
    G_API_GENEVA double getQualityThreshold(bool &hasQualityThreshold) const;

    /** @brief Sets the name of a "termination file" */
    G_API_GENEVA void setTerminationFile(std::string terminationFile, bool terminateOnFileModification);
    /** @brief Retrieves the current name of the termination file and also indicates whether the "touched halt" is active */
    G_API_GENEVA std::string getTerminationFile(bool &terminateOnFileModification) const;

    /** @brief Removes the quality threshold */
    G_API_GENEVA void resetQualityThreshold();
    /** @brief Checks whether a quality threshold has been set */
    G_API_GENEVA bool hasQualityThreshold() const;

    /** @brief Returns the current offset used to calculate the current iteration */
    G_API_GENEVA std::uint32_t getStartIteration() const;

    /** @brief Sets the number of iterations after which the algorithm should report about its inner state. */
    G_API_GENEVA void setReportIteration(std::uint32_t iter);
    /** @brief Returns the number of iterations after which the algorithm should report about its inner state. */
    G_API_GENEVA std::uint32_t getReportIteration() const;

    /** @brief Retrieves the current number of failed optimization attempts */
    G_API_GENEVA std::uint32_t getStallCounter() const;

    /** @brief Allows to set the number of iterations without improvement, after which individuals are asked to update their internal data structures */
    G_API_GENEVA void setStallCounterThreshold(std::uint32_t stallCounterThreshold);
    /** @brief Allows to retrieve the number of iterations without improvement, after which individuals are asked to update their internal data structures */
    G_API_GENEVA std::uint32_t getStallCounterThreshold() const;

    /** @brief Retrieve the best value found in the entire optimization run so far */
    G_API_GENEVA std::tuple<double, double> getBestKnownPrimaryFitness() const;
    /** @brief Retrieves the best value found in the current iteration */
    G_API_GENEVA std::tuple<double, double> getBestCurrentPrimaryFitness() const;

    /** @brief Specifies whether information about termination reasons should be emitted */
    G_API_GENEVA void setEmitTerminationReason(bool emitTerminatioReason = true);
    /** @brief Retrieves information on whether information about termination reasons should be emitted */
    G_API_GENEVA bool getEmitTerminationReason() const;

    /******************************************************************************/
    /**
     * This function converts an individual at a given position to the derived
     * type and returns it. In DEBUG mode, the function will check whether the
     * requested position exists.
     *
     * @param pos The position in our data array that shall be converted
     * @return A converted version of the GParameterSet object, as required by the user
     */
    template<typename target_type>
    std::shared_ptr<target_type> individual_cast(
        std::size_t pos
    ) const {
#ifdef DEBUG
        if (pos >= this->size()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In G_OptimizationAlgorithm_Base::individual_cast<>() : Error" << std::endl
                    << "Tried to access position " << pos << " which is >= array size " << this->size() << std::endl
            );
        }
#endif /* DEBUG */

        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<GParameterSet, target_type>(this->at(pos));
    }

    /***************************************************************************/

    /** @brief Retrieve the number of processable items in the current iteration. */
    G_API_GENEVA std::size_t getNProcessableItems() const;

    /** @brief If individuals have been stored in this population, they are added to the priority queue. */
    G_API_GENEVA void addCleanStoredBests(GParameterSetFixedSizePriorityQueue &bestIndividuals);

    /** @brief Helper function that determines whether we are currently inside of the first iteration */
    G_API_GENEVA bool inFirstIteration() const;
    /** @brief Helper function that determines whether we are after the first iteration */
    G_API_GENEVA bool afterFirstIteration() const;

    /** @brief Checks whether a checkpoint-file has the same "personality" as our own algorithm */
    G_API_GENEVA bool cp_personality_fits(const boost::filesystem::path &p) const;

protected:
    /***************************************************************************/
    // Some data

    Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY>
        m_gr; ///< A random number generator. Note that the actual calculation is done in a random number proxy / factory
    std::uniform_real_distribution<double>
        m_uniform_real_distribution; ///< Access to uniformly distributed double random values

    /***************************************************************************/
    // Overridden or virtual protected functions

    /** @brief Adds local configuration options to a GParserBuilder object */
    G_API_GENEVA void addConfigurationOptions_(
        Gem::Common::GParserBuilder &gpb
    ) override;
    /** @brief Loads the data of another GOptimizationAlgorithm object */
    G_API_GENEVA void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<G_OptimizationAlgorithm_Base>(
        G_OptimizationAlgorithm_Base const &
        , G_OptimizationAlgorithm_Base const &
        , Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    G_API_GENEVA void compare_(
        const GObject &cp
        , const Gem::Common::expectation &e
        , const double &limit
    ) const override;

    /** @brief Resets the class to the state before the optimize call. */
    virtual G_API_GENEVA void resetToOptimizationStart_() BASE;

    /** @brief Initialization code to be run before the optimization cycle */
    virtual G_API_GENEVA void init() BASE;
    /** @brief Finalization code to be run after the optimization cycle */
    virtual G_API_GENEVA void finalize() BASE;

    /** @brief Applies modifications to this object */
    G_API_GENEVA bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    G_API_GENEVA void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

    /** @brief Delegation of work to be performed to the private executor object */
    G_API_GENEVA Gem::Courtier::executor_status_t workOn(
        std::vector<std::shared_ptr<GParameterSet>> &workItems
        , bool resubmitUnprocessed = false
        , const std::string &caller = std::string()
    );
    /** @brief Retrieves a vector of old work items after job submission */
    G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getOldWorkItems();

    /** @brief Saves the state of the class to disc */
    G_API_GENEVA void saveCheckpoint(bf::path const & outputFile) const;

    /** @brief Extracts the short name of the optimization algorithm */
    G_API_GENEVA std::string extractOptAlgFromPath(const boost::filesystem::path &p) const;

    /** @brief Allows to set the personality type of the individuals */
    G_API_GENEVA void setIndividualPersonalities();
    /** @brief Resets the individual's personality types */
    G_API_GENEVA void resetIndividualPersonalities();

    /** @brief Sets the default size of the population */
    G_API_GENEVA void setDefaultPopulationSize(const std::size_t &defPopSize);

    // NB: protected, as a derived function may fall back to this function, cmp EA in non-pareto mode
    /** @brief Adds the individuals of this iteration to a priority queue. */
    virtual G_API_GENEVA void updateGlobalBestsPQ_(GParameterSetFixedSizePriorityQueue &bestIndividuals) BASE;
    /** @brief Adds the individuals of this iteration to a priority queue. */
    virtual G_API_GENEVA void updateIterationBestsPQ_(GParameterSetFixedSizePriorityQueue &bestIndividuals) BASE;

    /** @brief Set the number of "best" individuals to be recorded in each iteration */
    G_API_GENEVA void setNRecordBestIndividuals(std::size_t nRecordBestIndividuals);
    /** @brief Retrieve the number of best individuals to be recorded in each iteration */
    G_API_GENEVA std::size_t getNRecordBestIndividuals() const;

    /** @brief Allows derived classes to reset the stall counter. */
    G_API_GENEVA void resetStallCounter();

    /** @brief Lets individuals know about the current iteration of the optimization cycle. */
    G_API_GENEVA void markIteration();

    /** @brief Let individuals know the number of stalls encountered so far */
    G_API_GENEVA void markNStalls();

private:
    /***************************************************************************/
    // Overloaded or virtual base functions

    /** @brief This function encapsulates some common functionality of iteration-based optimization algorithms. */
    G_API_GENEVA G_OptimizationAlgorithm_Base const * const optimize_(std::uint32_t offset) final;
    /** @brief Emits a name for this class / object; this can be a long name with spaces */
    G_API_GENEVA std::string name_() const override = 0;
    /** @brief Creates a deep clone of this object */
    G_API_GENEVA GObject *clone_() const override = 0;

    /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
    G_API_GENEVA void runFitnessCalculation_() override = 0;
    /** @brief The actual business logic to be performed during each iteration */
    virtual G_API_GENEVA std::tuple<double, double> cycleLogic_() BASE = 0;

    /** @brief Retrieve the current iteration of the optimization run */
    G_API_GENEVA std::uint32_t getIteration_() const override;

    /** @brief Retrieves the best individual found up to now */
    G_API_GENEVA std::shared_ptr<GParameterSet> getBestGlobalIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found */
    G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getBestGlobalIndividuals_() const final;

    /** @brief Retrieves the best individual found in the iteration */
    G_API_GENEVA std::shared_ptr<GParameterSet> getBestIterationIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found in the */
    G_API_GENEVA std::vector<std::shared_ptr<GParameterSet>> getBestIterationIndividuals_() const final;

    /** @brief Retrieve the number of processable items in the current iteration. */
    virtual G_API_GENEVA std::size_t getNProcessableItems_() const BASE;

    /** @brief Retrieve a personality trait object belonging to this algorithm */
    virtual G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const BASE = 0;

    /** @brief Resizes the population to the desired level and does some error checks */
    virtual G_API_GENEVA void adjustPopulation_() BASE = 0;

    /** @brief Gives derived classes an opportunity to update their internal structures. */
    virtual G_API_GENEVA void actOnStalls_() BASE = 0;

    /***************************************************************************/

    /** @brief Update the stall counter. */
    G_API_GENEVA void updateStallCounter(const std::tuple<double, double> &bestEval);

    /** @brief This function returns true once a given time has passed */
    G_API_GENEVA bool timedHalt(const std::chrono::system_clock::time_point &currentTime) const;
    /** @brief This function checks whether a minimum amount of time has passed */
    G_API_GENEVA bool minTimePassed(const std::chrono::system_clock::time_point &currentTime) const;

    /** @brief This function returns true once the quality has passed a given threshold */
    G_API_GENEVA bool qualityHalt() const;

    /** @brief This function returns true once a given number of stalls has been exceeded in a row */
    G_API_GENEVA bool stallHalt() const;

    /** @brief This function returns true once a maximum number of iterations has been exceeded */
    G_API_GENEVA bool iterationHalt() const;
    /** @brief This function returns true when the minimum number of iterations has been passed. */
    G_API_GENEVA bool minIterationPassed() const;

    /** @brief This function returns true if a SIGHUP / CTRL_CLOSE_EVENT signal was sent */
    G_API_GENEVA bool sigHupHalt() const;

    /** @brief Triggers termination of the optimization run, when a file with a user-defined file is modified */
    G_API_GENEVA bool touchHalt() const;

    /** @brief A wrapper for customHalt_ that allows to emit the termination reason */
    G_API_GENEVA bool customHalt() const BASE;
    /** @brief Custom setting of halt criteria */
    virtual G_API_GENEVA bool customHalt_() const BASE;

    /** @brief This function checks whether a halt criterion has been reached. */
    G_API_GENEVA bool halt() const;

    /** @brief Check whether the max-iteration halt is set */
    G_API_GENEVA bool maxIterationHaltset() const;
    /** @brief Check whether a halt criterion based on the number of stalls has been set */
    G_API_GENEVA bool stallHaltSet() const;

    /** @brief Check whether the maxDuration-halt criterion has been set */
    G_API_GENEVA bool maxDurationHaltSet() const;

    /** @brief Check whether the quality-threshold halt-criterion has been set */
    G_API_GENEVA bool qualityThresholdHaltSet() const;
    /** @brief Marks the globally best known fitness in all individuals */
    G_API_GENEVA void markBestFitness();

    /** @brief Indicates whether the stallCounterThreshold_ has been exceeded */
    G_API_GENEVA bool stallCounterThresholdExceeded() const;

    /** @brief Retrieves an executor for the given execution mode */
    G_API_GENEVA std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> createExecutor(const execMode &e);

    /***************************************************************************/
    // Data

    std::uint32_t m_iteration = 0; ///< The current iteration
    std::uint32_t m_offset = DEFAULTOFFSET; ///< An iteration offset which can be used, if the optimization starts from a checkpoint file
    std::uint32_t m_minIteration = DEFAULTMINIT; ///< The minimum number of iterations
    std::uint32_t m_maxIteration = DEFAULTMAXIT; ///< The maximum number of iterations
    std::uint32_t m_maxStallIteration = DEFAULTMAXSTALLIT; ///< The maximum number of generations without improvement, after which optimization is stopped
    std::uint32_t m_reportIteration = DEFAULTREPORTITER; ///< The number of generations after which a report should be issued

    std::size_t m_nRecordbestGlobalIndividuals = DEFNRECORDBESTINDIVIDUALS; ///< Indicates the number of best individuals to be recorded/updated in each iteration
    GParameterSetFixedSizePriorityQueue m_bestGlobalIndividuals_pq{m_nRecordbestGlobalIndividuals}; ///< A priority queue with the best individuals found so far
    GParameterSetFixedSizePriorityQueue m_bestIterationIndividuals_pq{0}; ///< A priority queue with the best individuals of a given iteration; unlimited size so all individuals of an iteration fit in

    std::size_t m_defaultPopulationSize = DEFAULTPOPULATIONSIZE; ///< The nominal size of the population
    std::tuple<double, double> m_bestKnownPrimaryFitness = std::tuple<double, double>(
        0.
        , 0.
    ); ///< Records the best primary fitness found so far
    std::tuple<double, double> m_bestCurrentPrimaryFitness = std::tuple<double, double>(
        0.
        , 0.
    ); ///< Records the best fitness found in the current iteration

    std::uint32_t m_stallCounter = 0; ///< Counts the number of iterations without improvement
    std::uint32_t m_stallCounterThreshold = DEFAULTSTALLCOUNTERTHRESHOLD; ///< The number of stalls after which individuals are asked to update their internal data structures

    std::int32_t m_cp_interval = DEFAULTCHECKPOINTIT; ///< Number of iterations after which a checkpoint should be written. -1 means: Write whenever an improvement was encountered
    std::string m_cp_base_name = DEFAULTCPBASENAME; ///< The base name of the checkpoint file
    boost::filesystem::path m_cp_directory_path = boost::filesystem::path(DEFAULTCPDIR); ///< Path object to store the directory
    mutable std::string m_cp_last = "empty"; ///< The name of the last saved checkpoint
    bool m_cp_remove = true; ///< Whether checkpoint files should be overwritten or kept
    Gem::Common::serializationMode m_cp_serialization_mode = DEFAULTCPSERMODE; ///< Determines whether check-pointing should be done in text-, XML, or binary mode
    double m_qualityThreshold = DEFAULTQUALITYTHRESHOLD; ///< A threshold beyond which optimization is expected to stop
    bool m_hasQualityThreshold = false; ///< Specifies whether a qualityThreshold has been set
    std::chrono::duration<double>
        m_maxDuration = Gem::Common::duration_from_string(DEFAULTDURATION); ///< Maximum time-frame for the optimization
    std::chrono::duration<double> m_minDuration
        = Gem::Common::duration_from_string(DEFAULTMINDURATION); ///< Minimum time-frame for the optimization
    mutable std::chrono::system_clock::time_point
        m_startTime; ///< Used to store the start time of the optimization. Declared mutable so the halt criteria can be const
    std::string m_terminationFile
        = DEFAULTTERMINATIONFILE; ///< The name of a file which, when modified after the start of the optimization run, will cause termination of the run
    bool m_terminateOnFileModification = false;
    bool m_emitTerminationReason
        = DEFAULTEMITTERMINATIONREASON; ///< Specifies whether information about reasons for termination should be emitted
    std::atomic<bool> m_halted{true}; ///< Set to true when halt() has returned "true"
    std::vector<std::tuple<double, double>>
        m_worstKnownValids_cnt; ///< Stores the worst known valid evaluations up to the current iteration (first entry: raw, second: tranformed)
    std::vector<std::shared_ptr<GBasePluggableOM>> m_pluggable_monitors_cnt; ///< A collection of monitors

    std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>
        m_executor_ptr; ///< Holds the current executor for this algorithm
    execMode m_default_execMode
        = execMode::BROKER; ///< The default execution mode. Unless explicitöy requested by the user, we always go through the broker
    std::string m_default_executor_config
        = "./config/GBrokerExecutor.json"; ///< The default configuration file for the broker executor
};

/*******************************************************************************/
/////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// Some serialization-related exports and declarations. Note that namespace
// specifiers are included in the macros, no need for an explicit namespace boost::serialization

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBasePluggableOM)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::G_OptimizationAlgorithm_Base)

BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GBrokerExecutorT<Gem::Geneva::GParameterSet>)
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GSerialExecutorT<Gem::Geneva::GParameterSet>)
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GMTExecutorT<Gem::Geneva::GParameterSet>)

/******************************************************************************/

