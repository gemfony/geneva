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
#include <chrono>
#include <concepts>
#include <exception>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <vector>

// Boost headers go here
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GSerializableFunctionObjectT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp" // serialization of std::chrono time_point members
#include "common/GSerializeTupleT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "courtier/GProcessable.hpp" // the non-generic processing-lifecycle base

namespace Gem::Courtier {

/******************************************************************************/
// The processing exception (g_processing_exception) lives on the GProcessable lifecycle base, included
// above, so transports and processables share one type.

/******************************************************************************/
/**
	 * @brief The generic, self-contained base for a consumer-submittable work item: GProcessable plus a
	 * typed result store and the process() orchestration.
	 *
	 * GProcessable (the base) carries only the non-generic processing LIFECYCLE (status / routing / timing)
	 * and has no process() and no result store, so it is not by itself a submittable work item. This class
	 * adds the missing half: a typed result store (one or more processing_result_type values), the
	 * process() orchestration (timing, the configured pre-/post-processors, error/exception handling around
	 * the user's process_() hook) and the results-only return graft -- i.e. the full contract the courtier
	 * consumers require of an item they evaluate. Derive this (CRTP-style, passing yourself as
	 * processable_type) to make your OWN type runnable on any courtier consumer; re-implement the pure
	 * virtual process_() and make the derived class serializable so it travels on the networked transports.
	 *
	 * This base is deliberately free of any dependency on the geneva optimization library, which keeps the
	 * courtier consumer/transport machinery testable on its own (courtier sits BELOW geneva in the library
	 * order): the geneva-free demo work items in GDemoProcessingContainers derive this and are what the
	 * courtier unit tests submit. Geneva's own work item, Gem::Geneva::Genome::GOptimizableEntity, is a
	 * SPECIALIZED, standalone second implementation of the same contract -- it derives GProcessable directly
	 * and provides its own result store + process() (folding in optimization-specific orchestration:
	 * feasibility, the evaluation policy, multi-criterion fitness), so it does NOT derive this class.
	 *
	 * @tparam processable_type The concrete type deriving GProcessingContainerT (CRTP)
	 * @tparam processing_result_type The result type of the process_ call; should be copyable
	 */
template <
    typename processable_type,
    typename processing_result_type>
    requires (!std::is_void_v<processing_result_type>)
class GProcessingContainerT : public GProcessable {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        // The non-generic lifecycle state (status, errors, routing counters, timing) is serialised by
        // the GProcessable base; this class adds only the result store and the (typed) pre-/post-
        // processors plus their veto flags.
        ar &make_nvp("GProcessable", boost::serialization::base_object<GProcessable>(*this)) &
            BOOST_SERIALIZATION_NVP(pre_processing_disabled_) &
            BOOST_SERIALIZATION_NVP(post_processing_disabled_) &
            BOOST_SERIALIZATION_NVP(pre_processor_ptr_) &
            BOOST_SERIALIZATION_NVP(post_processor_ptr_) &
            BOOST_SERIALIZATION_NVP(stored_results_cnt_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    using payload_type = processable_type;
    using result_type = processing_result_type;

    /***************************************************************************/
    /**
	  * @brief Initialization with the number of stored results
	  *
	  * @param n_stored_results The number of result slots to allocate (each default-initialized)
	  */
    explicit GProcessingContainerT(std::size_t n_stored_results)
      : stored_results_cnt_(n_stored_results, processing_result_type()) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * @brief The copy constructor
	  *
	  * @param cp Another GProcessingContainerT object to be copied
	  */
    explicit GProcessingContainerT(
        GProcessingContainerT<processable_type, processing_result_type> const &cp
    )
      : GProcessable(cp) // copies the non-generic lifecycle state (status, errors, counters, timing)
      , pre_processing_disabled_(cp.pre_processing_disabled_)
      , post_processing_disabled_(cp.post_processing_disabled_)
      , stored_results_cnt_(
            cp.stored_results_cnt_
        ) // Note: processing_result_type must be copyable (e.g. it should not contain pointers)
    {
        Gem::Common::copyCloneableSmartPointer(cp.pre_processor_ptr_, pre_processor_ptr_);
        Gem::Common::copyCloneableSmartPointer(cp.post_processor_ptr_, post_processor_ptr_);
    }

    /***************************************************************************/
    /**
	  * @brief Copy assignment operator
	  *
	  * @param cp Another GProcessingContainerT object whose data is copied into this one
	  * @return A reference to this object
	  */
    GProcessingContainerT<processable_type, processing_result_type> &
    operator=(GProcessingContainerT<processable_type, processing_result_type> const &cp) {
        GProcessable::operator=(cp); // the non-generic lifecycle state
        pre_processing_disabled_ = cp.pre_processing_disabled_;
        post_processing_disabled_ = cp.post_processing_disabled_;
        stored_results_cnt_ =
            cp.stored_results_cnt_; // Note: processing_result_type must be copyable (e.g. it should not contain pointers)

        Gem::Common::copyCloneableSmartPointer(cp.pre_processor_ptr_, pre_processor_ptr_);
        Gem::Common::copyCloneableSmartPointer(cp.post_processor_ptr_, post_processor_ptr_);

        return *this;
    }

    /***************************************************************************/
    // Defaulted or deleted constructors, destructor and move assignment operator

    // Default constructor may be found in private section

    // Move operations: a defaulted member-wise move is correct and noexcept
    // here. The heavy members (stored_results_cnt_, stored_error_descriptions_,
    // pre_/post_processor_ptr_) have their ownership transferred rather than
    // being deep-copied as the copy operations do -- this is the point of being
    // movable on the work-transport (broker / MPI / websocket) path. The
    // remaining scalar/enum/time-point members are moved trivially.
    GProcessingContainerT(
        GProcessingContainerT<processable_type, processing_result_type> &&
    ) noexcept = default;
    GProcessingContainerT<processable_type, processing_result_type> &
    operator=(GProcessingContainerT<processable_type, processing_result_type> &&) noexcept =
        default;

    virtual ~GProcessingContainerT() = default;

    /***************************************************************************/
    /**
	  * @brief Sets the vector of stored results to a given collection and marks
	  * the object as processed
	  *
	  * @param result_cnt The new result vector (must match the configured number of stored results)
	  * @return The first stored result after the assignment
	  */
    processing_result_type
    markAsProcessedWith(std::vector<processing_result_type> const &result_cnt) {
#ifdef DEBUG
        // Check that we have been given a suitable new results vector
        if(result_cnt.size() != stored_results_cnt_.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GProcessingContainerT::markAsProcessedWith(): Vector dimensions" << '\n'
                << "do not fit: " << result_cnt.size() << " / " << stored_results_cnt_.size()
                << '\n'
            );
        }
#endif

        // Transfer the new values
        stored_results_cnt_ = result_cnt;

        // Clear the error descriptions
        stored_error_descriptions_.clear();

        // Mark as processed
        processing_status_ = processingStatus::PROCESSED;

        // This part of the code should never be reached if an exception was thrown
        return this->stored_results_cnt_.at(0);
    }

