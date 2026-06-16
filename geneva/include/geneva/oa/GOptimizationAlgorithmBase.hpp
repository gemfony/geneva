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
#include <concepts>
#include <ctime>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

// Boost header files go here

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GThreadPool.hpp"
#include "common/GContainerT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GStdFilesystemPathSerialization.hpp"
#include "courtier/GExecutorStatusT.hpp" // executor_status_t (workOn's return type)
// --- Submission goes through courtier: the local consumer is selected by Go2 (or a standalone main)
//     and plumbed in via setLocalConsumer() / setBroker(), see workOn ---
#include "courtier/GBrokerT.hpp"
#include "courtier/GExecutorT.hpp"
#include "courtier/GSubmissionPolicy.hpp"
#include "courtier/consumers/GSerialConsumerT.hpp"
#include "courtier/consumers/GStdThreadConsumerT.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/ind/GIndividualSlot.hpp"
#include "geneva/par/GOptimizableEntityFixedSizePriorityQueue.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/Interface/GOptimizerIT.hpp"
#include "geneva/GenevaHelperFunctions.hpp"
#include "geneva/oa/GBasePluggableOM.hpp" // the pluggable-monitor CRTP root (extracted from this header)

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
/**
 * Identifies which courtier LOCAL consumer an algorithm should submit through when courtier
 * routing is active (Phase-7 increment 1). Go2 maps the chosen parallelisation mnemonic onto one of
 * these and plumbs it into the algorithm via GOptimizationAlgorithmBase::setLocalConsumer(). "none" (the
 * default) means "do not route through courtier" -- the legacy executor path is used instead.
 */
enum class local_consumer_kind {
    none,         ///< Not routed through courtier (legacy executor path).
    serial,       ///< Inline, single-threaded courtier consumer (mnemonic "sc").
    multithreaded ///< Thread-pool courtier consumer (mnemonic "stc").
};

/******************************************************************************/
// GBasePluggableOM -- the CRTP category root of all pluggable optimization monitors -- now lives in
// its own header (geneva/oa/GBasePluggableOM.hpp, included at the top of this file) so that
// monitor-only consumers do not drag in the large GOptimizationAlgorithmBase definition, and changes
// to the algorithm base do not force the monitor hierarchy to recompile.

// Forward declaration
class GAdaptionConfigBase;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements basic operations found in iteration-based optimization algorithms.
 * E.g., one might want to stop the optimization after a given number of cycles, or after
 * a given amount of time. The class also defines the interface functions common to these
 * algorithms, such as a general call to "optimize()".
 */
