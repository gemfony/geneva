/**
 * @file GWorkerT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <functional>
#include <chrono>

// Boost headers go here
#include <boost/numeric/conversion/cast.hpp>

#ifndef GENEVA_LIBRARY_COLLECTION_GWORKERT_HPP
#define GENEVA_LIBRARY_COLLECTION_GWORKERT_HPP

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "courtier/GProcessingContainerT.hpp"

namespace Gem {
namespace Courtier {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The base class for a hierarchy of classes that unify the evaluation work
 * inside of consumers.
 */
template<class processable_type>
class GWorkerT {
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<Gem::Courtier::GProcessingContainerT<processable_type, typename processable_type::result_type>, processable_type>::value
		 , "GWorkerT: processable_type does not adhere to the GProcessingContainerT interface"
	 );

public:
	 /************************************************************************/
	 /** @brief The default constructor */
	 GWorkerT() = default;

	 /************************************************************************/
	 /**
	  * Initialization with the worker id (a consecutive, unique id to be
	  * supplied by the caller.
	  */
	 GWorkerT(std::size_t workerId) : m_worker_id(workerId)
	 { /* nothing */ }

protected:
	 /************************************************************************/
	 /**
	  * The copy constructor. We do not copy the worker id. It needs to be
	  * supplied by the caller. In order to avoid copying inside of the
	  * default version, we supply a custom copy constructor here.
	  */
	 GWorkerT(const GWorkerT<processable_type> &cp) { /* nothing */ }

public:
	 /************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GWorkerT() = default;

	 /************************************************************************/
	 /**
	  * Sets the worker id. This id may e.g. be used to let each worker act
	  * differently. E.g., the first worker may assume a different role than
	  * all the others. Note that we convert the external, unsigned type to
	  * an internal, signed representation, so we may detect worker ids that
	  * have not been initialized.
	  *
	  * @param The requested worker id
	  */
	 void setWorkerId(std::size_t workerId) {
		 m_worker_id = boost::numeric_cast<std::int32_t>(workerId);
	 }

	 /************************************************************************/
	 /**
	  * Retrieves the worker id. Calling this function prior to the initialization
	  * of the worker id will throw. "0" is an allowed value.
	  *
	  * @return The current worker id
	  */
	 std::size_t getWorkerId() const {
		 if(m_worker_id < 0) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GWorkerT<processable_type>::getWorkerId(): Error!" << std::endl
					 << "It appears as if the worker id was not set!" << std::endl
			 );
		 } else {
			 return boost::numeric_cast<std::size_t>(m_worker_id);
		 }
	 }

	 /************************************************************************/
	 /**
	  * Clones this object (or its derivatives). Each derivative must implement
	  * the clone_() function.
	  *
	  * @return A clone of this object (or its derivatives, camouflaged as a GWorkerT)
	  */
	 std::shared_ptr<GWorkerT<processable_type>> clone() const {
		 return this->clone_();
	 }

	 /************************************************************************/
	 /**
	  * The main entry point for the execution
	  */
	 void run() {
		 //---------------------------------------------------------------------
		 // For error descriptions
		 std::ostringstream error_streamer;
		 // Indicates whether an error was found
		 bool has_error = false;

		 //---------------------------------------------------------------------
		 // Some error checks
		 if(-1 == m_worker_id) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GWorkerT<processable_type>::run(): Error!" << std::endl
					 << "It appears as if the worker id was not set!" << std::endl
			 );
		 }

		 //---------------------------------------------------------------------
		 // The actual loop. While some error checks are already done in the
		 // process()-call, we also try to catch any other error that might occur,
		 // so we are sure to detect problems in user-code or our own code early.

		 try {
			 std::shared_ptr<processable_type> p;

			 // Any necessary setup work
			 this->processInit();

			 // The main loop
			 do {
				 // Retieve an item and check for its validity. Try again if
				 // we didn't receive a valid item
				 if(!(p=this->retrieve(m_retrieval_timeout))) { continue; }

				 // Initiate the actual processing
				 this->process(p);

				 // Return the item. Note that the submit function has the freedom
				 // to discard items if a submission is not possible.
				 this->submit(p, m_submission_timeout);

			 } while(!this->stop_requested());

			 // Perform any final work
			 this->processFinalize();

		 } catch (gemfony_exception& e) {
			 has_error = true;
			 error_streamer
				 << "In GWorkerT<processable_type>::run(): Caught gemfony_exception with message" << std::endl
				 << e.what() << std::endl;
		 } catch (std::exception &e) {
			 has_error = true;
			 error_streamer
				 << "In GWorkerT<processable_type>::run():" << std::endl
				 << "Caught std::exception with message" << std::endl
				 << e.what() << std::endl;
		 }
		 catch (boost::exception &e) {
			 has_error = true;
			 error_streamer
				 << "In GWorkerT<processable_type>::run():" << std::endl
				 << "Caught boost::exception with message" << std::endl
				 << boost::diagnostic_information(e) << std::endl;
		 }
		 catch (...) {
			 has_error = true;
			 error_streamer
				 << "In GWorkerT<processable_type>::run():" << std::endl
				 << "Caught unknown exception." << std::endl;
		 }

		 //---------------------------------------------------------------------
		 // Make it known if there was a problem
		 if(has_error) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place) << error_streamer.str()
			 );
		 }

		 //---------------------------------------------------------------------
	 }

	 /************************************************************************/
	 /**
	  * Parses a given configuration file
	  *
	  * @param configFile The name of a configuration file
	  */
	 void parseConfigFile(const std::string &configFile) {
		 // Create a parser builder object -- local options will be added to it
		 Gem::Common::GParserBuilder gpb;

		 // Add configuration options of this and of derived classes
		 addConfigurationOptions(gpb);

		 // Do the actual parsing. Note that this
		 // will try to write out a default configuration file,
		 // if no existing config file can be found
		 gpb.parseConfigFile(configFile);
	 }

	 /************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object. This function
	  * only relays to our local implementation, which may be overridden in derived
	  * classes. Note that the overriding function should take care to call the parent's
	  * addConfigurationOptions_() function.
	  *
	  * @param gpb The GParserBuilder object, to which configuration options will be added
	  */
	 virtual void addConfigurationOptions(
		 Gem::Common::GParserBuilder &gpb
	 ) {
		 this->addConfigurationOptions_(gpb);
	 }