    /***************************************************************************/
    /**
	  * Perform the actual processing steps. E.g. in optimization algorithms,
	  * post-processing allows to run a sub-optimization. The amount of time
	  * needed for processing is measured for logging purposes. Where one of the
	  * processing functions throws an exception, the function will store the
	  * necessary exception information locally and rethrow the exception.
	  * Note that user-defined processing- and post-processing functions need
	  * to make sure to set the results (be it main- or secondary results) of the
	  * process()-call. This function has no way to ensure that this is the case.
	  *
	  * @param res_vec Allows to inject an external evaluation
	  * @return The first result of the processing calls
	  */
    processing_result_type process(
        const std::vector<processing_result_type> &res_vec = std::vector<processing_result_type>()
    ) {
        // We only accept items that are due for processing
        if(processingStatus::DO_PROCESS != processing_status_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GProcessingContainerT::process(): Function called while processing_status_ "
                   "was set to "
                << processing_status_ << '\n'
                << "Expected " << processingStatus::DO_PROCESS << '\n'
            );
        }

        // Assign a new evaluation id
        // evaluation_id_ = std::string("eval_") + Gem::Common::to_string(boost::uuids::random_generator()());

        // Clear the error descriptions
        stored_error_descriptions_.clear();

        // "Nullify the result list.
        this->clear_stored_results_vec();

        std::ostringstream error_description_stream; // NOLINT(cppcoreguidelines-init-variables)
        processing_result_type main_result;

