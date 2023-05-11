/**
 * @file GOpenCLWorkerT.hpp
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

// Standard headers go here
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cast.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

// OpenCL includes
#define CL_HPP_ENABLE_EXCEPTIONS // This will force OpenCL C++ classes to raise exceptions rather than to use an error code
#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_MINIMUM_OPENCL_VERSION 300
#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp" // Use the file in our local directory -- cl.hpp is not delivered by default on MacOS X
#else
#include <CL/opencl.hpp>
#endif

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GParserBuilder.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GStdThreadConsumerT.hpp"
#include "courtier/GWorkerT.hpp"

namespace Gem::Courtier {

/** @brief A default value for the Open CL code file */
const std::string GOCLWT_DEF_CODEFILE = "./code/default.cl";
/** @brief A default value fot the number of entries in a work group */
const int GOCLWT_DEF_WGS = 192;

/******************************************************************************/
/**
 * A worker class for the GStdThreadConsumerT, targeted at OpenCL work. Derived
 * classes particularly need to implement the process_() function, where all task-
 * specific work may take place. This class is purely virtual and cannot be
 * instantiated directly.
 */
template<typename processable_type>
class GOpenCLWorkerT
	: public GLocalConsumerWorkerT<processable_type>
{
public:
	 /***************************************************************************/
	 /**
	  * Initialization with an external OpenCL context and the name of a
	  * configuration file.
	  *
	  * @param A context that was provided by an external entity
	  */
	 GOpenCLWorkerT(
		 const cl::Device &device
		 , const std::string &configFile
	 )
		 : GLocalConsumerWorkerT<processable_type>()
		 , m_device(1, device)
		 , m_context(m_device)
		 , m_queue(m_context, m_device[0], CL_QUEUE_PROFILING_ENABLE)
		 , m_configFile(configFile)
		 , m_codeFile()
		 , m_workGroupSize(0)
	 { /* nothing */ }

protected:
	 /***************************************************************************/
	 /**
	  * Initialization with a copy of another GOpenCLWorkerT object, a thread id
	  * and a pointer to a GStdThreadConsumerT<processable_type> (or a derivative
	  * thereof). The constructor is protected, as it is only intended to be used
	  * from the clone_() function of this class and from derived constructors.
	  */
	 GOpenCLWorkerT(
		 const GOpenCLWorkerT<processable_type> &cp
	 )
		 : GLocalConsumerWorkerT<processable_type>(cp)
		 , m_device(cp.m_device)
		 , m_context(m_device)
		 , m_queue(m_context, m_device[0], CL_QUEUE_PROFILING_ENABLE)
		 , m_configFile(cp.m_configFile)
		 , m_codeFile(cp.m_codeFile)
		 , m_workGroupSize(cp.m_workGroupSize)
	 // Many variables are further initialized in initOpenCLProgram() via processInit()
	 { /* nothing */ }

public:
	 /***************************************************************************/
	 /**
	  * The destructor
	  */
     ~GOpenCLWorkerT() override = default;

     /** @brief The default constructor: Intentionally undefined */
     GOpenCLWorkerT() = delete;
     /** @brief The assignment operator: Intentionally undefined */
     const GOpenCLWorkerT<processable_type> &operator=(const GOpenCLWorkerT<processable_type> &) = delete;

protected:
	 /************************************************************************/
	 /**
	  * Allows derived classes to perform any initialization work required
	  * prior to building the program objects. Particularly, it is possible
	  * to set up the data needed for the OpenCL compiler options.
	  */
	 virtual void initOpenCL(std::shared_ptr<processable_type> p) BASE
	 { /* nothing */ }

	 /************************************************************************/
	 /**
	  * Initialization of kernel objects
	  */
	 virtual void initKernels(std::shared_ptr<processable_type> p) BASE
	 { /* nothing */ }

	 /************************************************************************/
	 /**
	  * Initialization code for processing.
	  */
     void processInit_(std::shared_ptr<processable_type> p) override {
         // Make sure our parent class gets initialized
         GLocalConsumerWorkerT<processable_type>::processInit_(p);

         // TODO: This function seems to be called twice, also in the derived class
		 // Load local options
		 this->parseConfigFile(m_configFile);

		 // Perform preparatory work needed for the compilation of the OpenCL program
		 this->initOpenCL(p);

		 // Load the OpenCL code and compile it as needed
		 this->initOpenCLProgram();

		 // Initialization of kernel objects
		 this->initKernels(p);
	 }

	 /************************************************************************/
	 /**
	  * Finalization code for processing.
	  */
	 void processFinalize_() override {
         // Make sure our parent class's function is called
         GLocalConsumerWorkerT<processable_type>::processFinalize_();
     }

	 /***************************************************************************/
	 /**
	  * Emits compiler options for OpenCL
	  */
	 [[nodiscard]] virtual std::string getCompilerOptions() const
	 {
		 std::string compilerOptions =
			 " -DWORKGROUPSIZE=" + Gem::Common::to_string(m_workGroupSize) + " -cl-fast-relaxed-math";
		 return compilerOptions;
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  */
     void addConfigurationOptions_(
         Gem::Common::GParserBuilder &gpb
     ) override {
         // Make sure any options from our parent class are processed
         GLocalConsumerWorkerT<processable_type>::addConfigurationOptions_(gpb);

         // Add local options
		 std::string comment;

		 comment = "";
		 comment += "The name of the file holding the OpenCL code;";
		 gpb.registerFileParameter<std::string>(
			 "codeFile"
			 , m_codeFile
			 , GOCLWT_DEF_CODEFILE
			 , Gem::Common::VAR_IS_ESSENTIAL
			 , comment
		 );

		 comment = "";
		 comment += "The size of each work group;";
		 gpb.registerFileParameter<std::size_t>(
			 "workGroupSize"
			 , m_workGroupSize
			 , GOCLWT_DEF_WGS
			 , Gem::Common::VAR_IS_ESSENTIAL
			 , comment
		 );
	 }

	 /***************************************************************************/
	 /**
	  * A utility function that calculates the time needed for running a given
	  * OpenCL command
	  */
	 [[nodiscard]] double duration(const cl::Event &e) const
	 {
		 auto ev_start_time = (cl_ulong) 0;
		 auto ev_end_time = (cl_ulong) 0;
		 e.getProfilingInfo(
			 CL_PROFILING_COMMAND_QUEUED
			 , &ev_start_time
		 );
		 e.getProfilingInfo(
			 CL_PROFILING_COMMAND_END
			 , &ev_end_time
		 );
		 return (double) (ev_end_time - ev_start_time) * 1.0e-9;
	 }

	 /***************************************************************************/
	 /**
	  * The device we are supposed to act on. It is stored in a std::vector for
	  * simplicity reasons, so we can more easily initialize the context_ .
	  */
	 const std::vector<cl::Device> m_device;
	 cl::Context m_context; ///< The OpenCL context the class should act on
	 cl::CommandQueue m_queue; ///< A queue that is attached to a specific device

	 std::string m_configFile; ///< The name of a configuration file
	 std::string m_codeFile; ///< The file holding the OpenCL code
	 std::size_t m_workGroupSize; ///< The number of items in each work group

	 cl::Program::Sources m_source; ///< The program sources
	 cl::Program m_program; ///< The actual program object

	 cl::Event m_event; ///< Synchronization in the OpenCL context

private:
	 /***************************************************************************/
	 /**
	  * Initializes the OpenCL stack
	  */
	 void initOpenCLProgram()
	 {
		 try {
			 // Initialize the program object
             m_source = Gem::Common::loadTextLinesFromFile(m_codeFile);
			 m_program = cl::Program(
				 m_context
				 , m_source
			 );
			 m_program.build(
				 m_device
				 , (this->getCompilerOptions()).c_str());
		 } catch (cl::Error &err) {
			 std::cerr << "Error! " << err.what() << std::endl;
			 exit(1);
		 } catch (gemfony_exception &err) {
			 std::cerr << "Error! " << err.what() << std::endl;
			 exit(1);
		 }
	 }

     /** @brief Creation of deep clones of this object('s derivatives) */
     virtual std::shared_ptr<GWorkerT<processable_type>> clone_() const BASE = 0;
};

/******************************************************************************/

} /* namespace Gem::Courtier */

