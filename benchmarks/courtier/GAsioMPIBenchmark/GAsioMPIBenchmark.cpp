/**
 * @file GAsioMPIBenchmark.cpp
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

// TODO: improve code quality

// Standard header files go here
#include <iostream>
#include <filesystem>

// Boost header files go here
#include <boost/process.hpp>

// Geneva header files go here

#include "GAsioMPIBenchmarkConfig.hpp"
#include "common/GPlotDesigner.hpp"

using namespace Gem::Tests;

// name of the directory for ROOT files
const std::string resultDirName{"results"};
const std::string resultPrefix{"result"};

// name of the file produces by the subprogram
const std::string executionTimesFileNameBeforeRename{"executionTimesVector.ser"};
// name of the directory to move executionTimes of individual runs of the subprogram
const std::string executionTimesDirName{"executionTimes"};
const std::string executionTimesFilePrefix{"executionTimes"};

// line color so be used when drawing multiple curves in the same graph
// These are ROOT constants
const std::vector<std::string> lineColors{
        "kBlack",
        "kGray",
        "kRed",
        "kGreen",
        "kBlue",
        "kYellow",
        "kMagenta",
        "kCyan",
        "kOrange",
        "kSpring",
        "kTeal",
        "kAzure",
        "kViolet",
        "kPink"
};

Gem::Common::serializationMode serMode = Gem::Common::serializationMode::TEXT;

/**
 * stores the execution times for asio and MPI for one specific number of clients
 */
struct ExecutionTimes {
    std::uint32_t nClients;
    std::vector<std::tuple<double, double, double, double>> executionTimesAsio;
    std::vector<std::tuple<double, double, double, double>> executionTimesMPI;
};

/**
 * loads a vector of exectuion times from a specified file
 */
std::vector<std::tuple<double, double, double, double>> loadExTimesFromFile(const std::string &path) {
    std::vector<std::tuple<double, double, double, double>> exTimes{};
    std::ifstream ifs(path);
    boost::archive::text_iarchive ia(ifs);
    ia & exTimes;

    return exTimes;
}

/**
 * takes a vector with error values and returns a vector with only x-values and the mean as y-values
 */
std::vector<std::tuple<double, double>> extractMean(
        const std::vector<std::tuple<double, double, double, double>> &loadExTimesFromFile) {
    std::vector<std::tuple<double, double>> means{};

    std::for_each(loadExTimesFromFile.begin(), loadExTimesFromFile.end(), [&means](const auto &item) {
        means.push_back(
                std::tuple<double, double>(
                        std::get<0>(item),
                        std::get<2>(item))
        );
    });

    return means;
}

std::string getNumberOfClientsPrefix(const std::uint32_t &nClients) {
    std::stringstream sout{};
    sout << std::setfill('0') << std::setw(4) << nClients;

    return sout.str();
}

std::string getCommandBanner(const std::string &command) {
    std::stringstream sout{};
    sout << "-----------------------------------------" << std::endl
         << "running command: `" << command << "` as a new process" << std::endl
         << "-----------------------------------------" << std::endl;

    return sout.str();
}

void measureExecutionTimesMPI(const GAsioMPIBenchmarkConfig &config, std::uint32_t nClients) {
    boost::process::ipstream pipeStream{};

    std::string command = "mpirun --oversubscribe -np "
                          + std::to_string(nClients + 1) // one server + nClients
                          + " "
                          + config.getMBenchmarkExecutableName()
                          + " --consumer mpi";

    std::cout << getCommandBanner(command) << std::endl;

    boost::process::child c(command, boost::process::std_out > pipeStream);

    // pipe std out of mpirun to this process
    std::string line{};
    while (pipeStream && std::getline(pipeStream, line) && !line.empty())
        std::cerr << line << std::endl;

    c.wait();
}

void measureExecutionTimesAsio(const GAsioMPIBenchmarkConfig &config, std::uint32_t nClients) {
    boost::process::ipstream pipeStream{};

    std::string command = config.getMBenchmarkExecutableName() + " --consumer asio";

    std::cout << getCommandBanner(command) << std::endl;

    // run once without the --client attribute to start a server
    boost::process::child server(command, boost::process::std_out > pipeStream);

    // start nClients clients and store handles in a vector
    std::vector<boost::process::child> clients{};
    for (int i{0}; i < nClients; ++i) {
        // this will call the constructor of the boost::process::child class and start the process
        clients.emplace_back(command + " --client");
    }

    // pipe std out the server to this process
    std::string line{};
    while (pipeStream && std::getline(pipeStream, line) && !line.empty())
        std::cerr << line << std::endl;


    // wait for the completion of all processes
    server.wait();
    std::for_each(clients.begin(), clients.end(), [](boost::process::child &client) { client.wait(); });
}

