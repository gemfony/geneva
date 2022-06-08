/**
 * @file GNetworkedConsumerBenchmark.cpp
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

#include "GNetworkedConsumerBenchmarkConfig.hpp"
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
        "kMagenta",
        "kCyan",
        "kOrange",
        "kSpring",
        "kTeal",
        "kAzure",
        "kViolet",
        "kPink",
        "kYellow"
};

Gem::Common::serializationMode serMode = Gem::Common::serializationMode::TEXT;

/**
 * stores the execution times for asio and MPI for one specific number of clients
 */
struct ExTimesSleepAtX {
    std::uint32_t nClients;
    /*
     * <sleep_time, error, mean, stddev>
     */
    std::vector<std::tuple<double, double, double, double>> executionTimesAsio;
    std::vector<std::tuple<double, double, double, double>> executionTimesMPI;
};

/**
 * stores the execution times for asio and MPI for a specific execution time of the fitness function
 */
struct ExTimesClientsAtX {
    double sleepTime;
    /**
     * <clients, error, mean, stdDev>
     */
    std::vector<std::tuple<double, double, double, double>> executionTimesAsio;
    std::vector<std::tuple<double, double, double, double>> executionTimesMPI;
};

std::vector<ExTimesClientsAtX> sleepAtXToClientsAtX(const std::vector<ExTimesSleepAtX> &sleepAtXVec) {
    // output vector
    std::vector<ExTimesClientsAtX> clientsAtXVec{};

    // number of different values for sleepTime in the input vector
    // sleep times are equal for all input elements, therefore just choose first element with asio
    std::vector<double> sleepTimes{};
    std::for_each(sleepAtXVec[0].executionTimesAsio.begin(), sleepAtXVec[0].executionTimesAsio.end(),
                  [&sleepTimes](const auto &tup) { sleepTimes.push_back(std::get<0>(tup)); });

    // iterate over all sleep times, for each add a new entry to the output vector
    for (std::size_t i{0}; i < sleepTimes.size(); ++i) {
        ExTimesClientsAtX toAdd{
                sleepTimes[i],
                std::vector<std::tuple<double, double, double, double>>{},
                std::vector<std::tuple<double, double, double, double>>{}
        };

        // extract clients, error, mean, stdDev for each nClients for a fixed sleep time
        // and add it to the vector

        std::for_each(sleepAtXVec.begin(), sleepAtXVec.end(), [&toAdd, &i](const ExTimesSleepAtX &sax) {
            toAdd.executionTimesAsio.emplace_back(
                    sax.nClients,
                    std::get<1>(sax.executionTimesAsio[i]),
                    std::get<2>(sax.executionTimesAsio[i]),
                    std::get<3>(sax.executionTimesAsio[i])
            );
        });

        std::for_each(sleepAtXVec.begin(), sleepAtXVec.end(), [&toAdd, &i](const ExTimesSleepAtX &sax) {
            toAdd.executionTimesMPI.emplace_back(
                    sax.nClients,
                    std::get<1>(sax.executionTimesMPI[i]),
                    std::get<2>(sax.executionTimesMPI[i]),
                    std::get<3>(sax.executionTimesMPI[i])
            );
        });

        clientsAtXVec.push_back(toAdd);
    }

    return clientsAtXVec;
}

/**
 * returns the maximum y-value for a vector of execution times.
 */
double getYMax(const std::vector<ExTimesSleepAtX> &exTimesVec) {
    double maxY{0};

    // iterate over all execution times for both asio and mpi to extract maximum y-value
    for (const auto &exTimes: exTimesVec) {
        for (const auto &tup: exTimes.executionTimesAsio) {
            if (std::get<2>(tup) > maxY) {
                maxY = std::get<2>(tup);
            }
        }
        for (const auto &tup: exTimes.executionTimesMPI) {
            if (std::get<2>(tup) > maxY) {
                maxY = std::get<2>(tup);
            }
        }
    }

    return maxY;
}