class GOptimizationAlgorithmBase // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GCommonInterfaceT<GOptimizationAlgorithmBase>
  , public Gem::Common::GUniquePtrContainerT<gpar::GIndividualSlot>
  , public Interface::GOptimizerIT<GOptimizationAlgorithmBase> {
private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /***************************************************************************/
    /**
     * Single declaration of this class'es local data members. This drives serialize(),
     * load_() and compare_() from one place. Plain members use make_member(); the
     * cloneable pointer container pluggable_monitors_cnt_ uses make_cloneable_container_member()
     * (deep-cloned on load); the atomic halted_ uses make_atomic_member() (loaded via
     * .store(.load()), compared via its loaded value, serialised through the existing
     * std::atomic<bool> free serialization).
     *
     * Deliberately NOT in this tuple, and handled manually in load_()/compare_() instead:
     *  - the container base GPtrContainerT<GOptimizableEntity> (a base-object, deep-copied
     *    on load via operator=);
     *  - best_iteration_individuals_pq_ (intentionally NOT persisted -- transient per
     *    iteration; copied in memory by load_() and compared by compare_()).
     *
     * cp_directory_path_ (std::filesystem::path) is included: it now serialises via the
     * free serialization in GStdFilesystemPathSerialization.hpp and is plain-assignable
     * in memory, so it needs no special handling anymore.
     */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("iteration_", iteration_),
            Gem::Common::make_member("offset_", offset_),
            Gem::Common::make_member("max_iteration_", max_iteration_),
            Gem::Common::make_member("min_iteration_", min_iteration_),
            Gem::Common::make_member("max_stall_iteration_", max_stall_iteration_),
            Gem::Common::make_member("report_iteration_", report_iteration_),
            Gem::Common::make_member("n_recordbest_global_individuals_", n_recordbest_global_individuals_),
            Gem::Common::make_member("best_global_individuals_pq_", best_global_individuals_pq_),
            Gem::Common::make_member("default_population_size_", default_population_size_),
            Gem::Common::make_member("best_known_primary_fitness_", best_known_primary_fitness_),
            Gem::Common::make_member("best_current_primary_fitness_", best_current_primary_fitness_),
            Gem::Common::make_member("stall_counter_", stall_counter_),
            Gem::Common::make_member("stall_counter_threshold_", stall_counter_threshold_),
            Gem::Common::make_member("cp_interval_", cp_interval_),
            Gem::Common::make_member("cp_base_name_", cp_base_name_),
            Gem::Common::make_member("cp_directory_path_", cp_directory_path_),
            Gem::Common::make_member("cp_last_", cp_last_),
            Gem::Common::make_member("cp_remove_", cp_remove_),
            Gem::Common::make_member("cp_serialization_mode_", cp_serialization_mode_),
            Gem::Common::make_member("quality_threshold_", quality_threshold_),
            Gem::Common::make_member("has_quality_threshold_", has_quality_threshold_),
            Gem::Common::make_member("max_duration_", max_duration_),
            Gem::Common::make_member("min_duration_", min_duration_),
            Gem::Common::make_member("termination_file_", termination_file_),
            Gem::Common::make_member("terminate_on_file_modification_", terminate_on_file_modification_),
            Gem::Common::make_member("emit_termination_reason_", emit_termination_reason_),
            Gem::Common::make_member("worst_known_valids_cnt_", worst_known_valids_cnt_),
            Gem::Common::make_member("n_threads_", n_threads_),
            Gem::Common::make_atomic_member("halted_", halted_),
            Gem::Common::make_cloneable_container_member("pluggable_monitors_cnt_", pluggable_monitors_cnt_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("iteration_", iteration_),
            Gem::Common::make_member("offset_", offset_),
            Gem::Common::make_member("max_iteration_", max_iteration_),
            Gem::Common::make_member("min_iteration_", min_iteration_),
            Gem::Common::make_member("max_stall_iteration_", max_stall_iteration_),
            Gem::Common::make_member("report_iteration_", report_iteration_),
            Gem::Common::make_member("n_recordbest_global_individuals_", n_recordbest_global_individuals_),
            Gem::Common::make_member("best_global_individuals_pq_", best_global_individuals_pq_),
            Gem::Common::make_member("default_population_size_", default_population_size_),
            Gem::Common::make_member("best_known_primary_fitness_", best_known_primary_fitness_),
            Gem::Common::make_member("best_current_primary_fitness_", best_current_primary_fitness_),
            Gem::Common::make_member("stall_counter_", stall_counter_),
            Gem::Common::make_member("stall_counter_threshold_", stall_counter_threshold_),
            Gem::Common::make_member("cp_interval_", cp_interval_),
            Gem::Common::make_member("cp_base_name_", cp_base_name_),
            Gem::Common::make_member("cp_directory_path_", cp_directory_path_),
            Gem::Common::make_member("cp_last_", cp_last_),
            Gem::Common::make_member("cp_remove_", cp_remove_),
            Gem::Common::make_member("cp_serialization_mode_", cp_serialization_mode_),
            Gem::Common::make_member("quality_threshold_", quality_threshold_),
            Gem::Common::make_member("has_quality_threshold_", has_quality_threshold_),
            Gem::Common::make_member("max_duration_", max_duration_),
            Gem::Common::make_member("min_duration_", min_duration_),
            Gem::Common::make_member("termination_file_", termination_file_),
            Gem::Common::make_member("terminate_on_file_modification_", terminate_on_file_modification_),
            Gem::Common::make_member("emit_termination_reason_", emit_termination_reason_),
            Gem::Common::make_member("worst_known_valids_cnt_", worst_known_valids_cnt_),
            Gem::Common::make_member("n_threads_", n_threads_),
            Gem::Common::make_atomic_member("halted_", halted_),
            Gem::Common::make_cloneable_container_member("pluggable_monitors_cnt_", pluggable_monitors_cnt_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // This is the CRTP category root. Its CRTP base
        // (Gem::Common::GCommonInterfaceT<GOptimizationAlgorithmBase>) carries no state and is therefore
        // not serialized as a base_object -- mirroring GObject, whose serialize() is
        // likewise empty. Only the stateful container base (GPtrContainerT), which is
        // a base-object rather than a local member, is serialized here.
        ar &make_nvp(
                "GStdPtrVectorInterfaceT_T",
                boost::serialization::base_object<Gem::Common::GUniquePtrContainerT<gpar::GIndividualSlot>>(*this)
            );

        // All members are derived from the single localMembers() declaration: plain
        // members serialise directly, the cloneable smart pointers (de)serialise as
        // polymorphic pointers, and halted_ goes through the std::atomic<bool> free
        // serialization.
        Gem::Common::serialize_members(ar, this->localMembers());
    }

    ///////////////////////////////////////////////////////////////////////

public:
    // The private split-serialization member load(Archive&, unsigned) below
    // name-hides the public load(const&) / load(shared_ptr<>) inherited from
    // Gem::Common::GCommonInterfaceT<GOptimizationAlgorithmBase>. Re-expose them so callers (and
    // the standard unit tests) can load one GOptimizationAlgorithmBase from another.
    using Gem::Common::GCommonInterfaceT<GOptimizationAlgorithmBase>::load;

    /***************************************************************************/
    // The population element is a GIndividualSlot (the individual + its OA scratch). Keep the
    // user-facing API individual-based: a user adds bare individuals and the algorithm wraps each in a
    // slot. The inherited slot push_back overloads remain available (re-exposed via the using-declaration
    // so the individual overload below does not name-hide them) for population-growth code that already
    // holds slots.
    using Gem::Common::GUniquePtrContainerT<gpar::GIndividualSlot>::push_back;

    /** @brief Adds an individual to the population, wrapping it in a fresh GIndividualSlot */
    void push_back(std::unique_ptr<gpar::GOptimizableEntity> ind) {
        Gem::Common::GUniquePtrContainerT<gpar::GIndividualSlot>::push_back(
            std::make_unique<gpar::GIndividualSlot>(std::move(ind))
        );
    }

    /** @brief The copy constructor */
    GOptimizationAlgorithmBase(GOptimizationAlgorithmBase const &cp);

    /***************************************************************************/
    // Defaulted functions

    GOptimizationAlgorithmBase() = default;
    ~GOptimizationAlgorithmBase() override = default;

    /***************************************************************************/

    /** @brief Performs the necessary administratory work of doing check-pointing */
    void checkpoint(bool is_better) const;

    /** @brief Loads the state of the class from disc */
    void loadCheckpoint(std::filesystem::path const &cp_file);

    /**
     * @brief Hands this algorithm an OA-owned adaption configuration to use for the run (Phase 8 step 4).
     * Go2 installs the config it holds for this algorithm's type before the algorithm runs in a chain; an
     * adapting algorithm (EA / SA) adopts it (validating that it matches the population's genome) instead
     * of deriving a default from the genome layout. The base does nothing -- non-adapting algorithms
     * (swarm, gradient descent, scan) have no adaption config.
     */
    virtual void setAdaptionConfig(std::shared_ptr<GAdaptionConfigBase> /* config */) { /* no-op */ }

    /**
     * @brief The OA-owned adaption configuration in effect for this run, or null. Adapting algorithms
     * (EA / SA) build it at init() and return it here so telemetry (e.g. the adaptor-property monitor) can
     * read each group's adaptor settings without snapshotting the structure-only genome layout. The base
     * (non-adapting algorithms) returns null.
     */
    virtual std::shared_ptr<const GAdaptionConfigBase> getAdaptionConfig() const { return {}; }

    /** @brief Checks whether the optimization process has been halted */
    bool halted() const;

    /** @brief Allows to set the number of generations after which a checkpoint should be written */
    void setCheckpointInterval(std::int32_t cp_interval);
    /** @brief Allows to retrieve the number of generations after which a checkpoint should be written */
    std::int32_t getCheckpointInterval() const;

    /** @brief Allows to set the base name of the checkpoint file and the directory where it should be stored. */
    void setCheckpointBaseName(std::string cp_directory, std::string cp_base_name);
    /** @brief Allows to retrieve the base name of the checkpoint file. */
    std::string getCheckpointBaseName() const;
    /** @brief Allows to retrieve the directory where checkpoint files should be stored */
    std::string getCheckpointDirectory() const;
    /** @brief Allows to retrieve the directory where checkpoint files should be stored */
    std::filesystem::path getCheckpointDirectoryPath() const;
    /** @brief Determines whether checkpointing should be done in Text-, XML- or Binary-mode */
    void setCheckpointSerializationMode(Gem::Common::serializationMode cp_ser_mode);
    /** @brief Retrieves the current checkpointing serialization mode */
    Gem::Common::serializationMode getCheckpointSerializationMode() const;
    /** @brief Allows to set the cp_overwrite_ flag */
    void setRemoveCheckpointFiles(bool cp_remove);
    /** @brief Allows to check whether checkpoint files will be removed */
    bool checkpointFilesAreRemoved() const;

    /** @brief Resets the class to the state before the optimize call. */
    void resetToOptimizationStart();

    /******************************************************************************/
    /**
     * Selects the courtier LOCAL consumer this algorithm submits through. Called by Go2 once the
     * parallelisation mnemonic is known, or directly for standalone use; transient runtime state,
     * neither serialized nor cloned. @p n_threads is honoured only for the multithreaded kind
     * (0 == hardware concurrency). If neither this nor setBroker() is called, init() defaults
     * to a multithreaded local consumer.
     */
    void setLocalConsumer(local_consumer_kind kind, unsigned int n_threads = 0) {
        local_kind_    = kind;
        local_threads_ = n_threads;
    }

    /** @brief Sets the number of threads used for parallel organizational work (adaption,
     *  recombination, ...). 0 means "automatic" (hardware concurrency). */
    void setNThreads(std::uint16_t n_threads);
    /** @brief Retrieves the number of threads used for parallel organizational work. */
    [[nodiscard]] std::uint16_t getNThreads() const;

    /******************************************************************************/
    /**
     * Injects a ready-to-use courtier broker that this algorithm should submit through, instead of
     * building its own local consumer (Phase-7 increment 2). The broker must already have its
     * consumer registered, its clone function set, and -- for networked consumers -- its server
     * started. Used by Go2 for the networked consumers (asio/websocket), where a single server-backed
     * consumer is shared across the whole run rather than created per algorithm. Transient runtime
     * state, neither serialized nor cloned; takes precedence over setLocalConsumer().
     */
    void setBroker(std::shared_ptr<Gem::Courtier::GBrokerT<gpar::GOptimizableEntity>> broker) {
        broker_          = std::move(broker);
        external_broker_ = true;
    }

    /******************************************************************************/

    /** @brief Emits information specific to this class */
    void informationUpdate(const infoMode &im);

    /** @brief Checks whether a better solution was found. */
    bool progress() const;

    /** @brief Allows to register a pluggable optimization monitor. */
    void registerPluggableOM(std::shared_ptr<GBasePluggableOM> pluggable_om);
    /** @brief Allows to reset the local pluggable optimization monitors */
    void resetPluggableOM();
    /** @brief Allows to check whether pluggable optimization monitors were registered */
    bool hasPluggableOptimizationMonitors() const;

    /** @brief Retrieves the default population size */
    std::size_t getDefaultPopulationSize() const;
    /** @brief Retrieve the current population size */
    std::size_t getPopulationSize() const;

    /** @brief Set the number of iterations after which the optimization should be stopped */
    void setMaxIteration(std::uint32_t max_iteration);
    /** @brief Retrieve the number of iterations after which optimization should be stopped */
    std::uint32_t getMaxIteration() const;

    /** @brief Sets the minimum number of iterations */
    void setMinIteration(std::uint32_t min_iteration);
    /** @brief Retrieves the currently set minimum number of iterations */
    std::uint32_t getMinIteration() const;

    /** @brief Sets the maximum number of iterations allowed without improvement of the best individual */
    void setMaxStallIteration(std::uint32_t max_stall_iteration);
    /** @brief Retrieves the maximum number of generations allowed in an optimization run without improvement of the best individual. */
    std::uint32_t getMaxStallIteration() const;

    /** @brief Sets the maximum allowed processing time */
    void setMaxTime(std::chrono::duration<double> max_duration);
    /** @brief Retrieves the value of the max_duration_ parameter. */
    std::chrono::duration<double> getMaxTime() const;

    /** @brief Sets the minimum required processing time */
    void setMinTime(std::chrono::duration<double> min_duration);
    /** @brief Retrieves the value of the min_duration_ parameter */
    std::chrono::duration<double> getMinTime() const;

    /** @brief Sets a quality threshold beyond which optimization is expected to stop */
    void setQualityThreshold(double quality_threshold, bool has_quality_threshold);
    /** @brief Retrieves the current value of the quality threshold and also indicates whether the threshold is active */
    double getQualityThreshold(bool &has_quality_threshold) const;

    /** @brief Sets the name of a "termination file" */
    void setTerminationFile(std::string termination_file, bool terminate_on_file_modification);
    /** @brief Retrieves the current name of the termination file and also indicates whether the "touched halt" is active */
    std::string getTerminationFile(bool &terminate_on_file_modification) const;

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
    void setStallCounterThreshold(std::uint32_t stall_counter_threshold);
    /** @brief Allows to retrieve the number of iterations without improvement, after which individuals are asked to update their internal data structures */
    std::uint32_t getStallCounterThreshold() const;

    /** @brief Retrieve the best value found in the entire optimization run so far */
    std::tuple<double, double> getBestKnownPrimaryFitness() const;
    /** @brief Retrieves the best value found in the current iteration */
    std::tuple<double, double> getBestCurrentPrimaryFitness() const;

    /** @brief Specifies whether information about termination reasons should be emitted */
    void setEmitTerminationReason(bool emit_terminatio_reason = true);
    /** @brief Retrieves information on whether information about termination reasons should be emitted */
    bool getEmitTerminationReason() const;

    /******************************************************************************/
    /**
     * This function converts an individual at a given position to the derived
     * type and returns it. In DEBUG mode, the function will check whether the
     * requested position exists.
     *
     * @param pos The position in our data array that shall be converted
     * @return A converted version of the GOptimizableEntity object, as required by the user
     */
    template <typename target_type>
    std::shared_ptr<target_type> individual_cast(std::size_t pos) const {
#ifdef DEBUG
        if(pos >= this->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizationAlgorithmBase::individual_cast<>() : Error" << '\n'
                << "Tried to access position " << pos << " which is >= array size " << this->size()
                << '\n'
            );
        }
#endif /* DEBUG */

        // The population owns each slot by unique_ptr, and each slot owns its individual. Callers
        // (pluggable monitors) only read the individual transiently, so hand back a NON-OWNING shared_ptr
        // view (no-op deleter) of the live individual rather than co-owning or cloning it -- the slot
        // outlives the call (the population owns it). Does error checks on the conversion internally.
        std::shared_ptr<gpar::GOptimizableEntity> view(&this->at(pos)->individual(), [](gpar::GOptimizableEntity *) {});
        return Gem::Common::convertSmartPointer<gpar::GOptimizableEntity, target_type>(view);
    }

    /***************************************************************************/

    /** @brief Retrieve the number of processable items in the current iteration. */
    std::size_t getNProcessableItems() const;

    /** @brief If individuals have been stored in this population, they are added to the priority queue. */
    void addCleanStoredBests(gpar::GOptimizableEntityFixedSizePriorityQueue &best_individuals);

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
    void load_(const GOptimizationAlgorithmBase *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GOptimizationAlgorithmBase>(
        GOptimizationAlgorithmBase const &,
        GOptimizationAlgorithmBase const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GOptimizationAlgorithmBase &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Resets the class to the state before the optimize call. */
    virtual void resetToOptimizationStart_();

    /** @brief Initialization code to be run before the optimization cycle */
    virtual void init();
    /** @brief Finalization code to be run after the optimization cycle */
    virtual void finalize();

    /***************************************************************************/
    /**
     * A thread pool shared by all optimization algorithms for parallel per-individual
     * "organizational" work (e.g. EA adaption and recombination, SA adaption). It is
     * created in init() (sized by n_threads_) and released in finalize(), so it only
     * lives for the duration of a run and is neither serialized nor cloned.
     */
    std::shared_ptr<Gem::Common::GThreadPool> tp_ptr_;
    /** @brief The number of threads used for parallel organizational work. */
    std::uint16_t n_threads_ = Gem::Common::DEFAULTNHARDWARETHREADS;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

    /***************************************************************************/

    /** @brief Submits the contiguous sub-range [start, end) of @p work_items for evaluation through
     *  courtier. The algorithm passes the range it wants evaluated explicitly (no per-item DO_PROCESS
     *  flagging needed); the consumer marks and reconciles exactly that span in place. */
    Gem::Courtier::executor_status_t workOn(
        std::vector<std::unique_ptr<gpar::GOptimizableEntity>> &work_items,
        std::size_t start,
        std::size_t end
    );

    /**
     * @brief Submits the population's [start, end) range for evaluation. The population holds slots, but
     * the courtier deals in individuals: this moves each slot's individual out into a submission vector
     * (positions preserved), runs workOn() on it, then moves the (possibly reconciled) individuals back
     * into their slots. The slots -- and the OA scratch they carry -- stay put. workOn() is in-place
     * (the work-item vector keeps its size), so the move-back by index is exact.
     */
    Gem::Courtier::executor_status_t workOnPopulation(std::size_t start, std::size_t end);
    /** @brief Retrieves a vector of old work items after job submission */
    std::vector<std::unique_ptr<gpar::GOptimizableEntity>> getOldWorkItems();

    /** @brief Saves the state of the class to disc */
    void saveCheckpoint(std::filesystem::path const &output_file) const;

    /** @brief Extracts the short name of the optimization algorithm */
    std::string extractOptAlgFromPath(const std::filesystem::path &p) const;

    /** @brief Allows to set the personality type of the individuals */
    void setIndividualPersonalities();
    /** @brief Resets the individual's personality types */
    void resetIndividualPersonalities();

    /** @brief Sets the default size of the population */
    void setDefaultPopulationSize(std::size_t def_pop_size);

    // NB: protected, as a derived function may fall back to this function, cmp EA in non-pareto mode
    /** @brief Adds the individuals of this iteration to a priority queue. */
    virtual void updateGlobalBestsPQ_(gpar::GOptimizableEntityFixedSizePriorityQueue &best_individuals);
    /** @brief Adds the individuals of this iteration to a priority queue. */
    virtual void updateIterationBestsPQ_(gpar::GOptimizableEntityFixedSizePriorityQueue &best_individuals);

    /** @brief Set the number of "best" individuals to be recorded in each iteration */
    void setNRecordBestIndividuals(std::size_t n_record_best_individuals);
    /** @brief Retrieve the number of best individuals to be recorded in each iteration */
    std::size_t getNRecordBestIndividuals() const;

    /** @brief Allows derived classes to reset the stall counter. */
    void resetStallCounter();

    /** @brief Lets individuals know about the current iteration of the optimization cycle. */
    void markIteration();

    /** @brief Let individuals know the number of stalls encountered so far */
    void markNStalls();

    /**
     * @brief Whether this optimization run was just resumed from a checkpoint. Set by loadCheckpoint()
     * (after the population -- with its OA-owned scratch -- has been deserialised) and cleared once the
     * run's setup has consumed it (in optimize_(), right after init()). While true, the setup steps
     * PRESERVE the restored per-individual scratch (personality + adaption / swarm / CG POD blocks)
     * instead of re-seeding it, so a resumed algorithm keeps its evolved state. It is deliberately NOT
     * serialized (it is a transient resume marker, and loadCheckpoint sets it AFTER fromFile anyway).
     */
    bool resumedFromCheckpoint() const { return resumed_from_checkpoint_; }
    bool resumed_from_checkpoint_ = false;

private:
    /***************************************************************************/
    // Overloaded or virtual base functions

    /** @brief This function encapsulates some common functionality of iteration-based optimization algorithms. */
    GOptimizationAlgorithmBase const *optimize_(std::uint32_t offset) final;
    /** @brief Emits a name for this class / object; this can be a long name with spaces */
    std::string name_() const override = 0;
    /** @brief Creates a deep clone of this object */
    GOptimizationAlgorithmBase *clone_() const override = 0;

    /** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
    void runFitnessCalculation_() override = 0;
    /** @brief The actual business logic to be performed during each iteration */
    virtual std::tuple<double, double> cycleLogic_() = 0;

    /** @brief Retrieve the current iteration of the optimization run */
    std::uint32_t getIteration_() const override;

    /** @brief Retrieves the best individual found up to now */
    std::shared_ptr<gpar::GOptimizableEntity> getBestGlobalIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found */
    std::vector<std::shared_ptr<gpar::GOptimizableEntity>>
    getBestGlobalIndividuals_() const final;

    /** @brief Retrieves the best individual found in the iteration */
    std::shared_ptr<gpar::GOptimizableEntity> getBestIterationIndividual_() const final;
    /** @brief Retrieves a list of the best individuals found in the */
    std::vector<std::shared_ptr<gpar::GOptimizableEntity>>
    getBestIterationIndividuals_() const final;

    /** @brief Retrieve the number of processable items in the current iteration. */
    virtual std::size_t getNProcessableItems_() const;

    /** @brief The submission policy this algorithm uses when routed through courtier (Phase 7).
     *  Default: clone-on-partial-return (population-based, tolerant -- EA/SA/Swarm). The "need-all"
     *  algorithms (GD/CGD/Nelder-Mead/ParameterScan), which cannot proceed with a missing or failed
     *  evaluation, override this to full-success-or-fatal. */
    virtual Gem::Courtier::GSubmissionPolicy getSubmissionPolicy_() const {
        return Gem::Courtier::GSubmissionPolicy::clone_on_partial_return();
    }

    /** @brief Retrieve a personality trait object belonging to this algorithm */
    virtual std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const = 0;

    /** @brief Resizes the population to the desired level and does some error checks */
    virtual void adjustPopulation_() = 0;

    /** @brief Gives derived classes an opportunity to update their internal structures. */
    virtual void actOnStalls_() = 0;

    /***************************************************************************/

    /** @brief Update the stall counter. */
    void updateStallCounter(const std::tuple<double, double> &best_eval);

    /** @brief This function returns true once a given time has passed */
    bool timedHalt(const std::chrono::system_clock::time_point &current_time) const;
    /** @brief This function checks whether a minimum amount of time has passed */
    bool minTimePassed(const std::chrono::system_clock::time_point &current_time) const;

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

    /** @brief Check whether the max_duration-halt criterion has been set */
    bool maxDurationHaltSet() const;

    /** @brief Check whether the quality-threshold halt-criterion has been set */
    bool qualityThresholdHaltSet() const;
    /** @brief Marks the globally best known fitness in all individuals */
    void markBestFitness();

    /** @brief Indicates whether the stall_counter_threshold_ has been exceeded */
    bool stallCounterThresholdExceeded() const;

    /***************************************************************************/
    // Data

    std::uint32_t iteration_ = 0; ///< The current iteration
    std::uint32_t offset_ =
        DEFAULTOFFSET; ///< An iteration offset which can be used, if the optimization starts from a checkpoint file
    std::uint32_t min_iteration_ = DEFAULTMINIT; ///< The minimum number of iterations
    std::uint32_t max_iteration_ = DEFAULTMAXIT; ///< The maximum number of iterations
    std::uint32_t max_stall_iteration_ =
        DEFAULTMAXSTALLIT; ///< The maximum number of generations without improvement, after which optimization is stopped
    std::uint32_t report_iteration_ =
        DEFAULTREPORTITER; ///< The number of generations after which a report should be issued

    std::size_t n_recordbest_global_individuals_ =
        DEFNRECORDBESTINDIVIDUALS; ///< Indicates the number of best individuals to be recorded/updated in each iteration
    gpar::GOptimizableEntityFixedSizePriorityQueue best_global_individuals_pq_{
        n_recordbest_global_individuals_
    }; ///< A priority queue with the best individuals found so far
    gpar::GOptimizableEntityFixedSizePriorityQueue best_iteration_individuals_pq_{
        n_recordbest_global_individuals_
    }; ///< A priority queue with the best individuals of a given iteration; unlimited size so all individuals of an iteration fit in

    std::size_t default_population_size_ =
        DEFAULTPOPULATIONSIZE; ///< The nominal size of the population
    std::tuple<double, double> best_known_primary_fitness_ =
        std::tuple<double, double>(0., 0.); ///< Records the best primary fitness found so far
    std::tuple<double, double> best_current_primary_fitness_ = std::tuple<double, double>(
        0.,
        0.
    ); ///< Records the best fitness found in the current iteration

    std::uint32_t stall_counter_ = 0; ///< Counts the number of iterations without improvement
    std::uint32_t stall_counter_threshold_ =
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
    double quality_threshold_ =
        DEFAULTQUALITYTHRESHOLD; ///< A threshold beyond which optimization is expected to stop
    bool has_quality_threshold_ = false; ///< Specifies whether a quality_threshold has been set
    std::chrono::duration<double> max_duration_ = Gem::Common::duration_from_string(
        DEFAULTDURATION
    ); ///< Maximum time-frame for the optimization
    std::chrono::duration<double> min_duration_ = Gem::Common::duration_from_string(
        DEFAULTMINDURATION
    ); ///< Minimum time-frame for the optimization
    mutable std::chrono::system_clock::time_point
        start_time_; ///< Used to store the start time of the optimization. Declared mutable so the halt criteria can be const
    mutable std::filesystem::file_time_type
        file_start_time_; ///< Used for the touchHalt-feature, as system_clock file_time may not be comparable
    std::string termination_file_ =
        DEFAULTTERMINATIONFILE; ///< The name of a file which, when modified after the start of the optimization run, will cause termination of the run
    bool terminate_on_file_modification_ = false;
    bool emit_termination_reason_ =
        DEFAULTEMITTERMINATIONREASON; ///< Specifies whether information about reasons for termination should be emitted
    std::atomic<bool> halted_{true}; ///< Set to true when halt() has returned "true"
    std::vector<std::tuple<double, double>>
        worst_known_valids_cnt_; ///< Stores the worst known valid evaluations up to the current iteration (first entry: raw, second: tranformed)
    std::vector<std::shared_ptr<GBasePluggableOM>>
        pluggable_monitors_cnt_; ///< A collection of monitors

    // --- courtier submission (TRANSIENT, not serialized/cloned). The algorithm submits through
    // courtier's span+policy executor; the broker/executor/consumer are lazily created on first use.
    // For a LOCAL consumer the kind/thread-count are plumbed in via setLocalConsumer()
    // (selecting GSerialConsumerT vs GStdThreadConsumerT); a ready networked broker is injected via
    // setBroker(). init() defaults the kind to multithreaded when neither is set. ---
    local_consumer_kind local_kind_ = local_consumer_kind::none; ///< Which local consumer (none == legacy path)
    unsigned int local_threads_ = 0; ///< Thread-pool size for the multithreaded kind (0 == hardware concurrency)
    bool external_broker_ = false; ///< True when Go2 injected a ready broker (networked) via setBroker()
    std::shared_ptr<Gem::Courtier::GBrokerT<gpar::GOptimizableEntity>> broker_;
    std::shared_ptr<Gem::Courtier::GExecutorT<gpar::GOptimizableEntity>> executor_;
    /** @brief Submits the contiguous sub-range [start, end) of @p work_items through courtier. */
    Gem::Courtier::executor_status_t workOnViaConsumer_(
        std::vector<std::unique_ptr<gpar::GOptimizableEntity>> &work_items,
        std::size_t start,
        std::size_t end
    );
};

/*******************************************************************************/
/////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************/

} /* namespace Gem::Geneva::OptimizationAlgorithms */

/******************************************************************************/
// Some serialization-related exports and declarations. Note that namespace
// specifiers are included in the macros, no need for an explicit namespace boost::serialization

// GBasePluggableOM's BOOST_SERIALIZATION_ASSUME_ABSTRACT lives in geneva/oa/GBasePluggableOM.hpp.
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::OptimizationAlgorithms::GOptimizationAlgorithmBase) // NOLINT
/******************************************************************************/
