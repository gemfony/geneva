/**
 * @file GNetworkedConsumerStability.cpp
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
#include <algorithm>

// Boost header files go here
#include <boost/process.hpp>
#include <utility>

// Geneva header files go here

#include "GNetworkedConsumerStabilityConfig.hpp"
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

/**
 * directory where ORTE (part of openmpi) should store temporary files
 */
const std::string orteTempDirBase{"/tmp/GNetworkedConsumerStability_OPENMPI_ORTE"};

/**
 * Amount of data points to plot. i.e. a resolution for a test with duration 1 hour would result in one data point for each minute
 */
const std::uint32_t graphResolution{30};

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

// forward declare struct
struct ExTimesSleepAtX;

struct ExTimes3D {
    /**
     * <sleep_time, clients, mean> for each competitor
     */
    std::vector<std::vector<std::tuple<double, double, double>>> competitorExTimes;

    /**
     * allows adding the measurements of another instance of this class to this instance.
     * This method assumes that the argument has the same number of competitors as this object
     */
    ExTimes3D &operator+=(const ExTimes3D &rhs) {
        // iterate over all competitors
        for (std::size_t i{0}; i < this->competitorExTimes.size(); ++i) {
            // insert all measurements of rhs for competitor i into the measurements for competitor i of this object
            this->competitorExTimes[i].insert(this->competitorExTimes[i].end(),
                                              rhs.competitorExTimes[i].begin(),
                                              rhs.competitorExTimes[i].end());
        }

        return *this;
    }

    /**
     * creates a ExTimes3D from a vector of ExTimesSleepAtX
     */
    static ExTimes3D fromSleepTimesAtXVec(const std::vector<struct ExTimesSleepAtX> &sleepAtXVec);
};

/**
 * stores the execution times for all competitors for a specific execution time of the fitness function
 */
struct ExTimesClientsAtX {
    double sleepTime;
    /*
     * <clients, error, mean, stddev> for each competitor
     */
    std::vector<std::vector<std::tuple<double, double, double, double>>> competitorExTimes;
};

/**
 * stores the execution times for all competitors for one specific number of clients
 */
struct ExTimesSleepAtX {
    std::uint32_t nClients;
    /*
     * <sleep_time, error, mean, stddev> for each competitor
     */
    std::vector<std::vector<std::tuple<double, double, double, double>>> competitorExTimes;

    /**
     * Converts one ExTimesSleepAtX to a vector of ExTimesClientsAtX.
     */
    [[nodiscard]] std::vector<ExTimesClientsAtX> toClientsAtX() const {
        // output vector
        std::vector<ExTimesClientsAtX> result{};

        // retrieve different values that occur in sleepTimes
        std::vector<double> sleepTimes{};
        std::for_each(this->competitorExTimes[0].begin(), this->competitorExTimes[0].end(),
                      [&sleepTimes](const auto &tup) { sleepTimes.push_back(std::get<0>(tup)); });

        // for each sleep time we must create a new object of type ExTimesClientsAtX
        for (std::size_t i{0}; i < sleepTimes.size(); ++i) {
            result.push_back(
                    ExTimesClientsAtX{
                            sleepTimes[i],
                            std::vector<std::vector<std::tuple<double, double, double, double>>>{} // empty element to fill later
                    });

            // add execution times for this specific sleep time for each competitor
            for (const auto &exTimesSpecificCompetitor: this->competitorExTimes) {
                result.back().competitorExTimes.push_back(
                        std::vector<std::tuple<double, double, double, double>>{
                                std::tuple<double, double, double, double>{
                                        this->nClients, // swap original sleep time with clients
                                        std::get<1>(exTimesSpecificCompetitor[i]),
                                        std::get<2>(exTimesSpecificCompetitor[i]),
                                        std::get<3>(exTimesSpecificCompetitor[i])
                                }
                        });
            }

        }

        return result;
    }