/**
 * loads a vector of execution times from a specified file
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
template<typename x_type, typename y_type>
std::vector<std::tuple<x_type, y_type>> extractMean(
        const std::vector<std::tuple<x_type, double, y_type, double>> &exTimes) {
    std::vector<std::tuple<x_type, y_type>> means{};

    std::for_each(exTimes.begin(), exTimes.end(), [&means](const auto &item) {
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

void measureExecutionTimes(const GNetworkedConsumerBenchmarkConfig &config,
                           std::uint32_t nClients,
                           const Competitor &competitor) {
    std::cout << nClients << " " << competitor.name << std::endl;
}

void measureExecutionTimesMPI(const GNetworkedConsumerBenchmarkConfig &config, std::uint32_t nClients) {
    boost::process::ipstream pipeStream{};

    std::string command = config.getMpirunLocation()
                          + " --oversubscribe -np "
                          + std::to_string(nClients + 1) // one server + nClients
                          + " "
                          + config.getMBenchmarkExecutableName()
                          + " --consumer mpi"
                          // use as many io-threads as clients to be able to process all in parallel
                          + " --mpi_master_nIOThreads " + std::to_string(nClients);

    std::cout << getCommandBanner(command) << std::endl;

    boost::process::child c(command, boost::process::std_out > pipeStream);

    // pipe std out of mpirun to this process
    std::string line{};
    while (pipeStream && std::getline(pipeStream, line) && !line.empty())
        std::cerr << line << std::endl;

    c.wait();
}

void measureExecutionTimesAsio(const GNetworkedConsumerBenchmarkConfig &config, std::uint32_t nClients) {
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

void renameIntermediateFiles(const GNetworkedConsumerBenchmarkConfig &config, const std::string &suffix,
                             std::uint32_t nClients) {
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

Gem::Common::GPlotDesigner configurePlotterSleepTimeToOptTime(
        const std::vector<ExTimesSleepAtX> &sleepAtXVec,
        const std::string &title,
        const std::string &xLabel,
        const std::string &yLabel,
        const bool &singlePlot,
        const bool &clientsAtX) {

    const double yMax = getYMax(sleepAtXVec);
    const double yAxisUpperLimit = yMax + (yMax / 50.0); // set upper y-axis limit slightly above the greatest y-value

    // swap shape of vector if required
    std::vector<ExTimesClientsAtX> clientsAtXVec{};
    if (clientsAtX) {
        clientsAtXVec = sleepAtXToClientsAtX(sleepAtXVec);
    }

    std::string legendTitle{};
    if (clientsAtX) {
        legendTitle = "Time for one fitness calculation";
    } else {
        legendTitle = "Number of clients";
    }

    std::uint32_t numberOfEntries = clientsAtX ? clientsAtXVec.size() : sleepAtXVec.size();

    Gem::Common::GPlotDesigner gpd(title,
                                   2,
                                   (singlePlot ? 1 : numberOfEntries));

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

        // set title for the legend which belongs to this graph and all subplots
        asioMainGraph->setLegendTitle(legendTitle);
        mpiMainGraph->setLegendTitle(legendTitle);

        // set the y-axis limits. This defaults to the limits of the y-values. But only for this graph.
        // This means that any subplots would not be visible if their y-values are out of range
        asioMainGraph->setYAxisLimits(0.0, yAxisUpperLimit);
        mpiMainGraph->setYAxisLimits(0.0, yAxisUpperLimit);
        // x-values are equal for each plot, so we can stick to the default range of the main graph

        // set the legends for the first iteration (main graph)
        if (clientsAtX) {
            asioMainGraph->setLegendEntry(std::to_string(clientsAtXVec[0].sleepTime));
            mpiMainGraph->setLegendEntry(std::to_string(clientsAtXVec[0].sleepTime));
        } else {
            asioMainGraph->setLegendEntry(std::to_string(sleepAtXVec[0].nClients));
            mpiMainGraph->setLegendEntry(std::to_string(sleepAtXVec[0].nClients));
        }

        // notify that we want to print the legend for these graphs
        asioMainGraph->setPlotLegend(true);
        mpiMainGraph->setPlotLegend(true);

        // add the data to the graphs
        if (clientsAtX) {
            (*asioMainGraph) & extractMean(clientsAtXVec[0].executionTimesAsio);
            (*mpiMainGraph) & extractMean(clientsAtXVec[0].executionTimesMPI);
        } else {
            (*asioMainGraph) & extractMean(sleepAtXVec[0].executionTimesAsio);
            (*mpiMainGraph) & extractMean(sleepAtXVec[0].executionTimesMPI);
        }

        // add all following graphs as subplots
        for (int i{1}; i < sleepAtXVec.size(); ++i) { // start from second element
            auto asioSubGraph = std::make_shared<Gem::Common::GGraph2D>();
            auto mpiSubGraph = std::make_shared<Gem::Common::GGraph2D>();

            // add the data to the sub-graphs
            if (clientsAtX) {
                (*asioSubGraph) & extractMean(clientsAtXVec[i].executionTimesAsio);
                (*mpiSubGraph) & extractMean(clientsAtXVec[i].executionTimesMPI);
            } else {
                (*asioSubGraph) & extractMean(sleepAtXVec[i].executionTimesAsio);
                (*mpiSubGraph) & extractMean(sleepAtXVec[i].executionTimesMPI);
            }

            // set drawing options
            asioSubGraph->setDrawingArguments("L*");
            mpiSubGraph->setDrawingArguments("L*");

            // set line colors
            asioSubGraph->setLineColor(lineColors[i % lineColors.size()]); // modulo to prevent out of bounds error
            mpiSubGraph->setLineColor(lineColors[i % lineColors.size()]);

            // set the legends for the secondary graphs
            if (clientsAtX) {
                asioSubGraph->setLegendEntry(std::to_string(clientsAtXVec[i].sleepTime));
                mpiSubGraph->setLegendEntry(std::to_string(clientsAtXVec[i].sleepTime));
            } else {
                asioSubGraph->setLegendEntry(std::to_string(sleepAtXVec[i].nClients));
                mpiSubGraph->setLegendEntry(std::to_string(sleepAtXVec[i].nClients));
            }

            // notify that we want to plot the legend for these graphs
            asioSubGraph->setPlotLegend(true);
            mpiSubGraph->setPlotLegend(true);

            // add the sub-graphs to the main-graph
            asioMainGraph->registerSecondaryPlotter(asioSubGraph);
            mpiMainGraph->registerSecondaryPlotter(mpiSubGraph);
        }

        gpd.registerPlotter(asioMainGraph);
        gpd.registerPlotter(mpiMainGraph);

    } else {
        // create a separate plot fore each curve
        for (std::uint32_t i{0}; i < numberOfEntries; ++i) {
            // create two graphs
            auto asioGraph = std::make_shared<Gem::Common::GGraph2ED>();
            auto mpiGraph = std::make_shared<Gem::Common::GGraph2ED>();

            // set labels
            if (clientsAtX) {
                asioGraph->setPlotLabel("Asio sleep time = " + std::to_string(clientsAtXVec[i].sleepTime));
                mpiGraph->setPlotLabel("MPI sleep time = " + std::to_string(clientsAtXVec[i].sleepTime));
            } else {
                asioGraph->setPlotLabel("Asio clients = " + std::to_string(sleepAtXVec[i].nClients));
                mpiGraph->setPlotLabel("MPI clients = " + std::to_string(sleepAtXVec[i].nClients));
            }


            asioGraph->setXAxisLabel(xLabel);
            asioGraph->setYAxisLabel(yLabel);
            mpiGraph->setXAxisLabel(xLabel);
            mpiGraph->setYAxisLabel(yLabel);

            // to compare the graphs better all axis should be equally scaled
            asioGraph->setYAxisLimits(0.0, yAxisUpperLimit);
            mpiGraph->setYAxisLimits(0.0, yAxisUpperLimit);

            // add the data to the graphs
            if (clientsAtX) {
                (*asioGraph) & clientsAtXVec[i].executionTimesAsio;
                (*mpiGraph) & clientsAtXVec[i].executionTimesMPI;
            } else {
                (*asioGraph) & sleepAtXVec[i].executionTimesAsio;
                (*mpiGraph) & sleepAtXVec[i].executionTimesMPI;
            }


            // register graphs with the plotter
            gpd.registerPlotter(asioGraph);
            gpd.registerPlotter(mpiGraph);
        }
    }
    gpd.setCanvasDimensions(800, 1200);

    return gpd;
}

void
plotAbsoluteTimes(const std::vector<ExTimesSleepAtX> &exTimesVec, const GNetworkedConsumerBenchmarkConfig &config) {
    // plot directly with no modification, because values are already absolute

    configurePlotterSleepTimeToOptTime(exTimesVec,
                                       "Absolute time for optimizations for different numbers of consumers and duration of fitness calculation",
                                       "duration of one fitness calculation [s]",
                                       "time needed for one optimization [s]",
                                       true,
                                       false)
            .writeToFile(std::filesystem::path("abs_singlePlot_sleepToOpt" + config.getResultFileName()));

    configurePlotterSleepTimeToOptTime(exTimesVec,
                                       "Absolute time for optimizations for different numbers of consumers and duration of fitness calculation",
                                       "duration of one fitness calculation [s]",
                                       "time needed for one optimization [s]",
                                       false,
                                       false)
            .writeToFile(std::filesystem::path("abs_multiplePlots_sleepToOpt" + config.getResultFileName()));

    configurePlotterSleepTimeToOptTime(exTimesVec,
                                       "Absolute time for optimizations for different numbers of consumers and duration of fitness calculation",
                                       "number of clients",
                                       "time needed for one optimization [s]",
                                       true,
                                       true)
            .writeToFile(std::filesystem::path("abs_singlePlot_clientsToOpt" + config.getResultFileName()));

    configurePlotterSleepTimeToOptTime(exTimesVec,
                                       "Absolute time for optimizations for different numbers of consumers and duration of fitness calculation",
                                       "number of clients",
                                       "time needed for one optimization [s]",
                                       false,
                                       true)
            .writeToFile(std::filesystem::path("abs_multiplePlots_clientsToOpt" + config.getResultFileName()));
}

void combineGraphsToPlot(const GNetworkedConsumerBenchmarkConfig &config) {
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

    std::vector<ExTimesSleepAtX> exTimesVec;
    for (int i{0}; i < exTimesFiles.size(); i += 2) { // iterate in steps of two (get one asio and one mpi file)
        // add execution times for one number of clients to the vector
        exTimesVec.push_back(
                ExTimesSleepAtX{
                        config.getNClients()[i / 2], // the amount of clients used for this run of the subprogram
                        loadExTimesFromFile(
                                exTimesFiles[i].path().string()), // asio ex-times (asio is alphabetically before mpi
                        loadExTimesFromFile(exTimesFiles[i + 1].path().string())
                });
    }

    plotAbsoluteTimes(exTimesVec, config);
}

std::string getHeader(const GNetworkedConsumerBenchmarkConfig &config) {
    std::stringstream sout{};
    sout << "-----------------------------------------" << std::endl
         << "starting " << config.getNClients().size() << " benchmark(s) for asio and mpi" << std::endl
         << "consumer numbers to benchmark: [ " << Gem::Common::vecToString(config.getNClients()) << "]"
         << std::endl
         << "-----------------------------------------" << std::endl;

    return sout.str();
}

int main(int argc, char **argv) {
    GNetworkedConsumerBenchmarkConfig config{argc, argv};

    if (!config.getOnlyGenerateGraphs()) {
        std::cout << getHeader(config) << std::endl;
        resetOutputDirs();

        for (const std::uint32_t &nClients: config.getNClients()) {
//            measureExecutionTimesAsio(config, nClients);
//            renameIntermediateFiles(config, "asio", nClients);
//
//            measureExecutionTimesMPI(config, nClients);
//            renameIntermediateFiles(config, "mpi", nClients);

            for (const auto &competitor: config.getCompetitors()) {
                measureExecutionTimes(config, nClients, competitor);
//                renameIntermediateFiles(config, competitor.shortName, nClients);
            }
        }
    }

    std::cout << "Generating the plots" << std::endl;
//    combineGraphsToPlot(config);

    return 0;
}