void renameIntermediateFiles(const GAsioMPIBenchmarkConfig &config, const std::string &suffix, std::uint32_t nClients) {
    namespace fs = std::filesystem;

    fs::path workDir = fs::current_path();

    // copy the root file to the results directory
    fs::path resultDir = workDir / resultDirName;
    std::string resultFileName{getNumberOfClientsPrefix(nClients) + "_" + resultPrefix + "_" + suffix};
    fs::rename(workDir / config.getMIntermediateResultFileName(), resultDir / resultFileName);

    // copy the serialized graph object to graph objects directory
    fs::path executionTimesDir = workDir / executionTimesDirName;
    std::string graphObjectFileName{getNumberOfClientsPrefix(nClients) + "_" + executionTimesFilePrefix + "_" + suffix};
    fs::rename(workDir / executionTimesFileNameBeforeRename, executionTimesDir / graphObjectFileName);
}

void resetOutputDirs() {
    namespace fs = std::filesystem;

    fs::path workDir = fs::current_path();
    fs::path graphsDir = workDir / executionTimesDirName;
    fs::path resultDir = workDir / resultDirName;

    fs::remove_all(graphsDir);
    fs::remove_all(resultDir);

    fs::create_directory(graphsDir);
    fs::create_directory(resultDir);
}

Gem::Common::GPlotDesigner configurePlotter(
        const std::vector<ExecutionTimes> &exTimesVec,
        const std::string &title,
        const std::string &xLabel,
        const std::string &yLabel,
        const bool &singlePlot) {

    Gem::Common::GPlotDesigner gpd(title,
                                   2,
                                   (singlePlot ? 1 : exTimesVec.size()));

    if (singlePlot) {
        // NOTE: multiple graphs in a single plot can only be done with GGraph2D not with GGraph2ED
        // create one first main graph for asio and mpi
        auto asioMainGraph = std::make_shared<Gem::Common::GGraph2D>();
        auto mpiMainGraph = std::make_shared<Gem::Common::GGraph2D>();

        // set labels for main graph
        asioMainGraph->setPlotLabel("Asio");
        mpiMainGraph->setPlotLabel("MPI");

        asioMainGraph->setXAxisLabel(xLabel);
        asioMainGraph->setYAxisLabel(yLabel);
        mpiMainGraph->setXAxisLabel(xLabel);
        mpiMainGraph->setYAxisLabel(yLabel);

        // set drawing arguments
        asioMainGraph->setDrawingArguments("ALP*");
        mpiMainGraph->setDrawingArguments("ALP*");

        // set the line colors for the fist iteration
        asioMainGraph->setLineColor(lineColors[0]);
        mpiMainGraph->setLineColor(lineColors[0]);

        // add the data to the graphs
        (*asioMainGraph) & extractMean(exTimesVec[0].executionTimesAsio);
        (*mpiMainGraph) & extractMean(exTimesVec[0].executionTimesMPI);

        // add all following graphs as subplots
        for (int i{1}; i < exTimesVec.size(); ++i) { // start from second element
            auto asioSubGraph = std::make_shared<Gem::Common::GGraph2D>();
            auto mpiSubGraph = std::make_shared<Gem::Common::GGraph2D>();

            // add the data to the sub-graphs
            (*asioSubGraph) & extractMean(exTimesVec[i].executionTimesAsio);
            (*mpiSubGraph) & extractMean(exTimesVec[i].executionTimesMPI);

            // set drawing options
            asioSubGraph->setDrawingArguments("L*");
            mpiSubGraph->setDrawingArguments("L*");

            // set line colors
            asioSubGraph->setLineColor(lineColors[i % lineColors.size()]); // modulo to prevent out of bounds error
            mpiSubGraph->setLineColor(lineColors[i % lineColors.size()]);

            // add the sub-graphs to the main-graph
            asioMainGraph->registerSecondaryPlotter(asioSubGraph);
            mpiMainGraph->registerSecondaryPlotter(mpiSubGraph);
        }

        // TODO: print legend

        gpd.registerPlotter(asioMainGraph);
        gpd.registerPlotter(mpiMainGraph);

    } else {
        for (const auto &et: exTimesVec) {
            // create two graphs
            auto asioGraph = std::make_shared<Gem::Common::GGraph2ED>();
            auto mpiGraph = std::make_shared<Gem::Common::GGraph2ED>();

            // set labels
            asioGraph->setPlotLabel("Asio clients = " + std::to_string(et.nClients));
            mpiGraph->setPlotLabel("MPI clients = " + std::to_string(et.nClients));

            asioGraph->setXAxisLabel(xLabel);
            asioGraph->setYAxisLabel(yLabel);
            mpiGraph->setXAxisLabel(xLabel);
            mpiGraph->setYAxisLabel(yLabel);

            // add the data to the graphs
            (*asioGraph) & et.executionTimesAsio;
            (*mpiGraph) & et.executionTimesMPI;

            // register graphs with the plotter
            gpd.registerPlotter(asioGraph);
            gpd.registerPlotter(mpiGraph);
        }


    }

    gpd.setCanvasDimensions(800, 1200);

    return gpd;
}

