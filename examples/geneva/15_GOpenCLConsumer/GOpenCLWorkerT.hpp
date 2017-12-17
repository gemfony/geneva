/**
 * @file GOpenCLWorkerT.hpp
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

#ifndef GOPENCLWORKER_HPP_
#define GOPENCLWORKER_HPP_

// Standard headers go here
#include <string>
#include <stdio.h>
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
#define __CL_ENABLE_EXCEPTIONS // This will force OpenCL C++ classes to raise exceptions rather than to use an error code
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp" // Use the file in our local directory -- cl.hpp is not delivered by default on MacOS X
#else
#include <CL/cl.hpp>
#endif

// Geneva headers go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GParserBuilder.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GBaseConsumerT.hpp"
#include "courtier/GStdThreadConsumerT.hpp"

namespace Gem {
namespace Courtier {

/** @brief A default value for the Open CL code file */
const std::string GOCLWT_DEF_CODEFILE = "./code/default.cl";
/** @brief A default value fot the number of entries in a work group */
const int GOCLWT_DEF_WGS = 192;

/******************************************************************************/
/**
 * A worker class for the GStdThreadConsumerT, targeted at OpenCL work. Derived
 * classes particularly need to implement the process() function, where all task-
 * specific work may take place. This class is purely virtual and cannot be
 * instantiated directly.
 */
template<typename processable_type>
class GOpenCLWorkerT
	: public GStdThreadConsumerT<processable_type>::GWorker
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
		 : GStdThreadConsumerT<processable_type>::GWorker()
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
	  * from the clone() function of this class and from derived constructors.
	  */
	 GOpenCLWorkerT(
		 const GOpenCLWorkerT<processable_type> &cp
		 , const std::size_t &thread_id
		 , const GStdThreadConsumerT <processable_type> *c_ptr
	 )
		 : GStdThreadConsumerT<processable_type>::GWorker(cp, thread_id, c_ptr)
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
	 virtual ~GOpenCLWorkerT()
	 { /* nothing */ }

protected:
	 /************************************************************************/
	 /**
	  * Allows derived classes to perform any initialization work required
	  * prior to building the program objects. Particularly, it is possible
	  * to set up the data needed for the OpenCL compiler options.
	  */
	 virtual void initOpenCL(std::shared_ptr<processable_type> p)
	 { /* nothing */ }

	 /************************************************************************/
	 /**
	  * Initialization of kernel objects
	  */
	 virtual void initKernels(std::shared_ptr<processable_type> p)
	 { /* nothing */ }

	 /************************************************************************/
	 /**
	  * Initialization code for processing. Can be specified in derived classes.
	  */
	 virtual void processInit(std::shared_ptr<processable_type> p)
	 {
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
	  * Finalization code for processing. Can be specified in derived classes.
	  */
	 virtual void processFinalize()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Emits compiler options for OpenCL
	  */
	 virtual std::string getCompilerOptions() const
	 {
		 std::string compilerOptions =
			 " -DWORKGROUPSIZE=" + Gem::Common::to_string(m_workGroupSize) + " -cl-fast-relaxed-math";
		 return compilerOptions;
	 }

	 /***************************************************************************/
	 /**
	  * Adds local configuration options to a GParserBuilder object
	  */
	 virtual void addConfigurationOptions(Gem::Common::GParserBuilder &gpb)
	 {
		 // Call our parent class'es function
		 GStdThreadConsumerT<processable_type>::GWorker::addConfigurationOptions(gpb);

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
	 double duration(const cl::Event &e) const
	 {
		 cl_ulong ev_start_time = (cl_ulong) 0;
		 cl_ulong ev_end_time = (cl_ulong) 0;
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
			 std::string openCLSource = Gem::Common::loadTextDataFromFile(m_codeFile);
			 m_source = cl::Program::Sources(
				 1
				 , std::make_pair(
					 openCLSource.c_str()
					 , openCLSource.length() + 1
				 ));
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

	 /***************************************************************************/

	 /** @brief The default constructor: Intentionally private and undefined */
	 GOpenCLWorkerT();

	 /** @brief The copy constructor: Intentionally private and undefined */
	 GOpenCLWorkerT(const GOpenCLWorkerT<processable_type> &);

	 /** @brief The assignment operator: Intentionally private and undefined */
	 const GOpenCLWorkerT<processable_type> &operator=(const GOpenCLWorkerT<processable_type> &);

	 /***************************************************************************/
	 // Some purely virtual functions that need to be specified in derived classes

public:
	 /** @brief Creates a deep clone of this object, camouflaged as a GWorker */
	 virtual std::shared_ptr<typename Gem::Courtier::GStdThreadConsumerT<processable_type>::GWorker> clone(
		 const std::size_t &
		 , const GStdThreadConsumerT <processable_type> *
	 ) const = 0;

	 /** @brief Actual per-item work is done here */
	 virtual void process(std::shared_ptr<processable_type> p) = 0;
};

/******************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

#endif /* GOPENCLWORKER_HPP_ */