protected:
	 /************************************************************************/
	 /**
	  * The actual implementation of adding configuration options. "protected",
	  * so it may be called by derived classes.
	  *
	  * @param gpb The GParserBuilder object, to which configuration options will be added
	  */
	 virtual void addConfigurationOptions_(
		 Gem::Common::GParserBuilder &gpb
	 ) { /* nothing -- no local data */ }

private:
	 /************************************************************************/
	 /**
	  * Initialization code for processing. Specify custom code in the virtual
	  * processInit_ function.
	  */
	 void processInit() {
		 this->processInit_();
	 }

	 /************************************************************************/
	 /**
	  * Single-pass processing of work items. Specify custom code for processing
	  * in the virtual process_ function.
	  */
	 void process(std::shared_ptr <processable_type> p) {
		 // GProcesingContainerT-derivatives may emit a g_processing_exception if
		 // they have detected a problem in the actual processing code. This will
		 // usually invalidate the work item, but not the entire application. Hence
		 // we capture this exception here and leave it to other recipients of the
		 // work item to decide on its fate.
		 try {
			 this->process_(p);
		 } catch(const g_processing_exception& e) {
			 glogger
				 << "In GWorkerT<processable_type>::process()"
				 << "The work item has flagged a processing exception with the message" << std::endl
				 << e << std::endl
				 << "The item will be returned. It is up to the recipient of the work item" << std::endl
				 << "to decide on its fate" << std::endl
				 << GWARNING;
		 }
	 }

	 /************************************************************************/
	 /**
	  * Finalization code for processing. Specify the actual work in derived
	  * classes by overriding processFinalize_()
	  */
	 void processFinalize() {
		 this->processFinalize_();
	 }

	 /************************************************************************/
	 /**
	  * Retrieval of work items
	  */
	 std::shared_ptr<processable_type> retrieve(const std::chrono::milliseconds& timeout) {
		 return this->retrieve_(timeout);
	 }

	 /************************************************************************/
	 /**
	  * Submission of work items
	  */
	 void submit(
		 std::shared_ptr<processable_type> item_ptr
		 , const std::chrono::milliseconds& timeout
	 ) {
		 this->submit_(item_ptr, timeout);
	 }

	 /************************************************************************/
	 /**
	  * Indicates whether the worker was asked to stop processing
	  */
	 bool stop_requested() const {
		 return this->stop_requested_();
	 }

	 /************************************************************************/
	 // Some purely virtual functions, to be implemented in derived classes

	 /** @brief Creation of deep clones of this object('s derivatives) */
	 virtual std::shared_ptr<GWorkerT<processable_type>> clone_() const BASE = 0;
	 /** @brief Initialization code for processing. */
	 virtual void processInit_() BASE = 0;
	 /** @brief Actual per-item work is done here -- all error-detection instrumentation is done in the protected "process() function. */
	 virtual void process_(std::shared_ptr <processable_type> p) BASE = 0;
	 /** @brief Finalization code for processing. */
	 virtual void processFinalize_() BASE = 0;
	 /** @brief Retrieval of work items */
	 virtual std::shared_ptr<processable_type> retrieve_(const std::chrono::milliseconds&) BASE = 0;
	 /** @brief Submission of work items */
	 virtual void submit_(std::shared_ptr<processable_type>, const std::chrono::milliseconds&) BASE = 0;
	 /** @brief Indicates whether the worker was asked to stop processing */
	 virtual bool stop_requested_() const BASE = 0;

	 /************************************************************************/
	 // Data

	 const std::chrono::milliseconds m_submission_timeout = std::chrono::milliseconds(200); ///< Timeout for submit operations
	 const std::chrono::milliseconds m_retrieval_timeout  = std::chrono::milliseconds(200); ///< Timeout for retrieval operations

	 std::int32_t m_worker_id = -1; ///< The id of the thread running this class'es operator()

	 /************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class acts as a little helper, holding information and functions
 * needed for retrieving and submitting work items as well as termination.
 */
