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

// Standard headers go here

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <type_traits>
#include <functional>
#include <memory>
#include <atomic>

// Boost headers go here
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_raw.hpp>
#include <boost/spirit/include/qi_hold.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

// Geneva headers go here
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "courtier/GProcessingContainerT.hpp"

namespace Gem::Courtier {

/******************************************************************************/
/**
 * This class forms the basis of a hierarchy of classes designed for client-side
 * network communication. Their task is to retrieve serialized objects from the server
 * over a given protocol (implemented in derived classes), to instantiate the
 * corresponding object, to process it and to deliver the results to the server.
 * This class assumes that the template parameter implements the "process()" call.
 *
 * TODO: Identify this client with a UUID
 */
template<typename processable_type>
class GBaseClientT
{
	 // Make sure processable_type adheres to the GProcessingContainerT interface
	 static_assert(
		 std::is_base_of<
			 Gem::Courtier::GProcessingContainerT<processable_type, typename processable_type::result_type>
			 , processable_type
		 >::value
		 , "GBaseClientT: processable_type does not adhere to the GProcessingContainerT interface"
	 );

public:
	 //---------------------------------------------------------------------------
	 /**
	  * A constructor that accepts a model of the item to be processed. This can be
	  * used to avoid having to transfer or reload data that doesn't change. Note that
	  * the model must understand the clone() command.
	  *
	  * @param additionalDataTemplate The model of the item to be processed
	  */
	 explicit GBaseClientT(std::shared_ptr<processable_type> additionalDataTemplate)
		 : m_additionalDataTemplate(additionalDataTemplate)
	 { /* nothing*/ }

	 //---------------------------------------------------------------------------
	 // Defaulted or Deleted constructors and assignment operators

	 GBaseClientT() = default;

	 GBaseClientT(const GBaseClientT<processable_type>&) = delete;
	 GBaseClientT(GBaseClientT<processable_type>&&) = delete;

	virtual ~GBaseClientT() BASE = default;

	 GBaseClientT<processable_type>& operator=(const GBaseClientT<processable_type>&) = delete;
	 GBaseClientT<processable_type>& operator=(GBaseClientT<processable_type>&&) = delete;

	 //---------------------------------------------------------------------------
	 /**
	  * Allows to set a maximum number of processing steps. If set to 0 or left unset,
	  * processing will be done until process() returns false.
	  *
	  * @param processMax Desired value for the m_processMax variable
	  */
	 void setProcessMax(std::uint32_t processMax) {
		 m_processMax = processMax;
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Retrieves the value of the m_processMax variable.
	  *
	  * @return The value of the m_processMax variable
	  */
	 [[nodiscard]] std::uint32_t getProcessMax() const {
		 return m_processMax;
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Retrieves the number of items processed so far
	  */
	 [[nodiscard]] std::uint32_t getNProcessed() const {
		 return m_processed;
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Sets the maximum allowed processing time
	  *
	  * @param maxDuration The maximum allowed processing time
	  */
	 void setMaxTime(const std::chrono::duration<double> &maxDuration) {
		 m_maxDuration = maxDuration;
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Retrieves the value of the m_maxDuration parameter.
	  *
	  * @return The maximum allowed processing time
	  */
	 std::chrono::duration<double> getMaxTime() {
		 return m_maxDuration;
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Checks whether a terminal error was flagged
	  */
	 [[nodiscard]] bool terminalErrorFlagged() const {
		 return m_terminalError.load();
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Checks whether the close-flag was set
	  */
	 [[nodiscard]] bool closeRequested() const {
		 return m_closeRequested.load();
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * The main initialization code
	  */
	 void run() {
		 run_state r = run_state::INIT;

		 //------------------------------------------------------------------------
		 // init section
		 try {
			 r = run_state::INIT;

			 if (not this->init()) { // Initialize the client
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GBaseClientT<T>::run(): Initialization failed. Leaving ..." << std::endl
				 );
			 }

			 r = run_state::RUN;
			 run_(); // The main loop

			 r = run_state::FINALLY;
			 if (not this->finally()) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GBaseClientT<T>::run(): Finalization failed." << std::endl
				 );
			 }
		 } catch (gemfony_exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG,  time_and_place)
					 << "In GBaseClientT<T>::run() / " << rs_to_str(r) << ":" << std::endl
					 << "Caught gemfony_exception" << std::endl
					 << "with message" << std::endl
					 << e.what()
			 );
		 } catch (boost::exception& e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBaseClientT<T>::run() / " << rs_to_str(r) << ": Caught boost::exception with message" << std::endl
					 << boost::diagnostic_information(e) << std::endl
			 );
		 } catch (std::exception &e) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBaseClientT<T>::run() / " << rs_to_str(r) << ": Caught std::exception with message" << std::endl
					 << e.what() << std::endl
			 );
		 } catch (...) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GBaseClientT<T>::run() / " << rs_to_str(r) << ": Caught unknown exception." << std::endl
			 );
		 }

		 //------------------------------------------------------------------------
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Allows to set a flag indicating that the application should terminate
	  */
	 void flagCloseRequested() {
		 m_closeRequested.store(true);
	 }