        try {
            // Perform the actual processing
            const auto start_time = std::chrono::high_resolution_clock::now();
            this->preProcess_();
            const auto after_pre_processing = std::chrono::high_resolution_clock::now();

            // Do the actual processing
            this->process_(res_vec);

            // The fitness has now been computed, so the work item is processed. Mark it PROCESSED
            // BEFORE post-processing: a post-processor refines an ALREADY-EVALUATED item (e.g. by running
            // a short sub-optimization) and rejects a dirty one. If processing flagged an error, the
            // error status is left intact.
            if(not this->has_errors()) {
                processing_status_ = processingStatus::PROCESSED;
            }

            const auto after_processing = std::chrono::high_resolution_clock::now();
            this->postProcess_();
            const auto after_post_processing = std::chrono::high_resolution_clock::now();

            // Make a note of the time needed for each step
            pre_processing_time_ =
                std::chrono::duration<double>(after_pre_processing - start_time).count();
            processing_time_ =
                std::chrono::duration<double>(after_processing - after_pre_processing).count();
            post_processing_time_ =
                std::chrono::duration<double>(after_post_processing - after_processing).count();

            processing_status_ = processingStatus::PROCESSED;
        }
        catch(std::exception &e) {
            // Let the audience know we had an error
            processing_status_ = processingStatus::EXCEPTION_CAUGHT;
            error_description_stream
                << "In GProcessingContainerT<processable_type>::process():" << '\n'
                << "Processing has thrown an exception with message" << '\n'
                << e.what() << '\n'
                << "We will rethrow this exception" << '\n';
        }
        catch(...) {
            // Let the audience know we had an error
            processing_status_ = processingStatus::EXCEPTION_CAUGHT;
            error_description_stream
                << "In GProcessingContainerT<processable_type>::process():" << '\n'
                << "Processing has thrown an unknown exception." << '\n';
        }

        if(this->has_errors()) { // Either an exception was caught or the user has flagged an error
            // Do some cleanup
            pre_processing_time_ = 0.;
            processing_time_ = 0.;
            post_processing_time_ = 0.;

            // "Nullify the result list.
            this->clear_stored_results_vec();

            // Store the exceptions for later reference
            if(processingStatus::EXCEPTION_CAUGHT == processing_status_) {
                // Error information added by the user might already be stored in this variable. Hence we use +=
                stored_error_descriptions_ += error_description_stream.str();
            }

            throw g_processing_exception( // Note: this is a specific exception to flag errors during processing
					g_error_streamer(DO_LOG, Gem::Common::timeAndPlace()) << stored_error_descriptions_
				);
        }

        // This part of the code should never be reached if an exception was thrown
        return this->stored_results_cnt_.at(0);
    }

    /***************************************************************************/
    /**
	  * Retrieval of the stored result. This function does not allow modifications
	  * of its return value. It will throw, if value retrieval is initiated for a work
	  * item which does not have the PROCESSED flag set.
	  *
	  * @param id The id of the stored result to be returned
	  * @return The stored result at position id in stored_results_vec_
	  */
    processing_result_type getStoredResult(std::size_t id = 0) const {
        if(not this->is_processed()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GProcessingContainerT::getStoredResult(): Tried to" << '\n'
                << "retrieve stored result while the PROCESSED flag was not set" << '\n'
            );
        }

