/**
 * @file GImageBuilder.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva/P library collection. No part of
 * this code may be distributed without prior, written consent by
 * the management of Gemfony scientific.
 */

// Standard header files go here
#include <iostream>
#include <vector>
#include <tuple>
#include <memory>

// Boost headers
#include <boost/program_options.hpp>

// Geneva header files go here
#include "common/GCommonHelperFunctions.hpp"
#include "courtier/GStdThreadConsumerT.hpp"
#include "geneva/Go2.hpp"
#include "geneva/GPluggableOptimizationMonitors.hpp"

// The individual that should be optimized
#include "GImageIndividual.hpp"

// The consumer used for OpenCL targets
#include "GImageOpenCLWorker.hpp"

// Information retrieval and printing
#include "GImagePOM.hpp"

// OpenCL includes
#define __CL_ENABLE_EXCEPTIONS // This will force OpenCL C++ classes to raise exceptions rather than to use an error code
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp" // Use the file in our local directory -- cl.hpp is not delivered by default on MacOS X
#else
#include <CL/cl.hpp>
#endif

using namespace Gem::Geneva;
using namespace Gem::Courtier;

namespace po = boost::program_options;

const cl_device_type DEFAULTDEVICETYPE = CL_DEVICE_TYPE_ALL; // GPU+CPU


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A function that allows parsing of the command line
 */