protected:
	 //---------------------------------------------------------------------------
	 /**
	  * Increment of the processing counter. We do not use atomics, as only
	  * one processing step is supposed to run at the same time
	  */
	 void incrementProcessingCounter() {
		 m_processed++;
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Allows to flag an error that qualifies as a halt condition
	  */
     void flagTerminalError() {
		 m_terminalError.store(true);
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Loads the additional data template into the processable_type target.
	  * This function needs to be called for each new item by derived classes.
	  */
     void loadDataTemplate(std::shared_ptr<processable_type> target) {
		 // If we have a model for the item to be parallelized, load its data into the target
		 if(m_additionalDataTemplate) {
			 target->loadConstantData(m_additionalDataTemplate);
		 }
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Checks whether a halt condition was reached.
	  *
	  * @return A boolean indicating whether a halt condition was reached
	  */
	 bool halt() {
		 // Has a terminal error been flagged?
		 if(terminalErrorFlagged()) {
			 glogger
				 << "Client is terminating because an unrecoverable error was flagged" << std::endl
				 << GLOGGING;

			 return true;
		 }

		 // Has the application been asked to shut down?
		 if(closeRequested()) {
			 glogger
				 << "Client is terminating because the application was asked to shut down" << std::endl
				 << GLOGGING;

			 return true;
		 }

		 // Maximum number of processing steps reached ?
		 if (m_processMax > 0 && (m_processed >= m_processMax)) {
			 glogger
				 << "Client is terminating because the maximum number of processing steps was exceeded" << std::endl
				 << GLOGGING;

			 return true;
		 }

		 // Maximum duration reached ?
		 if (m_maxDuration.count() > 0. && ((std::chrono::high_resolution_clock::now() - m_startTime) >= m_maxDuration)) {
			 glogger
				 << "Client is terminating because the maximum time frame was exceeded" << std::endl
				 << GLOGGING;

			 return true;
		 }

		 // Custom halt condition reached ?
		 if (customHalt()) {
			 glogger
				 << "Client is terminating because custom halt condition has triggered" << std::endl
				 << GLOGGING;

			 return true;
		 }

		 return false;
	 }

	 //---------------------------------------------------------------------------
	 /**
	  * Parses an in-bound "idle" command string, so we know how long the client
	  * should wait before reconnecting to the server. The idle command will be
	  * of the type "idle(5000)", where the number specifies the amount of
	  * milliseconds the client should wait before reconnecting.
	  */
     bool parseIdleCommand(std::uint32_t &idleTime, const std::string &idleCommand) {
		 using boost::spirit::ascii::space;
		 using boost::spirit::qi::phrase_parse;
		 using boost::spirit::qi::uint_;
		 using boost::spirit::qi::lit;
		 using boost::spirit::ascii::string;
		 using boost::spirit::lexeme;
		 using boost::spirit::qi::attr;

		 using boost::spirit::qi::raw;
		 using boost::spirit::qi::alpha;
		 using boost::spirit::qi::alnum;
		 using boost::spirit::qi::hold;

		 using boost::spirit::qi::_1;

		 bool success = false;

		 std::string::const_iterator from = idleCommand.begin();
		 std::string::const_iterator to = idleCommand.end();

		 success = phrase_parse(
			 from
			 , to
			 , (lit("idle") >> '(' >> uint_ >> ')')[boost::phoenix::ref(idleTime)=_1]
			 , space
		 );

		 return (success && (from==to));
	 }

private:
	 //---------------------------------------------------------------------------
	 /** @brief Performs initialization work */
	 virtual bool init() BASE { return true; }

	 //---------------------------------------------------------------------------
	 /** @brief This is the main loop of the client, after initialization */
	 virtual void run_() BASE = 0;

	 //---------------------------------------------------------------------------
	 /** @brief Perform necessary finalization activities */
	 virtual bool finally() BASE { return true; }

	 //---------------------------------------------------------------------------
	 /** @brief Custom halt condition for processing */
	 virtual bool customHalt() BASE { return false; }

	 //---------------------------------------------------------------------------
	 /** brief Transformation of run_state to a string */
	 std::string rs_to_str(run_state r) {
		 switch(r) {
			 case run_state::INIT:
				 return "run_state::INIT";
				 /* break; */

			 case run_state::RUN:
				 return "run_state::RUN";
                 /* break; */

			 case run_state::FINALLY:
				 return "run_state::FINALLY";
                 /* break; */
		 }

		 // Make the compiler happy
		 return std::string();
	 }

	 //---------------------------------------------------------------------------
	 // Data

	 std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now(); ///< Used to store the start time of the optimization
	 std::chrono::duration<double> m_maxDuration = std::chrono::microseconds(0); ///< Maximum time frame for the optimization

	 std::uint32_t m_processed = 0; ///< The number of processed items so far
	 std::uint32_t m_processMax = 0; ///< The maximum number of items to process

	 std::atomic<bool> m_terminalError{false}; ///< Indicates whether a terminal error was received
	 std::atomic<bool> m_closeRequested{false}; ///< Indicates whether a the termination was requested by the server

	 std::shared_ptr<processable_type> m_additionalDataTemplate; ///< Optionally holds a template of the object to be processed

	 //---------------------------------------------------------------------------
};

/******************************************************************************/


} // namespace Gem::Courtier