        return stored_results_cnt_.at(id);
    }

    /******************************************************************************/
    /**
	  * Retrieve the id assigned to the current evaluation. Note that there is no
	  * guaranty that the item has indeed been processed. This is id simply represents
	  * the processing id assigned at the beginning of the last process()-call.
	  */
    /*
		std::string getCurrentEvaluationID() const {
			return evaluation_id_;
		}*/

    /***************************************************************************/
    /**
	  * @brief Loads user-specified data. This function can be overloaded by derived classes. It
	  * is mainly intended to provide a mechanism to "deposit" an item at a remote site
	  * that holds otherwise constant data. That data then does not need to be serialized
	  * but can be loaded whenever a new work item arrives and has been de-serialized. Note
	  * that, if your individuals do not serialize important parts of an object, you need
	  * to make sure that constant data is loaded after reloading a checkpoint.
	  *
	  * @param cd_ptr A pointer to the object whose data should be loaded
	  */
    void loadConstantData(std::shared_ptr<processable_type> cd_ptr) {
        this->loadConstantData_(cd_ptr);
    }

    /***************************************************************************/
    /**
	  * @brief Whether this (deserialized) work item arrived WITHOUT its input data -- the lightweight
	  * "results-only" return form, in which a worker returns the computed results but not the (large)
	  * input parameters, because the server still holds the originally-submitted item. The server grafts
	  * the input data back on via graftInputDataFrom() before using the item. Default false; a derived
	  * type that supports results-only returns overrides the hook below.
	  *
	  * @return true iff this item's input data was omitted on the wire and must be grafted from the original
	  */
    bool inputDataOmitted() const { return this->inputDataOmitted_(); }

    /***************************************************************************/
    /**
	  * @brief Grafts the input data (parameters) of @p original onto this item, which carries valid
	  * computed results but no input data (a results-only return). After the graft the item is complete
	  * and equivalent to a full return. Default no-op.
	  *
	  * @param original The originally-submitted item, still held by the server, that supplies the input data
	  */
    void graftInputDataFrom(const processable_type &original) { this->graftInputDataFrom_(original); }

    /***************************************************************************/
    /**
	  * @brief Allows to check whether any user-defined pre-processing before the process()-
	  * step may occur. This may alter the individual's data.
	  *
	  * @return true if pre-processing is currently allowed, false if it has been vetoed
	  */
    bool mayBePreProcessed() const noexcept {
        return not pre_processing_disabled_;
    }

    /***************************************************************************/
    /**
	  * @brief Allow or prevent pre-processing (used by pre-processing algorithms to prevent
	  * recursive pre-processing). See e.g. GEvolutionaryAlgorithmPostOptimizerT. Once a veto
	  * exists, no pre-processing will occur until the veto is lifted.
	  *
	  * @param veto true to disable (veto) pre-processing, false to allow it
	  */
    void vetoPreProcessing(bool veto) noexcept {
        pre_processing_disabled_ = veto;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to register a pre-processor object
	  *
	  * @param pre_processor_ptr The pre-processor function object to register (ignored if empty)
	  */
    void registerPreProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>>
            pre_processor_ptr
    ) {
        if(pre_processor_ptr) {
            pre_processor_ptr_ = pre_processor_ptr;
        }
    }

    /***************************************************************************/
    /**
	  * @brief Allows to check whether any user-defined post-processing after the process()-
	  * step may occur. This may be important if e.g. an optimization algorithm wants
	  * to submit evaluation work items to the broker which may then start an optimization
	  * run on the individual. This may alter the individual's data.
	  *
	  * @return true if post-processing is currently allowed, false if it has been vetoed
	  */
    bool mayBePostProcessed() const {
        return not post_processing_disabled_;
    }

    /***************************************************************************/
    /**
	  * @brief Allow or prevent post-processing (used by post-processing algorithms to prevent
	  * recursive post-processing). See e.g. GEvolutionaryAlgorithmPostOptimizerT. Once a veto
	  * exists, no post-processing will occur until the veto is lifted.
	  *
	  * @param veto true to disable (veto) post-processing, false to allow it
	  */
    void vetoPostProcessing(bool veto) {
        post_processing_disabled_ = veto;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to register a post-processor object
	  *
	  * @param post_processor_ptr The post-processor function object to register (ignored if empty)
	  */
    void registerPostProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>>
            post_processor_ptr
    ) {
        if(post_processor_ptr) {
            post_processor_ptr_ = post_processor_ptr;
        }
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the registered post-processor (or an empty pointer). The optimization algorithm uses this
	  * at setup to decide -- from the post-processor's allowed mnemonics and its own mnemonic -- whether to
	  * veto post-processing on this work item, so the work item needs no knowledge of the algorithm.
	  *
	  * @return The registered post-processor, or an empty pointer if none is registered
	  */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>> postProcessor() const {
        return post_processor_ptr_;
    }

    /***************************************************************************/
    /**
	  * @brief Removes any registered post-processor. Used by a post-processing algorithm on the clone it
	  * optimizes, so that the sub-optimization's own population carries no post-processor and cannot
	  * recurse into further post-processing.
	  */
    void clearPostProcessor() {
        post_processor_ptr_.reset();
    }

    /***************************************************************************/
    /**
	  * @brief Allows to retrieve the number of stored results
	  *
	  * @return The number of result slots held by this object
	  */
    std::size_t getNStoredResults() const {
        return stored_results_cnt_.size();
    }

    /***************************************************************************/
    /**
	  * @brief Loads the data of another GProcessingContainerT<processable_type, processing_result_type> object
	  *
	  * @param cp A pointer to the source object whose data is copied into this one (must differ from this)
	  */
    void load_pc(const GProcessingContainerT<processable_type, processing_result_type> *cp) {
        // Check that we are dealing with a GProcessingContainerT<processable_type, processing_result_type> reference independent of this object and convert the pointer
        const auto *p_load =
            Gem::Common::g_convert_and_compare<
                GProcessingContainerT<processable_type, processing_result_type>,
                GProcessingContainerT<processable_type, processing_result_type>>(cp, this);

        // Load local data
        iteration_counter_ = p_load->iteration_counter_;
        resubmission_counter_ = p_load->resubmission_counter_;
        collection_position_ = p_load->collection_position_;
        correlation_id_ = p_load->correlation_id_;
        pre_processing_disabled_ = p_load->pre_processing_disabled_;
        post_processing_disabled_ = p_load->post_processing_disabled_;
        pre_processing_time_ = p_load->pre_processing_time_;
        processing_time_ = p_load->processing_time_;
        post_processing_time_ = p_load->post_processing_time_;
        broker_raw_submission_time_ = p_load->broker_raw_submission_time_;
        broker_raw_retrieval_time_ = p_load->broker_raw_retrieval_time_;
        broker_proc_submission_time_ = p_load->broker_proc_submission_time_;
        broker_proc_retrieval_time_ = p_load->broker_proc_retrieval_time_;
        stored_results_cnt_ =
            p_load
                ->stored_results_cnt_; // note that this implies that processing_result_type is copyable --> e.g. it should not contain pointers
        stored_error_descriptions_ = p_load->stored_error_descriptions_;
        processing_status_ = p_load->processing_status_;
        // evaluation_id_ = p_load->evaluation_id_;

        Gem::Common::copyCloneableSmartPointer(p_load->pre_processor_ptr_, pre_processor_ptr_);
        Gem::Common::copyCloneableSmartPointer(p_load->post_processor_ptr_, post_processor_ptr_);
    }


protected:
    /***************************************************************************/
    /**
	  * @brief Retrieval of the stored result. This function allows modifications of its
	  * return value and is hence protected and only accessible by derived classes.
	  *
	  * @param id The id (position) of the stored result to be returned
	  * @return A modifiable reference to the stored result at position id
	  */
    processing_result_type &modifyStoredResult(std::size_t id = 0) {
        return stored_results_cnt_.at(id);
    }

    /***************************************************************************/
    /**
	  * @brief Allows derived classes to set the number of stored results. Note that this
	  * should happen prior to any operation with this object. Also note that this
	  * operation may invalidate other results already stored in this object.
	  *
	  * @param n_stored_results The number of stored results in this class
	  * @param new_val A value to be copied into new positions when the vector is increased
	  */
    void setNStoredResults(std::size_t n_stored_results, processing_result_type new_val) {
        stored_results_cnt_.resize(n_stored_results, new_val);
    }

    /***************************************************************************/
    /**
	  * @brief Allows derived classes to set the number of stored results, default-initializing
	  * new positions. Note that this should happen prior to any operation with this object. Also
	  * note that this operation may invalidate other results already stored in this object.
	  *
	  * @param n_stored_results The number of stored results in this class
	  */
    void setNStoredResults(std::size_t n_stored_results) {
        processing_result_type p;
        this->setNStoredResults(n_stored_results, p);
    }

    /***************************************************************************/
    /**
	  * @brief Allows to register a result, using its id (i.e. position in the internal
	  * result storage). This function should be called from inside of the process_ call.
	  *
	  * @param id The id (position) at which to store the result
	  * @param r The result value to store at position id
	  */
    void registerResult(std::size_t id, const processing_result_type &r) {
        stored_results_cnt_.at(id) = r;
    }

    /***************************************************************************/
    /**
     * @brief The default constructor. It is only needed for (de-)serialization purposes.
     * We want to enforce the specification of the number of evaluation criteria
     * in derived classes. Protected, so that a derived class can have a defaulted
     * default constructor.
     */
    GProcessingContainerT() = default;

private:
    /***************************************************************************/
    /**
	  * @brief Little helper function to (re-)initialize the result storage vector
	  */
    void clear_stored_results_vec() {
        // "Nullify the result list. We cannot use range-based for here, as stored_results_cnt_ might hold booleans
        for(auto it = stored_results_cnt_.begin(); it != stored_results_cnt_.end(); ++it) {
            *it = processing_result_type();
        }
    }

    /***************************************************************************/
    /** @brief Bridges the GProcessable status machine to this class's result store: when a status reset
     *  clears stored results, GProcessable::set_processing_status() calls this hook. */
    void clearStoredResults_() override { this->clear_stored_results_vec(); }

    /***************************************************************************/
    /**
	  * @brief Loads user-specified data. This function can be overloaded by derived classes. It
	  * is mainly intended to provide a mechanism to "deposit" an item at a remote site
	  * that holds otherwise constant data. That data then does not need to be serialized
	  * but can be loaded whenever a new work item arrives and has been de-serialized. Note
	  * that, if your work items do not serialize important parts of an object, you need
	  * to make sure that constant data is loaded after reloading a checkpoint.
	  *
	  * @param cd_ptr A pointer to the object whose constant data should be loaded (unused in the default no-op)
	  */
    virtual void loadConstantData_([[maybe_unused]] std::shared_ptr<processable_type> cd_ptr) { /* nothing */
    }

    /***************************************************************************/
    /** @brief Hook: whether this item arrived without its input data (a results-only return). Default
     *  false; overridden by work-item types that support the lightweight return form.
     *  @return false in the base. */
    virtual bool inputDataOmitted_() const { return false; }

    /** @brief Hook: graft the input data of @p original onto this (results-only) item. Default no-op.
     *  @param original The originally-submitted item supplying the input data (unused in the default). */
    virtual void graftInputDataFrom_([[maybe_unused]] const processable_type &original) { /* nothing */ }

    /***************************************************************************/

    /** @brief Allows derived classes to specify the tasks to be performed for this object
     *  @param res_vec An optional externally injected evaluation result vector available to the implementation */
    virtual void process_(
        const std::vector<processing_result_type> &res_vec = std::vector<processing_result_type>()
    ) = 0;

    /***************************************************************************/
    /**
		  * @brief Specifies tasks to be performed before the process_ call. Note: This function
		  * will reset the mayBePreProcessed_-flag.
  		  */
    void preProcess_() {
        if(this->mayBePreProcessed() && pre_processor_ptr_) {
            auto &p = dynamic_cast<processable_type &>(*this);
            (*pre_processor_ptr_)(p);
        }
    }

    /***************************************************************************/
    /**
	  * @brief Specifies tasks to be performed after the process_ call. Note: This function
	  * will reset the mayBePostProcessed_-flag.
  	  */
    void postProcess_() {
        if(this->mayBePostProcessed() && post_processor_ptr_) {
            auto &p = dynamic_cast<processable_type &>(*this);
            (*post_processor_ptr_)(p);
        }
    }

    /***************************************************************************/
    // Data -- the result store and the (typed) pre-/post-processors. The non-generic lifecycle state
    // (status, errors, routing counters, dispatch-scheduling, timing) lives on the GProcessable base.

    bool pre_processing_disabled_ = false; ///< Indicates whether pre-processing was disabled entirely
    bool post_processing_disabled_ =
        false; ///< Indicates whether post-processing was disabled entirely

    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>>
        pre_processor_ptr_; ///< Actions to be performed before processing
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<processable_type>>
        post_processor_ptr_; ///< Actions to be performed after processing

    std::vector<processing_result_type> stored_results_cnt_ = std::vector<processing_result_type>(
        1,
        processing_result_type()
    ); ///< The results stored by this object

    // std::string evaluation_id_ = "empty"; ///< A unique id that is assigned to an evaluation
};

/******************************************************************************/

} /* namespace Gem::Courtier */

/******************************************************************************/
/** @brief Mark this class as abstract */
namespace boost::serialization {
template <typename processable_type, typename processing_result_type>
struct is_abstract<Gem::Courtier::GProcessingContainerT<processable_type, processing_result_type>>
  : public std::true_type {};
template <typename processable_type, typename processing_result_type>
struct is_abstract<
    const Gem::Courtier::GProcessingContainerT<processable_type, processing_result_type>>
  : public std::true_type {};
} /* namespace boost::serialization */

/******************************************************************************/