template<class processable_type>
class GBrokerFerryT {
public:
	 /************************************************************************/
	 /**
	  * Initialization is only allowed with a single constructor, so we only need
	  * to check for content once.
	  */
	 GBrokerFerryT(
		 const std::size_t& worker_id
		 , std::function<std::shared_ptr<processable_type>(const std::chrono::milliseconds&)> retriever
		 , std::function<void(std::shared_ptr<processable_type>, const std::chrono::milliseconds&)> submitter
		 , std::function<bool()> stop_requested
	 )
		 : m_worker_id(worker_id)
		 , m_retriever(retriever)
		 , m_submitter(submitter)
		 , m_stop_requested(stop_requested)
	 {
		 if(!m_retriever) {
			 glogger
				 << "In GLocalConsumerWorkerT<processable_type>::GBrokerFerryT(): Error!" << std::endl
				 << "Empty retriever function found!" << std::endl
				 << "We cannot continue" << std::endl
				 << GTERMINATION;
		 }

		 if(!m_submitter) {
			 glogger
				 << "In GLocalConsumerWorkerT<processable_type>::GBrokerFerryT(): Error!" << std::endl
				 << "Empty submitter function found!" << std::endl
				 << "We cannot continue" << std::endl
				 << GTERMINATION;
		 }

		 if(!m_stop_requested) {
			 glogger
				 << "In GLocalConsumerWorkerT<processable_type>::GBrokerFerryT(): Error!" << std::endl
				 << "Empty termination function found!" << std::endl
				 << "We cannot continue" << std::endl
				 << GTERMINATION;
		 }
	 }

	 /************************************************************************/
	 /**
	  * Retrieval of work items
	  */
	 std::shared_ptr<processable_type> retrieve(const std::chrono::milliseconds& timeout) {
		 return this->m_retriever(timeout);
	 }

	 /************************************************************************/
	 /**
	  * Submission of work items
	  */
	 void submit(
		 std::shared_ptr<processable_type> item_ptr
		 , const std::chrono::milliseconds& timeout
	 ) {
		 return this->m_submitter(item_ptr, timeout);
	 }

	 /************************************************************************/
	 /**
	  * Indicates whether the worker was asked to stop processing
	  */
	 bool stop_requested() const {
		 return this->m_stop_requested();
	 }

	 /************************************************************************/
	 /**
	  * Access to the worker id
	  */
	 std::size_t getWorkerId() const {
		 return m_worker_id;
	 }

private:
	 /************************************************************************/
	 // Some deleted functions -- this class is non-copyable
	 GBrokerFerryT() = delete;
	 GBrokerFerryT(const GBrokerFerryT<processable_type>&) = delete;
	 GBrokerFerryT(GBrokerFerryT<processable_type>&&) = delete;
	 const GBrokerFerryT<processable_type>& operator=(const GBrokerFerryT<processable_type>&) = delete;
	 GBrokerFerryT<processable_type>& operator=(GBrokerFerryT<processable_type>&&) = delete;

