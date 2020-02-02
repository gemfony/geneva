/**
 * @file
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
#include <string>
#include <chrono>

// Boost header files go here
#include <boost/filesystem.hpp>

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GFactoryT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "geneva/GPluggableOptimizationMonitors.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for optimization algorithms.
 */
template<typename oa_type>
class G_OptimizationAlgorithm_FactoryT
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
	 explicit G_OptimizationAlgorithm_FactoryT(
	 	boost::filesystem::path const &configFile
	 )
		 : Gem::Common::GFactoryT<oa_type>(configFile)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A constructor which adds a content creation function
	  */
	 G_OptimizationAlgorithm_FactoryT(
		 boost::filesystem::path const & configFile
		 , std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
	 )
		 : Gem::Common::GFactoryT<oa_type>(configFile)
			, m_contentCreatorPtr(contentCreatorPtr)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 G_OptimizationAlgorithm_FactoryT(const G_OptimizationAlgorithm_FactoryT<oa_type> &cp)
		 : Gem::Common::GFactoryT<oa_type>(cp)
			, m_contentCreatorPtr()
			, m_maxIterationCL(cp.m_maxIterationCL)
			, m_maxStallIterationCL(cp.m_maxStallIterationCL)
			, m_maxSecondsCL(cp.m_maxSecondsCL)
	 {
		 if (cp.m_contentCreatorPtr) {
			 if (m_contentCreatorPtr) {
				 m_contentCreatorPtr->load(cp.m_contentCreatorPtr);
			 } else {
				 m_contentCreatorPtr = cp.m_contentCreatorPtr->clone();
			 }
		 } else {
			 m_contentCreatorPtr.reset();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~G_OptimizationAlgorithm_FactoryT() override = default;

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
		 boost::program_options::options_description &visible
		 , boost::program_options::options_description &hidden
	 ) BASE {
		 namespace po = boost::program_options;

		 hidden.add_options()(
			 (this->getMnemonic() + std::string("MaxIterations")).c_str()
			 , po::value<std::int32_t>(&m_maxIterationCL)->default_value(-1)
			 , (std::string("\t[G_OptimizationAlgorithm_FactoryT / ") + this->getMnemonic() +
				 "] The maximum allowed number of iterations or 0 to disable limit").c_str()
		 )(
			 (this->getMnemonic() + std::string("MaxStallIterations")).c_str()
			 , po::value<std::int32_t>(&m_maxStallIterationCL)->default_value(-1)
			 , (std::string("\t[G_OptimizationAlgorithm_FactoryT / ") + this->getMnemonic() +
				 "] The maximum allowed number of stalled iterations or 0 to disable limit").c_str()
		 )(
			 (this->getMnemonic() + std::string("MaxSeconds")).c_str()
			 , po::value<std::int32_t>(&m_maxSecondsCL)->default_value(-1)
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
	 template<typename target_type>
	 std::shared_ptr<target_type> get() {
		 return Gem::Common::convertSmartPointer<oa_type, target_type>(Gem::Common::GFactoryT<oa_type>::get());
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a content creator
	  */
	 void registerContentCreator(
		 std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> cc_ptr
	 ) {
		 if (not cc_ptr) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GOptiomizationAlgorithmFactoryT<T>::registerContentCreator(): Error!" << std::endl
					 << "Tried to register an empty pointer" << std::endl
			 );
		 }

		 m_contentCreatorPtr = cc_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a pluggable optimization monitor
	  */
	 void registerPluggableOM(std::shared_ptr<GBasePluggableOM> pluggableOM) {
		 if (pluggableOM) {
			 m_pluggableOM = pluggableOM;
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In G_OptimizationAlgorithm_FactoryT<>::registerPluggableOM(): Tried to register empty pluggable optimization monitor" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset the local pluggable optimization monitor
	  */
	 void resetPluggableOM() {
		 m_pluggableOM.reset();
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
	 void setMaxIterationCL(std::uint32_t maxIterationCL) {
		 m_maxIterationCL = boost::numeric_cast<std::int32_t>(maxIterationCL);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether the maximum number of iterations was set on the command line or using the manual function
	  */
	 bool maxIterationsCLSet() const {
		 if (m_maxIterationCL >= 0) return true;
		 else return false;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the maximum number of iterations as set on the command line
	  */
	 std::uint32_t getMaxIterationCL() const {
		 if (m_maxIterationCL >= 0) {
			 return boost::numeric_cast<std::uint32_t>(m_maxIterationCL);
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In G_OptimizationAlgorithm_Base<>::getMaxIterationCL(): Error!" << std::endl
					 << "m_maxIterationCL wasn't set" << std::endl
			 );

			 // Make the compiler happy
			 return std::uint32_t(0);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to manually set the maximum number of stall iterations as is usually specified on the command line
	  */
	 void setMaxStallIterationCL(std::uint32_t maxStallIterationCL) {
		 m_maxStallIterationCL = boost::numeric_cast<std::int32_t>(maxStallIterationCL);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether the maximum number of stall iterations was set on the command line or using the manual function
	  */
	 bool maxStallIterationsCLSet() const {
		 if (m_maxStallIterationCL >= 0) return true;
		 else return false;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the maximum number of stall iterations as set on the command line
	  */
	 std::uint32_t getMaxStallIterationCL() const {
		 if (m_maxStallIterationCL >= 0) {
			 return boost::numeric_cast<std::uint32_t>(m_maxStallIterationCL);
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In G_OptimizationAlgorithm_Base<>::getMaxStallIterationCL(): Error!" << std::endl
					 << "m_maxStallIterationCL wasn't set" << std::endl
			 );

			 // Make the compiler happy
			 return std::uint32_t(0);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to manually set the maximum number of seconds for a run as is usually specified on the command line
	  */
	 void setMaxSecondsCL(std::uint32_t maxSecondsCL) {
		 m_maxSecondsCL = boost::numeric_cast<std::int32_t>(maxSecondsCL);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether the maximum number of seconds was set on the command line or using the manual function
	  */
	 bool maxSecondsCLSet() const {
		 if (m_maxSecondsCL >= 0) return true;
		 else return false;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the maximum number of seconds as set on the command line
	  */
	 std::chrono::duration<double> getMaxTimeCL() const {
		 if (m_maxSecondsCL >= 0) {
			 std::chrono::duration<double> maxDuration = std::chrono::seconds(boost::numeric_cast<long>(m_maxSecondsCL));
			 return maxDuration;
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In G_OptimizationAlgorithm_Base<>::getMaxTimeCL(): Error!" << std::endl
					 << "m_maxSecondsCL wasn't set" << std::endl
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
		if (m_contentCreatorPtr) { // Has a content creation object been registered ? If so, add individuals to the population
			for (std::size_t ind = 0; ind < p_alg->getDefaultPopulationSize(); ind++) {
				std::shared_ptr<GParameterSet> p_ind = (*m_contentCreatorPtr)();
				if (not p_ind) { // No valid item received, the factory has run empty
					break;
				} else {
					p_alg->push_back(p_ind);
				}
			}
		}

		// Check if any pluggable optimization monitor was registered. If so,
		// load it into the optimization algorithm
		if (m_pluggableOM) {
			p_alg->registerPluggableOM(m_pluggableOM);
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
	 virtual void describeLocalOptions_(
		 Gem::Common::GParserBuilder &gpb
	 ) override {
		 /* nothing */
	 }

	 /***************************************************************************/
	 /**
	  * Allows to act on the configuration options received from the configuration file or from the command line
	  */
	 void postProcess_(std::shared_ptr<oa_type> &p) override {
		 // Set local options

		 // The maximum allowed number of iterations
		 if (this->maxIterationsCLSet()) {
			 p->oa_type::setMaxIteration(this->getMaxIterationCL());
		 }

		 // The maximum number of stalls until operation stops
		 if (this->maxStallIterationsCLSet()) {
			 p->oa_type::setMaxStallIteration(this->getMaxStallIterationCL());
		 }

		 // The maximum amount of time until operation stops
		 if (this->maxSecondsCLSet()) {
			 p->oa_type::setMaxTime(this->getMaxTimeCL());
		 }
	 }

	 /***************************************************************************/

	 std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> m_contentCreatorPtr; ///< Holds an object capable of producing objects of the desired type
	 std::shared_ptr<GBasePluggableOM> m_pluggableOM; // A user-defined means for information retrieval

private:
	 /***************************************************************************/
	 /** @brief Creates individuals of this type */
	 std::shared_ptr<oa_type> getObject_(Gem::Common::GParserBuilder &, const std::size_t &) override = 0;

	 /***************************************************************************/

	 std::int32_t m_maxIterationCL = -1; ///< The maximum number of iterations. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
	 std::int32_t m_maxStallIterationCL = -1; ///< The maximum number of generations without improvement, after which optimization is stopped. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
	 std::int32_t m_maxSecondsCL = -1; ///< The maximum number of seconds for the optimization to run. NOTE: SIGNED TO ALLOW CHECK WHETHER PARAMETER WAS SET
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