void assembleCommandLineOptions(
	boost::program_options::options_description& user_options
   , bool& showDevices
   , std::string& deviceDescription
   , std::string& logAll
   , std::string& logResults
   , std::string& monitorNAdaptions
   , std::string& logSigma
   , bool& logImages
   , bool& emitBestOnly
) {
	user_options.add_options()(
		"showDevices"
		, po::value<bool>(&showDevices)->implicit_value(true)->default_value(false) // This allows you say both --showDevices and --showDevices=true
		, "Shows all devices and then exits"
	)(
		"devices"
		, po::value<std::string>(&deviceDescription)->default_value("(0,0)") // The first device of the first platform
		, "Allows to specify the devices one wishes to use in the format \"(p1,d1), (p2,d2)\", where the p represent platforms and the d represent devices inside of the platforms."
	)(
		"logAll"
		, po::value<std::string>(&logAll)->default_value("empty")
		, "Logs all solutions to the file name provided as argument to this switch"
	)(
		"logResults"
		, po::value<std::string>(&logResults)->default_value("empty")
		, "Logs the results of all candidate solutions in an iteration"
	)(
		"monitorAdaptions"
		, po::value<std::string>(&monitorNAdaptions)->implicit_value(std::string("./nAdaptions.C"))->default_value("empty")
		, "Logs the number of adaptions for all individuals over the course of the optimization. Useful for evolutionary algorithms only."
	)(
		"logSigma"
		, po::value<std::string>(&logSigma)->implicit_value(std::string("./sigmaLog.C"))->default_value("empty")
		, "Logs the value of sigma for all or the best adaptors, if GDoubleGaussAdaptors are being used"
	)(
		"logImages"
		, po::value<bool>(&logImages)->implicit_value(true)->default_value(true)
		, "Logs the images in each iteration"
	)(
		"emitBestOnly"
		, po::value<bool>(&emitBestOnly)->implicit_value(true)->default_value(true)
		, "Determines whether only the best results should be emitted. Will only have an effect if \"logImages\" is set to \"true\""
	);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Set up a number of optimization monitors, mostly for debugging and
 * profiling purposes
 */
std::shared_ptr<GCollectiveMonitor> getPOM(
   const std::string& logAll
   , const std::string& logResults
   , const std::string& monitorNAdaptions
   , const std::string& logSigma
   , const bool& logImages
   , const bool& emitBestOnly
   , const std::tuple<std::size_t, std::size_t>& imageDimensions
) {
   std::shared_ptr<GCollectiveMonitor> collectiveMonitor_ptr(new GCollectiveMonitor());

   if(logAll != "empty") {
      std::shared_ptr<GAllSolutionFileLogger> allsolutionLogger_ptr(new GAllSolutionFileLogger(logAll));

      allsolutionLogger_ptr->setPrintWithNameAndType(true); // Output information about variable names and types
      allsolutionLogger_ptr->setPrintWithCommas(true); // Output commas between values
      allsolutionLogger_ptr->setUseTrueFitness(false); // Output "transformed" fitness, not the "true" value
      allsolutionLogger_ptr->setShowValidity(true); // Indicate, whether this is a valid solution

      collectiveMonitor_ptr->registerPluggableOM(allsolutionLogger_ptr);
   }

   if(logResults != "empty") {
      std::shared_ptr<GIterationResultsFileLogger> iterationResultLogger_ptr(new GIterationResultsFileLogger(logResults));

      iterationResultLogger_ptr->setPrintWithCommas(true); // Output commas between values
      iterationResultLogger_ptr->setUseTrueFitness(false); // Output "transformed" fitness, not the "true" value

      collectiveMonitor_ptr->registerPluggableOM(iterationResultLogger_ptr);
   }

   if(monitorNAdaptions != "empty") {
      std::shared_ptr<GNAdpationsLogger> nAdaptionsLogger_ptr(new GNAdpationsLogger(monitorNAdaptions));

      nAdaptionsLogger_ptr->setMonitorBestOnly(false); // Output information for all individuals
      nAdaptionsLogger_ptr->setAddPrintCommand(true); // Create a PNG file if Root-file is executed

      collectiveMonitor_ptr->registerPluggableOM(nAdaptionsLogger_ptr);
   }

   if(logSigma != "empty") {
      std::shared_ptr<GAdaptorPropertyLogger<double> >
         sigmaLogger_ptr(new GAdaptorPropertyLogger<double>(logSigma, "GDoubleGaussAdaptor", "sigma"));

      sigmaLogger_ptr->setMonitorBestOnly(false); // Output information for all individuals
      sigmaLogger_ptr->setAddPrintCommand(true); // Create a PNG file if Root-file is executed

      collectiveMonitor_ptr->registerPluggableOM(sigmaLogger_ptr);
   }

   // Create an additional POM for the image emission, of requested
   if(logImages) {
      std::shared_ptr<GImagePOM >
         imageLogger_ptr(new GImagePOM("./results/", emitBestOnly));

      imageLogger_ptr->setImageDimensions(imageDimensions, 1);

      collectiveMonitor_ptr->registerPluggableOM(imageLogger_ptr);
   }

   if(collectiveMonitor_ptr->hasOptimizationMonitors()) {
      return collectiveMonitor_ptr;
   } else {
      return std::shared_ptr<GCollectiveMonitor>(); // empty pointer indicates that no monitor was requested
   }
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
/**
 * Returns the platforms available on this computer
 */
std::vector<cl::Platform> getPlatforms() {
   std::vector<cl::Platform> platforms;

   try{
      // Extract the platforms
      cl::Platform::get(&platforms);

      if(platforms.empty()) {
         glogger
         << "In getPlatforms(): Error!" << std::endl
         << "No platforms found ..." << std::endl
         << GEXCEPTION;
      }
   } catch(cl::Error& err) {
      std::cout << err.what() << std::endl
            << err.err()  << std::endl;

      std::terminate();
   } catch(Gem::Common::gemfony_error_condition& err) {
      std::cout << err.what();
      std::terminate();
   }

   // Let the audience know
   return platforms;
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
/**
 * Emits the devices of a specified device type for a given platform
 */
std::vector<cl::Device> getDevices(const cl::Platform& platform, const cl_device_type& t) {
   std::vector<cl::Device> devices;

   try{
      // Extract a vector of devices
      platform.getDevices(t, &devices);

      if(devices.empty()) {
         glogger
         << "In getDevices(): Error!" << std::endl
         << "No devices found ..." << std::endl
         << GEXCEPTION;
      }
   } catch(cl::Error& err) {
      std::cout << err.what();
      std::terminate();
   } catch(Gem::Common::gemfony_error_condition& err) {
      std::cout << err.what();
      std::terminate();
   }

   // Let the audience know
   return devices;
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
/**
 * Prints out information about all devices
 */
void printDeviceInfo() {
   std::vector<cl::Platform> platforms = getPlatforms();
   for(std::size_t p=0; p<platforms.size(); p++) {
      // Identify the platform
      std::cout << "Platform " << p << ": " << platforms[p].getInfo<CL_PLATFORM_NAME>() << std::endl << std::flush;

      std::vector<cl::Device> devices = getDevices(platforms[p], DEFAULTDEVICETYPE);
      for(std::size_t d=0; d<devices.size(); d++) {
         std::cout << "Device " << d << ": " << devices[d].getInfo<CL_DEVICE_NAME>() << std::endl << std::flush;
      }
   }
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
/**
 * Retrieves workers to be added to the GBoostThreadConsumerT
 */
std::vector<std::shared_ptr<GStdThreadConsumerT<GParameterSet>::GWorker> > getWorkers(
   std::string deviceDescription
   , std::tuple<std::size_t, std::size_t>& imageDimensions
) {
   bool first = true;

   // Dissect the deviceDescription
   std::vector<std::tuple<unsigned int, unsigned int> > deviceIds = Gem::Common::stringToUIntTupleVec(deviceDescription);
   std::vector<std::tuple<unsigned int, unsigned int> >::iterator it;

   // Will hold the workers
   std::vector<std::shared_ptr<GStdThreadConsumerT<GParameterSet>::GWorker>> workers;

   // Retrieve the workers
   std::vector<cl::Platform> platforms = getPlatforms();
   for(std::size_t p=0; p<platforms.size(); p++) {
      std::vector<cl::Device> devices = getDevices(platforms[p], DEFAULTDEVICETYPE);
      for(std::size_t d=0; d<devices.size(); d++) {
         for(it=deviceIds.begin(); it!=deviceIds.end(); ++it) {
            if(p == std::get<0>(*it) && d==std::get<1>(*it)) {
               std::shared_ptr<GImageOpenCLWorker> worker_ptr = std::shared_ptr<GImageOpenCLWorker>(
                     new GImageOpenCLWorker(
                           devices[d]
                          , "./config/GImageOpenCLWorker.json"
                     )
               );

               if(first) {
                  imageDimensions = worker_ptr->getImageDimensions();
                  first=false;
               }

               workers.push_back(worker_ptr);

               std::cout
               << "Added device " << devices[d].getInfo<CL_DEVICE_NAME>() << " of platform " << platforms[p].getInfo<CL_PLATFORM_NAME>() << std::endl;
            }
         }
      }
   }

   if(workers.empty()) {
      glogger
      << "In getWorkers(): Error!" << std::endl
      << "No workers could be retrieved" << std::endl
      << GEXCEPTION;
   }

   return workers;
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
/**
 * The main function
 */
int main(int argc, char **argv) {
	boost::program_options::options_description user_options;

   bool showDevices = false;
   std::string deviceDescription;
   std::string logAll = "empty";
   std::string logResults = "empty";
   std::string monitorNAdaptions = "empty";
   std::string logSigma = "empty";
   bool logImages;
   bool emitBestOnly;

   assembleCommandLineOptions(
		user_options
    , showDevices
    , deviceDescription
    , logAll
    , logResults
    , monitorNAdaptions
    , logSigma
    , logImages
    , emitBestOnly
   );

   // Create the optimizer
   Go2 go(argc, argv, "./config/Go2.json", user_options);

	//---------------------------------------------------------------------------
	// As we are dealing with a server, register a signal handler that allows us
	// to interrupt execution "on the run"
	signal(G_SIGHUP, GObject::sigHupHandler);

	//---------------------------------------------------------------------------
   // If we have only been asked to print device info, do so and exit
   if(showDevices) {
      printDeviceInfo();
      exit(0);
   }

   // Retrieve workers
   std::vector<std::shared_ptr<GStdThreadConsumerT<GParameterSet>::GWorker>> workers;
   std::tuple<std::size_t, std::size_t> imageDimensions;
   workers = getWorkers(deviceDescription, imageDimensions);

   // Set up the consumer
   GStdThreadConsumerT<GParameterSet>::setup("./config/GStdThreadConsumerT.json", workers);

   // Register pluggable optimization monitors, if requested by the user
   std::shared_ptr<GCollectiveMonitor> collectiveMonitor_ptr = getPOM(
      logAll
      , logResults
      , monitorNAdaptions
      , logSigma
      , logImages, emitBestOnly, imageDimensions
   );

   if(collectiveMonitor_ptr) {
      go.registerPluggableOM(collectiveMonitor_ptr);
   }

   // Create an image individual factory and create the first individual
   GImageIndividualFactory f("config/GImageIndividual.json");
   std::shared_ptr<GParameterSet> imageIndividual_ptr  = f();

   // Attach the individual to the collection
   go.push_back(imageIndividual_ptr);

   // Create an evolutionary algorithm in broker mode
   GEvolutionaryAlgorithmFactory ea("./config/GEvolutionaryAlgorithm.json");
   std::shared_ptr<GEvolutionaryAlgorithm> ea_ptr = ea.get<GEvolutionaryAlgorithm>();

   // Add the algorithm
   go & ea_ptr;

   // Perform the actual optimization and extract the best individual
   std::shared_ptr<GImageIndividual> p = go.optimize<GImageIndividual>();
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/