	 /************************************************************************/
	 // Data and stored functions

	 std::size_t m_worker_id = 0; ///< An id to be assigned to a worker

	 std::function<std::shared_ptr<processable_type>(const std::chrono::milliseconds&)> m_retriever; ///< Retrieval of new work item
	 std::function<void(std::shared_ptr<processable_type>, const std::chrono::milliseconds&)> m_submitter; ///< Submission of processed work items
	 std::function<bool()> m_stop_requested; ///< Termination of the exeecution run

	 /************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Unifies the processing of work items inside of consumers that do not
 * submit work for processing to a remote location.
 */
template<class processable_type>
class GLocalConsumerWorkerT : public GWorkerT<processable_type>
{
public:
	 /************************************************************************/
	 /** @brief The default constructor */
	 GLocalConsumerWorkerT() = default;

protected:
	 /************************************************************************/
	 /**
	  * The copy constructor.
	  */
	 GLocalConsumerWorkerT(const GLocalConsumerWorkerT<processable_type> &cp)
	 	: GWorkerT<processable_type>(cp)
	 { /* nothing */ }

public:
	 /************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GLocalConsumerWorkerT() = default;

	 /************************************************************************/
	 /**
	  * Allows to register a container object for various information
	  * needed by this class
	  */
	 void registerBrokerFerry(
		 std::shared_ptr<GBrokerFerryT<processable_type>> broker_ferry_ptr
	 ){
		 if(!broker_ferry_ptr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GLocalConsumerWorkerT<processable_type>::registerBrokerFerry(): Error!" << std::endl
					 << "Empty broker ferry object found!" << std::endl
			 );
		 }

		 m_broker_ferry_ptr = broker_ferry_ptr;

		 // Set the worker id immediately, so the run function does not stumble on
		 // an invalid value		 // Extract and set the worker id
		 this->setWorkerId(m_broker_ferry_ptr->getWorkerId());
	 }

private:
	 /************************************************************************/
	 /**
	  * Creation of deep clones of this object. Note that a new broker ferry
	  * needs to be registered with this object.
	  */
	 virtual std::shared_ptr<GWorkerT<processable_type>> clone_() const override {
		 return std::shared_ptr<GWorkerT<processable_type>>(new GLocalConsumerWorkerT<processable_type>(*this));
	 }

	 /************************************************************************/
	 /**
	  * Initialization code for processing.
	  */
	 virtual void processInit_() override {
		 if(!m_broker_ferry_ptr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GLocalConsumerWorkerT<processable_type>::processInit_(): Error!" << std::endl
					 << "Empty broker ferry object found!" << std::endl
			 );
		 }
	 }

	 /************************************************************************/
	 /**
	  * Only actual per-item work is done here -- Error-detection instrumentation
	  * is done in the protected "process() function of our parent class.
	  */
	 virtual void process_(std::shared_ptr<processable_type> p) override {
		 p->process();
	 }

	 /************************************************************************/
	 /**
	  * Finalization code for processing.
	  */
	 virtual void processFinalize_() override { /* nothing */ }

	 /************************************************************************/
	 /** @brief Retrieval of work items */
	 virtual std::shared_ptr<processable_type> retrieve_(
		 const std::chrono::milliseconds& timeout
	 ) override {
		 return this->m_broker_ferry_ptr->retrieve(timeout);
	 }

	 /************************************************************************/
	 /** @brief Submission of work items */
	 virtual void submit_(
		 std::shared_ptr<processable_type> p
		 , const std::chrono::milliseconds& timeout
	 ) override {
		 this->m_broker_ferry_ptr->submit(p, timeout);
	 }

	 /************************************************************************/
	 /** @brief Indicates whether the worker was asked to stop processing */
	 virtual bool stop_requested_() const override {
		 return this->m_broker_ferry_ptr->stop_requested();
	 }

	 /************************************************************************/
	 // Data

	 std::shared_ptr<GBrokerFerryT<processable_type>> m_broker_ferry_ptr; ///< A pointer to a container object holding information needed by this class
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GENEVA_LIBRARY_COLLECTION_GWORKERT_HPP */
