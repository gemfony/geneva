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

// Standard header files go here
#include <iostream>
#include <filesystem>

// Boost header files go here
#include <boost/process.hpp>

// Geneva header files go here

#include "GAsioMPIBenchmarkConfig.hpp"
#include "common/GPlotDesigner.hpp"

using namespace Gem::Tests;

const std::string graphObjectsDirectoryName{"graph_objects"};
const std::string intermediateGraphObjectFileName{"graph_object.ser"};
const std::string resultDirName{"results"};
const std::string resultPrefix{"result"};
const std::string graphObjectPrefix{"graph_object"};

Gem::Common::serializationMode serMode = Gem::Common::serializationMode::TEXT;

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
    std::string resultFileName{std::to_string(nClients) + "_" + resultPrefix + "_" + suffix};
    fs::rename(workDir / config.getMIntermediateResultFileName(), resultDir / resultFileName);

    // copy the serialized graph object to graph objects directory
    fs::path graphsDir = workDir / graphObjectsDirectoryName;
    std::string graphObjectFileName{std::to_string(nClients) + "_" + graphObjectPrefix + "_" + suffix};
    fs::rename(workDir / intermediateGraphObjectFileName, graphsDir / graphObjectFileName);
}

void resetOutputDirs() {
    namespace fs = std::filesystem;

    fs::path workDir = fs::current_path();
    fs::path graphsDir = workDir / graphObjectsDirectoryName;
    fs::path resultDir = workDir / resultDirName;

    fs::remove_all(graphsDir);
    fs::remove_all(resultDir);

    fs::create_directory(graphsDir);
    fs::create_directory(resultDir);
}

void combineGraphsToPlot(const GAsioMPIBenchmarkConfig &config) {
    namespace fs = std::filesystem;

    fs::path graphsDir = fs::current_path() / graphObjectsDirectoryName;


    Gem::Common::GPlotDesigner gpd("Processing times for different evaluation times of individuals ", 2,
                                   config.getNClients().size());


    // insert all file entries into a vector
    std::vector<fs::directory_entry> graphFiles{};
    for (auto const &dir_entry: std::filesystem::directory_iterator{graphsDir}) {
        graphFiles.push_back(dir_entry);
    }

    // sort by names, names indicate number of clients
    std::sort(graphFiles.begin(), graphFiles.end());

    // load the graphs from the sorted files and add to the GPlotterDesigner
    for (int i{0}; i < graphFiles.size(); ++i) {
        auto graph = std::make_shared<Gem::Common::GGraph2ED>();
        graph->fromFile(graphFiles[i], serMode);

        // generate an appropriate label for this graph
        // we have 2 files for each number of clients - one for asio one for mpi
        // TODO: make sorting also work for multiple digits
        auto nClients = config.getNClients()[i / 2];
        std::string label{((i % 2 == 0) ? "Asio" : "MPI") + (" clients=" + std::to_string(nClients))};

        // set the label
        graph->setPlotLabel(label);

        // add the graph to the plotter
        gpd.registerPlotter(graph);
    }

    gpd.setCanvasDimensions(800, 1200);
    gpd.writeToFile(config.getResultFileName());
}

std::string getHeader(const GAsioMPIBenchmarkConfig &config) {
    std::stringstream sout{};
    sout << "-----------------------------------------" << std::endl
         << "starting " << config.getNClients().size() << " benchmark(s) for asio and mpi" << std::endl
         << "consumer numbers to benchmark: [" << Gem::Common::vecToString(config.getNClients()) << std::endl
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