void plotAbsoluteTimes(const std::vector<ExecutionTimes> &exTimesVec, const GAsioMPIBenchmarkConfig &config) {
    // plot directly with no modification, because values are already absolute
    configurePlotter(exTimesVec,
                     "Absolute time for optimizations for different numbers of consumers and evaluation of the fitness.",
                     "time to calculate fitness [s]",
                     "time needed for one optimization [s]",
                     true)
            .writeToFile(std::filesystem::path("abs_onePlot_" + config.getResultFileName()));

    configurePlotter(exTimesVec,
                     "Absolute time for optimizations for different numbers of consumers and evaluation of the fitness.",
                     "time to calculate fitness [s]",
                     "time needed for one optimization [s]",
                     false)
            .writeToFile(std::filesystem::path("abs_multiplePlots_" + config.getResultFileName()));
}

void combineGraphsToPlot(const GAsioMPIBenchmarkConfig &config) {
    namespace fs = std::filesystem;

    fs::path executionTimesDir = fs::current_path() / executionTimesDirName;


    Gem::Common::GPlotDesigner gpd("Processing times for different evaluation times of individuals ", 2,
                                   config.getNClients().size());


    // insert all file entries into a vector
    std::vector<fs::directory_entry> exTimesFiles{};
    for (auto const &dir_entry: std::filesystem::directory_iterator{executionTimesDir}) {
        exTimesFiles.push_back(dir_entry);
    }

    // sort by names, name prefix indicates number of clients -> sort by clients
    std::sort(exTimesFiles.begin(), exTimesFiles.end());

    std::vector<ExecutionTimes> exTimesVec;
    for (int i{0}; i < exTimesFiles.size(); i += 2) { // iterate in steps of two (get one asio and one mpi file)
        // add execution times for one number of clients to the vector
        exTimesVec.push_back(
                ExecutionTimes{
                        config.getNClients()[i / 2], // the amount of clients used for this run of the subprogram
                        loadExTimesFromFile(
                                exTimesFiles[i].path().string()), // asio ex-times (asio is alphabetically before mpi
                        loadExTimesFromFile(exTimesFiles[i + 1].path().string())
                });
    }

    plotAbsoluteTimes(exTimesVec, config);
}

std::string getHeader(const GAsioMPIBenchmarkConfig &config) {
    std::stringstream sout{};
    sout << "-----------------------------------------" << std::endl
         << "starting " << config.getNClients().size() << " benchmark(s) for asio and mpi" << std::endl
         << "consumer numbers to benchmark: [ " << Gem::Common::vecToString(config.getNClients()) << "]" << std::endl
         << "-----------------------------------------" << std::endl;

    return sout.str();
}

int main(int argc, char **argv) {
    GAsioMPIBenchmarkConfig config{argc, argv};
    std::cout << getHeader(config) << std::endl;

    resetOutputDirs();
    for (const std::uint32_t &nClients: config.getNClients()) {
        measureExecutionTimesAsio(config, nClients);
        renameIntermediateFiles(config, "asio", nClients);

        measureExecutionTimesMPI(config, nClients);
        renameIntermediateFiles(config, "mpi", nClients);
    }

    combineGraphsToPlot(config);

    return 0;
}