    [[nodiscard]] ExTimes3D to3D() const {
        // create empty result
        ExTimes3D result{};

        // iterate over all competitors
        for (std::size_t i{0}; i < this->competitorExTimes.size(); ++i) {
            // create an empty default constructed entry for this competitor in the result
            result.competitorExTimes.emplace_back();

            // iterate over all measurements for competitor with index i
            for (std::size_t j{0}; j < this->competitorExTimes[i].size(); ++j) {
                // transform the measurement and insert into the result
                const auto tup{this->competitorExTimes[i][j]};
                result.competitorExTimes.back().emplace_back(
                        std::get<0>(tup), // sleep time
                        this->nClients,
                        std::get<2>(tup) // mean
                );
            }
        }

        return result;
    }
};

ExTimes3D ExTimes3D::fromSleepTimesAtXVec(const std::vector<struct ExTimesSleepAtX> &sleepAtXVec) {
    // result initially holds only the 3D representation of the first element of the argument
    ExTimes3D result{sleepAtXVec[0].to3D()};

    // iterate over all remaining arguments and add their 3D representation to the result
    for (std::size_t i{1}; i < sleepAtXVec.size(); ++i) { // start at second element
        result += sleepAtXVec[i].to3D();
    }

    return result;
}

/**
 * Converts a vector of ExTimesSleepAtX to a vector of ExTimesClientsAtX.
 */
std::vector<ExTimesClientsAtX> sleepAtXToClientsAtX(const std::vector<ExTimesSleepAtX> &sleepAtXVec) {
    // create vector which only contains results for one number of clients
    std::vector<ExTimesClientsAtX> result{sleepAtXVec[0].toClientsAtX()};

    // add results from all other client numbers to the first one, starting from index 1
    for (std::size_t i{1}; i < sleepAtXVec.size(); ++i) {
        // create vector that should be added to the result
        const auto toAdd{sleepAtXVec[i].toClientsAtX()};

        // iterate over all execution times
        for (std::size_t j{0}; j < result.size(); ++j) {

            // iterate over all competitors
            for (std::size_t k{0}; k < result[j].competitorExTimes.size(); ++k) {
                // add the new values to the result at the correct position
                result.at(j).competitorExTimes.at(k).push_back(
                        toAdd.at(j).competitorExTimes.at(k)[0]
                );
            }
        }

    }

    return result;
}

/**
 * returns the maximum y-value for a vector of execution times.
 */
