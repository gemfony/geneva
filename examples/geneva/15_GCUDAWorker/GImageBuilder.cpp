/**
 * @file GImageBuilder.cpp
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

// Standard header files go here
#include <iostream>
#include <memory>
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
#include "GImageCUDAWorker.hpp"

// Information retrieval and printing
#include "GImagePOM.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;

namespace po = boost::program_options;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A function that allows parsing of the command line
 */
void assembleCommandLineOptions(
    boost::program_options::options_description& user_options
    , bool& showDevices
    , std::string& logAll
    , std::string& logResults
    , std::string& monitorNAdaptions
    , std::string& logSigma
    , bool& logImages
    , bool& emitBestOnly
)
{
    user_options.add_options()(
        "showDevices"
        , po::value<bool>(&showDevices)->implicit_value(true)->default_value(false)
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
        , po::value<std::string>(&monitorNAdaptions)->implicit_value(std::string("./nAdaptions.C"))->
                                                      default_value("empty")
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
    const std::string& logAll,
    const std::string& logResults,
    const std::string& monitorNAdaptions,
    const std::string& logSigma,
    bool logImages,
    const std::string& resultDirectory,
    const std::string& targetFileName,
    bool emitBestOnly,
    bool useGPU,
    const std::tuple<int, int>& blockSize,
    const std::tuple<int, int>& gridSize
)
{
    std::shared_ptr<GCollectiveMonitor> collectiveMonitor_ptr(new GCollectiveMonitor());

    if (logAll != "empty")
    {
        std::shared_ptr<GAllSolutionFileLogger> allsolutionLogger_ptr(new GAllSolutionFileLogger(logAll));

        allsolutionLogger_ptr->setPrintWithNameAndType(true); // Output information about variable names and types
        allsolutionLogger_ptr->setPrintWithCommas(true); // Output commas between values
        allsolutionLogger_ptr->setUseTrueFitness(false); // Output "transformed" fitness, not the "true" value
        allsolutionLogger_ptr->setShowValidity(true); // Indicate, whether this is a valid solution

        collectiveMonitor_ptr->registerPluggableOM(allsolutionLogger_ptr);
    }

    if (logResults != "empty")
    {
        std::shared_ptr<GIterationResultsFileLogger> iterationResultLogger_ptr(
            new GIterationResultsFileLogger(logResults));

        iterationResultLogger_ptr->setPrintWithCommas(true); // Output commas between values
        iterationResultLogger_ptr->setUseTrueFitness(false); // Output "transformed" fitness, not the "true" value

        collectiveMonitor_ptr->registerPluggableOM(iterationResultLogger_ptr);
    }

    if (monitorNAdaptions != "empty")
    {
        std::shared_ptr<GNAdpationsLogger> nAdaptionsLogger_ptr(new GNAdpationsLogger(monitorNAdaptions));

        nAdaptionsLogger_ptr->setMonitorBestOnly(false); // Output information for all individuals
        nAdaptionsLogger_ptr->setAddPrintCommand(true); // Create a PNG file if Root-file is executed

        collectiveMonitor_ptr->registerPluggableOM(nAdaptionsLogger_ptr);
    }

    if (logSigma != "empty")
    {
        std::shared_ptr<GAdaptorPropertyLogger<double>>
            sigmaLogger_ptr(new GAdaptorPropertyLogger<double>(logSigma, "GDoubleGaussAdaptor", "sigma"));

        sigmaLogger_ptr->setMonitorBestOnly(false); // Output information for all individuals
        sigmaLogger_ptr->setAddPrintCommand(true); // Create a PNG file if Root-file is executed

        collectiveMonitor_ptr->registerPluggableOM(sigmaLogger_ptr);
    }

    // Create an additional POM for the image emission, if requested
    if (logImages)
    {
        std::shared_ptr<GImagePOM>
            imageLogger_ptr(new GImagePOM(
                    resultDirectory,
                    targetFileName,
                    emitBestOnly,
                    useGPU,
                    blockSize,
                    gridSize
                )
            );

        collectiveMonitor_ptr->registerPluggableOM(imageLogger_ptr);
    }

    if (collectiveMonitor_ptr->hasOptimizationMonitors())
    {
        return collectiveMonitor_ptr;
    }
    else
    {
        return std::shared_ptr<GCollectiveMonitor>(); // empty pointer indicates that no monitor was requested
    }
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
// Emits information on CUDA errors
void checkCuda(cudaError_t err, const char* msg)
{
    if (err != cudaSuccess)
    {
        fprintf(stderr, "CUDA Error! %s (%s)\n", msg,
                cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
/**
 * Prints out information about all devices
 */
void printDeviceInfo()
{
    int deviceCount = 0;

    // Anzahl der CUDA-fähigen Geräte abrufen
    checkCuda(cudaGetDeviceCount(&deviceCount), "deviceCount");

    if (deviceCount == 0)
    {
        std::cout << "No CUDA-capable devices found." << std::endl;
        return;
    }

    std::cout << "Number of CUDA-capable devices: " << deviceCount << "\n" << std::endl;

    // Informationen zu jedem Gerät abrufen und ausgeben
    for (int device = 0; device < deviceCount; ++device)
    {
        cudaDeviceProp deviceProp;
        checkCuda(cudaGetDeviceProperties(&deviceProp, device), "device properties");

        std::cout << "Device " << device << ": " << deviceProp.name << std::endl;
        std::cout << "  Compute Capability: " << deviceProp.major << "." << deviceProp.minor << std::endl;
        std::cout << "  Global Memory: " << static_cast<float>(deviceProp.totalGlobalMem) / (1 << 20) << " MB" <<
            std::endl;
        std::cout << "  Multiprocessors: " << deviceProp.multiProcessorCount << std::endl;

        // Number of CUDA-cores (this is an estimate)
        int cudaCores = 0;
        if (deviceProp.major == 2)
        {
            // Fermi
            cudaCores = deviceProp.multiProcessorCount * 32;
        }
        else if (deviceProp.major == 3)
        {
            // Kepler
            cudaCores = deviceProp.multiProcessorCount * 192;
        }
        else if (deviceProp.major == 5)
        {
            // Maxwell
            cudaCores = deviceProp.multiProcessorCount * 128;
        }
        else if (deviceProp.major == 6)
        {
            // Pascal
            cudaCores = deviceProp.multiProcessorCount * 64;
        }
        else if (deviceProp.major == 7)
        {
            // Volta, Turing
            cudaCores = deviceProp.multiProcessorCount * 64;
        }
        else if (deviceProp.major >= 8)
        {
            // Ampere und neuer
            cudaCores = deviceProp.multiProcessorCount * 64;
        }
        else
        {
            cudaCores = deviceProp.multiProcessorCount * 128; // Default estimate
        }

        std::cout << "  CUDA-Cores (estimate): " << cudaCores << std::endl;
        std::cout << "  Device Frequency: " << deviceProp.clockRate * 1e-3f << " MHz" << std::endl;
        std::cout << "  Memory Frequency: " << deviceProp.memoryClockRate * 1e-3f << " MHz" << std::endl;
        std::cout << "  Memory Bandwidth: " << deviceProp.memoryBusWidth << " Bit" << std::endl;
        std::cout << "  L2-Cache: " << deviceProp.l2CacheSize << " Bytes" << std::endl;
        std::cout << "  Maximum number of threads per block: " << deviceProp.maxThreadsPerBlock << std::endl;
        std::cout << "  Maximum Thread-Dimension: ("
            << deviceProp.maxThreadsDim[0] << ", "
            << deviceProp.maxThreadsDim[1] << ", "
            << deviceProp.maxThreadsDim[2] << ")" << std::endl;
        std::cout << "  Maximum Grid-Size: ("
            << deviceProp.maxGridSize[0] << ", "
            << deviceProp.maxGridSize[1] << ", "
            << deviceProp.maxGridSize[2] << ")" << "\n" << std::endl;
        if (deviceProp.concurrentKernels) {
            std::cout << "  The GPU supports concurrent Kernel-execution." << std::endl;
        } else {
            std::cout << "  The GPU does not support concurrent Kernel-execution." << std::endl;
        }
    }
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
/**
 * Retrieves a worker to be added to the GStdThreadConsumerT
 *
 * @return A CUDA worker template for the consumer
 */
std::shared_ptr<GImageCUDAWorker> getImageCUDAWorker()
{
    return std::make_shared<Gem::Courtier::GImageCUDAWorker>("./config/GImageCUDAWorker.json");
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
/**
 * The main function
 */
int main(int argc, char** argv)
{
    boost::program_options::options_description user_options;

    bool showDevices = false;
    std::string logAll = "empty";
    std::string logResults = "empty";
    std::string monitorNAdaptions = "empty";
    std::string logSigma = "empty";
    bool logImages;
    bool emitBestOnly;

    assembleCommandLineOptions(
        user_options
        , showDevices
        , logAll
        , logResults
        , monitorNAdaptions
        , logSigma
        , logImages
        , emitBestOnly
    );

    // Retrieve workers
    std::vector<std::shared_ptr<Gem::Courtier::GImageCUDAWorker>> workers;
    std::tuple<std::size_t, std::size_t> imageDimensions;

    // Retrieve a CUDA worker
    auto cudaWorker_ptr = getImageCUDAWorker();

    // Set up the consumer -- this call will register it with the broker
    GStdThreadConsumerT<GParameterSet>::setup("./config/GStdThreadConsumerT.json", cudaWorker_ptr);

    // Create the optimizer
    Go2 go(argc, argv, "./config/Go2.json", user_options);

    //---------------------------------------------------------------------------
    // As we are dealing with a server, register a signal handler that allows us
    // to interrupt execution "on the run"
    signal(G_SIGHUP, GObject::sigHupHandler);

    //---------------------------------------------------------------------------
    // If we have only been asked to print device info, do so and exit
    if (showDevices)
    {
        printDeviceInfo();
        exit(0);
    }

    // Register pluggable optimization monitors, if requested by the user
    std::shared_ptr<GCollectiveMonitor> collectiveMonitor_ptr = getPOM(
        logAll,
        logResults,
        monitorNAdaptions,
        logSigma,
        logImages,
        "./results/",
        cudaWorker_ptr->getTargetImageFileName(),
        emitBestOnly,
        cudaWorker_ptr->useGPU(),
        cudaWorker_ptr->getBlockSize(),
        cudaWorker_ptr->getGridSize()
    );

    if (collectiveMonitor_ptr)
    {
        go.registerPluggableOM(collectiveMonitor_ptr);
    }

    // Create an image individual factory and create the first individual
    GImageIndividualFactory f("config/GImageIndividual.json");
    std::shared_ptr<GParameterSet> imageIndividual_ptr = f();

    // Attach the individual to the collection
    go.push_back(imageIndividual_ptr);

    // Create an evolutionary algorithm in broker mode
    GEvolutionaryAlgorithmFactory ea("./config/GEvolutionaryAlgorithm.json");
    std::shared_ptr<GEvolutionaryAlgorithm> ea_ptr = ea.get<GEvolutionaryAlgorithm>();

    // Add the algorithm
    go & ea_ptr;

    // Perform the actual optimization and extract the best individual
    std::shared_ptr<GImageIndividual> p = go.optimize()->getBestGlobalIndividual<GImageIndividual>();

    // Note that the useful work of this program is done at the end of each
    // iteration when it writes out the current picture. So we do nothing
    // with the best individual here.
}

/********************************************************************************/
//////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