double getYMax(const std::vector<ExTimesSleepAtX> &exTimesVec) {
    double maxY{0};

    // iterate over all execution times for all competitors to extract maximum y-value
    for (const auto &exTimes: exTimesVec) {
        for (const auto &competitorExTimes: exTimes.competitorExTimes) {
            for (const auto &tup: competitorExTimes) {
                if (std::get<2>(tup) > maxY) {
                    maxY = std::get<2>(tup);
                }
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

std::string getCommandBanner(const std::string &command,
                             const std::uint32_t &nClients) {
    std::stringstream sout{};
    sout << "-----------------------------------------" << std::endl
         << "running command: `" << command << "` as a new process with " << nClients << " clients"
         << std::endl << "-----------------------------------------" << std::endl;

    return sout.str();
}

/**
 * There is a bug tracked here: `https://github.com/open-mpi/ompi/issues/7049` which can only be solved by cleaning
 * up the temporary files that are left over by ORTE.
 * This function does so.
 */
void cleanUpOrteTemp() {
    std::filesystem::remove_all(orteTempDirBase);
}

void measureExecutionTimesMPI(const GNetworkedConsumerStabilityConfig &config,
                              std::uint32_t nClients,
                              const Competitor &competitor) {
    boost::process::ipstream pipeStream{};

    std::string command = config.getMpirunLocation()
                          + " --oversubscribe -np "
                          + std::to_string(nClients + 1) // one server + nClients
                          // set temp directory in order to be able to clean it
                          + " --mca orte_tmpdir_base " + orteTempDirBase
                          + " " + config.getMStabilityExecutableName()
                          + " " + competitor.arguments
                          // if threads was set to auto, then set it dynamically. Otherwise, set it to the given fixed number
                          + " " + competitor.setThreadsParam
                          + " " + (competitor.nThreads.has_value() ? std::to_string(competitor.nThreads.value())
                                                                   : std::to_string(
                    std::min(nClients, config.getNMaxThreads())));

    // clean up temp directory from previous runs
    cleanUpOrteTemp();

    std::cout << getCommandBanner(command, nClients) << std::endl;

    boost::process::child c(command, boost::process::std_out > pipeStream);

    // pipe std out of mpirun to this process
    std::string line{};
    while (pipeStream && std::getline(pipeStream, line) && !line.empty())
        std::cerr << line << std::endl;

    c.wait();
}

void measureExecutionTimesWithClients(const GNetworkedConsumerStabilityConfig &config,
                                      std::uint32_t nClients,
                                      const Competitor &competitor) {
    boost::process::ipstream pipeStream{};

    std::string command = config.getMStabilityExecutableName()
                          + " " + competitor.arguments
                          + " " + competitor.setThreadsParam
                          + " " + (competitor.nThreads.has_value() ? std::to_string(competitor.nThreads.value())
                                                                   : std::to_string(
                    std::min(nClients, config.getNMaxThreads())));

    std::cout << getCommandBanner(command, nClients) << std::endl;

    // run once without the --client attribute to start a server
    boost::process::child server(command, boost::process::std_out > pipeStream);

    // wait for server to be online before starting clients
    std::this_thread::sleep_for(std::chrono::seconds(1));

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

void renameIntermediateFiles(const GNetworkedConsumerStabilityConfig &config, const std::string &suffix,
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


/**
 * creates a separate plot for each number of clients and each competitor configuration
 */
void createMultiplePlots(const bool &clientsAtX,
                         const std::string &xLabel,
                         const std::string &yLabel,
                         const double yAxisUpperLimit,
                         const GNetworkedConsumerStabilityConfig &config,
                         const std::vector<ExTimesClientsAtX> &clientsAtXVec,
                         const std::vector<ExTimesSleepAtX> &sleepAtXVec,
                         Gem::Common::GPlotDesigner &gpd) {
    for (uint32_t i{0}; i < (clientsAtX ? clientsAtXVec.size() : sleepAtXVec.size()); ++i) {
        // create as many graphs for each client as we have competitor configurations
        for (uint32_t j{0}; j < config.getCompetitors().size(); ++j) {
            auto graph = std::make_shared<Gem::Common::GGraph2ED>();

            const Competitor competitor = config.getCompetitors()[j];

            // set the labels
            if (clientsAtX) {
                graph->setPlotLabel(competitor.name + " sleep time = " + std::to_string(clientsAtXVec[i].sleepTime));
            } else {
                graph->setPlotLabel(competitor.name + " clients = " + std::to_string(sleepAtXVec[i].nClients));
            }

            graph->setXAxisLabel(xLabel);
            graph->setYAxisLabel(yLabel);

            // to compare the graphs better all axis should be equally scaled
            graph->setYAxisLimits(0.0, yAxisUpperLimit);

            // add the data to the graphs
            if (clientsAtX) {
                (*graph) & clientsAtXVec[i].competitorExTimes[j];
            } else {
                (*graph) & sleepAtXVec[i].competitorExTimes[j];
            }

            // register graph with the plotter
            gpd.registerPlotter(graph);
        }

    }
}

/**
 * creates a single plot with all client numbers for each competitor configuration
 */
void createSinglePlot(const bool &clientsAtX,
                      const std::string &xLabel,
                      const std::string &yLabel,
                      const double yAxisUpperLimit,
                      const std::string &legendTitle,
                      const GNetworkedConsumerStabilityConfig &config,
                      const std::vector<ExTimesSleepAtX> &sleepAtXVec,
                      const std::vector<ExTimesClientsAtX> &clientsAtXVec,
                      Gem::Common::GPlotDesigner &gpd) {
    // NOTE: multiple graphs in a single plot can only be done with GGraph2D not with GGraph2ED

    // create one graph for each competitor configuration
    for (std::int32_t i{0}; i < config.getCompetitors().size(); ++i) {
        const Competitor competitor = config.getCompetitors()[i];

        // create one first main graph

        auto mainGraph = std::make_shared<Gem::Common::GGraph2D>();

        // set labels for main graph
        mainGraph->setPlotLabel(competitor.name);
        mainGraph->setXAxisLabel(xLabel);
        mainGraph->setYAxisLabel(yLabel);

        // set drawing arguments
        mainGraph->setDrawingArguments("ALP*");

        // set the line color for the fist iteration
        mainGraph->setLineColor(lineColors[0]);

        // set title for the legend which belongs to this graph and all subplots
        mainGraph->setLegendTitle(legendTitle);

        // set the y-axis limit to the greatest y-value of all graphs including subplots.
        // This is necessary because the default would just set it to the greatest y-value of this graph.
        // This then would result in the subplot being not visible if their values are greater than the main plots values
        mainGraph->setYAxisLimits(0.0, yAxisUpperLimit);
        // x-values are equal for each plot, so we can stick to the default range of the main graph

        // set the legend for the first iteration (main graph)
        if (clientsAtX) {
            mainGraph->setLegendEntry(std::to_string(clientsAtXVec[0].sleepTime));
        } else {
            mainGraph->setLegendEntry(std::to_string(sleepAtXVec[0].nClients));
        }

        // notify that we want to print the legend for the main graph
        mainGraph->setPlotLegend(true);

        // add the data to the graph
        if (clientsAtX) {
            (*mainGraph) & extractMean(clientsAtXVec[0].competitorExTimes[i]);
        } else {
            (*mainGraph) & extractMean(sleepAtXVec[0].competitorExTimes[i]);
        }

        // add all following graphs as subplots
        for (int j{1} /* start from the second elem */ ;
             j < (clientsAtX ? clientsAtXVec.size() : sleepAtXVec.size()); ++j) {
            auto subGraph = std::make_shared<Gem::Common::GGraph2D>();

            // add the data to the sub-graphs
            if (clientsAtX) {
                (*subGraph) & extractMean(clientsAtXVec[j].competitorExTimes[i]);
            } else {
                (*subGraph) & extractMean(sleepAtXVec[j].competitorExTimes[i]);
            }

            // set drawing options
            subGraph->setDrawingArguments("L*");

            // set line colors
            subGraph->setLineColor(lineColors[j % lineColors.size()]); // modulo to prevent out of bounds error

            // set the legend for the secondary graph
            if (clientsAtX) {
                subGraph->setLegendEntry(std::to_string(clientsAtXVec[j].sleepTime));
            } else {
                subGraph->setLegendEntry(std::to_string(sleepAtXVec[j].nClients));
            }

            // notify that we want to plot the legend for these graphs
            subGraph->setPlotLegend(true);

            // add the sub-graphs to the main-graph
            mainGraph->registerSecondaryPlotter(subGraph);
        }

        // add main graph containing sub-graphs to the plotter
        gpd.registerPlotter(mainGraph);
    }
}

Gem::Common::GPlotDesigner configurePlotter2D(
        const std::vector<ExTimesSleepAtX> &sleepAtXVec,
        const std::string &title,
        const std::string &xLabel,
        const std::string &yLabel,
        const bool &singlePlot,
        const bool &clientsAtX,
        const GNetworkedConsumerStabilityConfig &config) {

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

    // one row for each competitor or one row for each combination of competitor and clients
    std::uint32_t nRows = (singlePlot ? config.getCompetitors().size() :
                           config.getCompetitors().size() * config.getNClients().size());

    // initialize empty plotter
    Gem::Common::GPlotDesigner gpd(title, 1, nRows);

    // add graphs to plotter
    if (singlePlot) {
        createSinglePlot(clientsAtX, xLabel, yLabel, yAxisUpperLimit, legendTitle, config, sleepAtXVec,
                         clientsAtXVec, gpd);

    } else {
        createMultiplePlots(clientsAtX, xLabel, yLabel, yAxisUpperLimit, config, clientsAtXVec, sleepAtXVec, gpd);
    }

    gpd.setCanvasDimensions(1920, 1163 * nRows);

    return gpd;
}

Gem::Common::GPlotDesigner configurePlotter3D(
        const std::vector<ExTimesSleepAtX> &sleepAtXVec,
        const std::string &title,
        const std::string &xLabel,
        const std::string &yLabel,
        const std::string &zLabel,
        const GNetworkedConsumerStabilityConfig &config) {

    const std::size_t nRows{config.getCompetitors().size()};

    Gem::Common::GPlotDesigner gpd(title, 1, nRows);

    // convert data to 3D representation
    const auto measurements3D{ExTimes3D::fromSleepTimesAtXVec(sleepAtXVec)};

    const double yMax = getYMax(sleepAtXVec);

    // create one graph for each competitor
    for (std::size_t i{0}; i < measurements3D.competitorExTimes.size(); ++i) {
        // create empty default constructed graph
        auto graph = std::make_shared<Gem::Common::GGraph3D>();

        // set the style of the graph
        graph->setDrawingArguments("surf1");

        // set the labels
        graph->setPlotLabel(config.getCompetitors()[i].name);
        graph->setXAxisLabel(xLabel);
        graph->setYAxisLabel(yLabel);
        graph->setZAxisLabel(zLabel);

        // set Z-axis limit to the same value of all graphs which is slightly above the overall maximum value that occurs
        graph->setZAxisLimits(0.0, yMax + (yMax / 50));

        (*graph) & measurements3D.competitorExTimes[i];

        gpd.registerPlotter(graph);

    }

    gpd.setCanvasDimensions(1920, 1163 * nRows);

    return gpd;
}

void plotAbsoluteTimes(const std::vector<ExTimesSleepAtX> &exTimesVec,
                       const GNetworkedConsumerStabilityConfig &config) {
    // plot directly with no modification, because values are already absolute

    const std::string title{
            "Absolute time for optimizations for different numbers of consumers and duration of fitness calculation"};
    const std::string labelResult{"duration of one optimization [s]"};
    const std::string labelSleepTime{"duration of one fitness calculation [s]"};
    const std::string labelClients{"number of clients"};

    configurePlotter2D(exTimesVec,
                       title,
                       labelSleepTime,
                       labelResult,
                       true,
                       false,
                       config)
            .writeToFile(std::filesystem::path("abs_2D_singlePlot_sleepToOpt_" + config.getResultFileName()));

    configurePlotter2D(exTimesVec,
                       title,
                       labelSleepTime,
                       labelResult,
                       false,
                       false,
                       config)
            .writeToFile(std::filesystem::path("abs_2D_multiplePlots_sleepToOpt_" + config.getResultFileName()));

    configurePlotter2D(exTimesVec,
                       title,
                       labelClients,
                       labelResult,
                       true,
                       true,
                       config)
            .writeToFile(std::filesystem::path("abs_2D_singlePlot_clientsToOpt_" + config.getResultFileName()));

    configurePlotter2D(exTimesVec,
                       title,
                       labelClients,
                       labelResult,
                       false,
                       true,
                       config)
            .writeToFile(std::filesystem::path("abs_2D_multiplePlots_clientsToOpt_" + config.getResultFileName()));

    configurePlotter3D(
            exTimesVec,
            title,
            labelSleepTime,
            labelClients,
            labelResult,
            config)
            .writeToFile(std::filesystem::path("abs_3D_" + config.getResultFileName()));
}

void createPlotFromResults(const GNetworkedConsumerStabilityConfig &config) {
    namespace fs = std::filesystem;

    fs::path executionTimesDir = fs::current_path() / executionTimesDirName;


    Gem::Common::GPlotDesigner gpd("Processing times for different evaluation times of individuals ", 2,
                                   config.getNClients().size());


    // insert all file entries into a vector
    std::vector<fs::directory_entry> exTimesFiles{};
    for (auto const &dir_entry: std::filesystem::directory_iterator{executionTimesDir}) {
        exTimesFiles.push_back(dir_entry);
    }

    // sort by names, name prefix indicates number of clients -> sort by clients and then by competitor configuration
    std::sort(exTimesFiles.begin(), exTimesFiles.end());

    std::vector<ExTimesSleepAtX> exTimesVec;

    // iterate over all result files
    // iterate in steps of competitor size and access elements in between in inner loop
    for (std::size_t i{0}; i < exTimesFiles.size(); i += config.getCompetitors().size()) {
        // for each number of clients, create a vector with execution times of all competitors

        // create element with empty execution times first
        exTimesVec.push_back(
                ExTimesSleepAtX{
                        config.getNClients()[i / config.getCompetitors().size()],
                        std::vector<std::vector<std::tuple<double, double, double, double>>>{}
                });

        // fill exTimes with values of all competitors
        for (std::size_t j{0}; j < config.getCompetitors().size(); ++j) {
            exTimesVec.back().competitorExTimes.push_back(
                    loadExTimesFromFile(exTimesFiles[i + j].path().string()));

        }
    }

    plotAbsoluteTimes(exTimesVec, config);
}

std::string timeNowString(std::chrono::system_clock::time_point time) {

    auto in_time_t = std::chrono::system_clock::to_time_t(time);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

class StabilityStatistic {
public:
    /**
     * competitor which this statistic belongs to
     */
    Competitor m_competitor;
    /**
     * Vector of elements: <minutes elapsed, connection issues detected until this time>
     */
    std::vector<std::tuple<std::uint32_t, std::uint32_t>> m_connectionsLost;
    /**
     * Vector of elements: <minutes elapsed, clients lost until this time>
     */
    std::vector<std::tuple<std::uint32_t, std::uint32_t>> m_clientsTerminated;
    /**
     * amount of data points stored (if any)
     */
    std::uint32_t m_resolution;

    StabilityStatistic() = delete;

    /**
     * Initializes the struct with a competitor and a given resolution
     * @param c competitor that this statistic belongs to
     * @param resolution the amount of data points stored in the vectors
     */
    explicit StabilityStatistic(Competitor c, std::uint32_t resolution)
            : m_competitor{std::move(c)},
              m_resolution{resolution},
              m_connectionsLost{},
              m_clientsTerminated{} {

        // initialize vectors with the required length in case the competitor is able to test the given property
        if (!m_competitor.connectionIssuesSubstring.empty()) {
            for (std::uint32_t i{0}; i < resolution; ++i) {
                m_connectionsLost.emplace_back(i, 0);
            }
        }
        if (!m_competitor.terminationSubString.empty()) {
            for (std::uint32_t i{0}; i < resolution; ++i) {
                m_clientsTerminated.emplace_back(i, 0);
            }
        }
    }

    /**
     * Shrinks the vectors such that they will have `resolution` elements.
     * Consecutive elements are grouped. The resulting x-value is the smallest x-value of the group.
     * The resulting y-value is the sum of all y-values of the group.
     * If any elements at the end are left over (due to non-divisibility), they are ignored
     * @param resolution number of elements in the vectors of the output object
     * @return self
     */
    StabilityStatistic &shrink(const std::uint32_t &resolution) {
        // shrink members
        m_connectionsLost = StabilityStatistic::shrink(m_connectionsLost, resolution);
        m_clientsTerminated = StabilityStatistic::shrink(m_clientsTerminated, resolution);

        m_resolution = resolution;

        // return self for chaining
        return *this;
    }

    /**
     * returns true if each x-axis tick represents one minute
     */
    [[nodiscard]] bool isMinuteScaled() const {
        if (!m_clientsTerminated.empty() && m_resolution >= 2) {
            return std::get<0>(m_clientsTerminated[0]) - std::get<0>(m_clientsTerminated[1]) == 1;
        }
        return false;
    }

private:

    static std::vector<std::tuple<std::uint32_t, std::uint32_t>> shrink(
            const std::vector<std::tuple<std::uint32_t, std::uint32_t>> &stat,
            const std::uint32_t &resolution) {
        if (!stat.empty() && resolution >= stat.size()) {
            throw std::invalid_argument{"Cannot shrink to a resolution greater or equal to number of data points"};
        }

        // distance of x-values of the result vector
        std::uint64_t stepWidth{stat.size() / resolution};
        std::vector<std::tuple<std::uint32_t, std::uint32_t>> result{};

        if (!stat.empty()) { // if empty we will not iterate and leave vector empty
            for (std::uint32_t i{0}; i < stat.size(); i += stepWidth) {
                // sum up all y-values until next tick
                std::uint32_t sum{0};
                for (std::uint32_t j{i}; j < j + stepWidth - 1; ++j) {
                    sum += std::get<1>(stat[j]);
                }
                // add x-value of first element of group and sum of y-values of elements of group
                result.emplace_back(i, sum);
            }
        }

        return result;
    }
};

enum ClientStatus {
    OK,
    CONNECTION_LOSS,
    SHUTDOWN
};

ClientStatus parseClientStatus(const Competitor &competitor, const std::string &line) {
    if (!competitor.connectionIssuesSubstring.empty() &&
        line.find(competitor.connectionIssuesSubstring) != std::string::npos) {
        return ClientStatus::CONNECTION_LOSS;
    }

    if (!competitor.terminationSubString.empty() &&
        line.find(competitor.terminationSubString) != std::string::npos) {
        return ClientStatus::SHUTDOWN;
    }

    return ClientStatus::OK;
}

void incrementStatNow(StabilityStatistic &stat,
                      const std::chrono::system_clock::time_point &timeStart,
                      const ClientStatus &status) {
    if (!stat.isMinuteScaled()) {
        throw std::invalid_argument{"Can only increment minute-scaled statistics."};
    }

    using namespace std::chrono;

    std::int64_t minutesElapsed{duration_cast<minutes>(system_clock::now() - timeStart).count()};

    // create pointer to vector which needs to be incremented
    auto vecPtr = status == CONNECTION_LOSS ? &stat.m_connectionsLost : &stat.m_clientsTerminated;

    // increment y-value for the current minute and all following minutes in the collection
    for (std::int64_t i{minutesElapsed}; i < vecPtr->size(); ++i) {
        // increment y-value
        std::get<1>((*vecPtr)[i]);
    }
}

StabilityStatistic analyseClientStatus(const GNetworkedConsumerStabilityConfig &config,
                         const Competitor &competitor,
                         boost::process::ipstream &pipeStream) {
    using namespace std::chrono;

    const auto timeStart = system_clock::now();
    const auto timeEnd = timeStart + minutes(config.getDuration().totalMinutes());

    // create stat object for the competitor with the given runtime in minutes
    StabilityStatistic stat{competitor, config.getDuration().totalMinutes()};

    std::string line{};

    while (system_clock::now() < timeEnd
           && pipeStream
           && std::getline(pipeStream, line)
           && !line.empty()) {
        switch (parseClientStatus(competitor, line)) {
            case OK: // do nothing if everything is ok
                break;
            case CONNECTION_LOSS:
                std::cout << "client connection loss detected at " << timeNowString(system_clock::now())
                          << std::endl;
                incrementStatNow(stat, timeStart, CONNECTION_LOSS);
                break;
            case SHUTDOWN:
                std::cout << "client shutdown detected at " << timeNowString(system_clock::now())
                          << std::endl;
                incrementStatNow(stat, timeStart, SHUTDOWN);
                break;
            default:
                break;
        }
    }

    return stat;
}

StabilityStatistic runTestMPI(const GNetworkedConsumerStabilityConfig &config,
                const Competitor &competitor) {
    using namespace boost::process;

    ipstream pipeStream{};

    std::string command = config.getMpirunLocation()
                          + " --oversubscribe -np "
                          + std::to_string(config.getNClients() + 1) // one server + nClients
                          // set temp directory in order to be able to clean it
                          + " --mca orte_tmpdir_base " + orteTempDirBase
                          + " " + config.getTestExecutableName()
                          + " " + competitor.arguments;

    // clean up temp directory from previous runs
    cleanUpOrteTemp();

    std::cout << getCommandBanner(command, config.getNClients()) << std::endl;

    // start sub-process by creating an object of type boost::process::child
    child c(command, std_out > pipeStream, std_err > pipeStream);

    // analyse the pipe stream to check for client status
    StabilityStatistic stat {analyseClientStatus(config, competitor, pipeStream)};

    // kill processes after time for analyzing has elapsed
    std::cout << "Time for testing this competitor configuration has elapsed. Killing child process..." << std::endl;
    c.terminate();

    // wait until child processes have exited
    c.wait();

    std::cout << "Test for this configuration completed." << std::endl;

    return stat;
}

StabilityStatistic runTestWithClients(const GNetworkedConsumerStabilityConfig &config,
                        const Competitor &competitor) {
    using namespace boost::process;

    ipstream pipeStream{};

    std::string command = config.getTestExecutableName()
                          + " " + competitor.arguments;

    std::cout << getCommandBanner(command, config.getNClients()) << std::endl;

    // run once without the --client attribute to start a server
    child server(command, std_out > pipeStream, std_err > pipeStream);

    // wait for server to be online before starting clients
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // start nClients clients and store handles in a vector
    std::vector<child> clients{};
    for (int i{0}; i < config.getNClients(); ++i) {
        // this will call the constructor of the boost::process::child class and start the process
        clients.emplace_back(command + " --client", std_out > pipeStream, std_err > pipeStream);
    }

    // check pipe-stream for connection issues for a specifies amount of time
    StabilityStatistic stat {analyseClientStatus(config, competitor, pipeStream)};

    // kill all processes after analyzing has been finished
    std::cout << "Time for testing this competitor configuration has elapsed. Killing child process..." << std::endl;
    server.terminate();
    std::for_each(clients.begin(), clients.end(), [](child &client) { client.terminate(); });

    // wait for the completion of all processes
    server.wait();
    std::for_each(clients.begin(), clients.end(), [](child &client) { client.wait(); });

    std::cout << "Test for this configuration completed." << std::endl;

    return stat;
}

StabilityStatistic runTest(const GNetworkedConsumerStabilityConfig &config, const Competitor &competitor) {
    using namespace std::chrono;

    if (competitor.arguments.find("--consumer mpi") != std::string::npos) {
        // mpi must be started differently
        return runTestMPI(config, competitor);
    } else {
        return runTestWithClients(config, competitor);
    }
}

std::string getHeader(const GNetworkedConsumerStabilityConfig &config) {
    std::stringstream sout{};
    sout << "-----------------------------------------" << std::endl
         << "starting stability test for the following configuration:" << std::endl
         << config;

    return sout.str();
}

int main(int argc, char **argv) {
    GNetworkedConsumerStabilityConfig config{argc, argv};

    // Sort all collections. Later on we can therefore assume that e.g. the competitors are alphabetically sorted
    config.sortAll();


    std::cout << getHeader(config) << std::endl;

    std::vector<StabilityStatistic> stats{};

    for (const Competitor &c: config.getCompetitors()) {
        stats.push_back(runTest(config, c));
    }

    std::cout << "Generating the plots ..." << std::endl;
    // TODO: generate plots
    std::cout << "Stability test finished." << std::endl;

    return 0;
}
