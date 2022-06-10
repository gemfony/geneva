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

void measureExecutionTimesMPI(const GNetworkedConsumerBenchmarkConfig &config,
                              std::uint32_t nClients,
                              const Competitor &competitor) {
    boost::process::ipstream pipeStream{};

    std::string command = config.getMpirunLocation()
                          + " --oversubscribe -np "
                          + std::to_string(nClients + 1) // one server + nClients
                          + " "
                          + config.getMBenchmarkExecutableName()
                          + " " + competitor.arguments
                          // if threads was set to auto, then set it dynamically. Otherwise, set it to the given fixed number
                          + " " + competitor.setThreadsParam
                          + " " + (competitor.nThreads.has_value() ? std::to_string(competitor.nThreads.value())
                                                                   : std::to_string(nClients));

    std::cout << getCommandBanner(command, nClients) << std::endl;

    boost::process::child c(command, boost::process::std_out > pipeStream);

    // pipe std out of mpirun to this process
    std::string line{};
    while (pipeStream && std::getline(pipeStream, line) && !line.empty())
        std::cerr << line << std::endl;

    c.wait();
}

void measureExecutionTimesWithClients(const GNetworkedConsumerBenchmarkConfig &config,
                                      std::uint32_t nClients,
                                      const Competitor &competitor) {
    boost::process::ipstream pipeStream{};

    std::string command = config.getMBenchmarkExecutableName()
                          + " " + competitor.arguments
                          + " " + competitor.setThreadsParam
                          + " " + (competitor.nThreads.has_value() ? std::to_string(competitor.nThreads.value())
                                                                   : std::to_string(nClients));

    std::cout << getCommandBanner(command, nClients) << std::endl;

    // run once without the --client attribute to start a server
    boost::process::child server(command, boost::process::std_out > pipeStream);

    // wait for server to be online before starting clients
    std::this_thread::sleep_for(std::chrono::seconds(5));

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

void measureExecutionTimes(const GNetworkedConsumerBenchmarkConfig &config,
                           std::uint32_t nClients,
                           const Competitor &competitor) {
    if (competitor.arguments.find("--consumer mpi") != std::string::npos) {
        // mpi must be started differently
        measureExecutionTimesMPI(config, nClients, competitor);
    } else {
        measureExecutionTimesWithClients(config, nClients, competitor);
    }

    std::cout << nClients << " " << competitor.name << std::endl;
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


/**
 * creates a separate plot for each number of clients and each competitor configuration
 */
void createMultiplePlots(const bool &clientsAtX,
                         const std::string &xLabel,
                         const std::string &yLabel,
                         const double yAxisUpperLimit,
                         const GNetworkedConsumerBenchmarkConfig &config,
                         const std::vector<ExTimesClientsAtX> &clientsAtXVec,
                         const std::vector<ExTimesSleepAtX> &sleepAtXVec,
                         Gem::Common::GPlotDesigner &gpd) {
    for (uint32_t i{0}; i < config.getNClients().size(); ++i) {
        // create as many graphs for each client as we have competitor configurations
        for (uint32_t j{0}; j < config.getCompetitors().size(); ++j) {
            auto graph = std::make_shared<Gem::Common::GGraph2ED>();

            const Competitor competitor = config.getCompetitors()[j];

            // set the labels
            if (clientsAtX) {
                graph->setPlotLabel(competitor.name + " sleep time = " + std::to_string(clientsAtXVec[i].sleepTime));
            } else {
                graph->setPlotLabel(competitor.name + " sleep time = " + std::to_string(sleepAtXVec[i].nClients));
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
                      const GNetworkedConsumerBenchmarkConfig &config,
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
        for (int j{1} /* start from the second elem */ ; j < sleepAtXVec.size(); ++j) {
            auto subGraph = std::make_shared<Gem::Common::GGraph2D>();

            // add the data to the sub-graphs
            if (clientsAtX) {
                (*subGraph) & extractMean(clientsAtXVec[j].competitorExTimes[j]);
            } else {
                (*subGraph) & extractMean(sleepAtXVec[j].competitorExTimes[j]);
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
        const GNetworkedConsumerBenchmarkConfig &config) {

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
    Gem::Common::GPlotDesigner gpd(title, 2, nRows);

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
        const GNetworkedConsumerBenchmarkConfig &config) {
    Gem::Common::GPlotDesigner gpd(title, 1, 1);

    // TODO: turn real data into 3-dimensional form and use it instead of dummy
    const std::vector<std::tuple<double, double, double>> dummy{
            {2.0, 1.0, 4.0},
            {1.0, 0.5, 2.0},
            {0.0, 0.0, 1.0},
            {0.5, 1.0, 3.0},
            {1.0, 5.0, 5.0}
    };

    const auto measurements3D{ExTimes3D::fromSleepTimesAtXVec(sleepAtXVec)};

    auto graph = std::make_shared<Gem::Common::GGraph3D>();

    graph->setDrawingArguments("surf1");
    graph->setXAxisLabel(xLabel);
    graph->setYAxisLabel(yLabel);
    graph->setZAxisLabel(zLabel);

    graph->setZAxisLimits(0.0, 25.0);

    (*graph) & measurements3D.competitorExTimes[0];

    gpd.registerPlotter(graph);

    gpd.setCanvasDimensions(1920, 1163);

    return gpd;
}

void plotAbsoluteTimes(const std::vector<ExTimesSleepAtX> &exTimesVec,
                       const GNetworkedConsumerBenchmarkConfig &config) {
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

void createPlotFromResults(const GNetworkedConsumerBenchmarkConfig &config) {
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

std::string getHeader(const GNetworkedConsumerBenchmarkConfig &config) {
    const std::string indent{"     "};

    std::stringstream sout{};
    sout << "-----------------------------------------" << std::endl
         << "starting " << config.getNClients().size() << " benchmark(s) for the following configurations:"
         << std::endl;

    std::for_each(config.getCompetitors().begin(), config.getCompetitors().end(),
                  [&indent, &sout](const auto &competitor) {
                      sout << indent << competitor << std::endl;
                  });

    sout << "consumer numbers to benchmark: [ " << Gem::Common::vecToString(config.getNClients()) << "]"
         << std::endl
         << "-----------------------------------------" << std::endl;

    return sout.str();
}

int main(int argc, char **argv) {
    GNetworkedConsumerBenchmarkConfig config{argc, argv};

    // Sort all collections. Later on we can therefore assume that e.g. the competitors are alphabetically sorted
    config.sortAll();

    if (!config.getOnlyGenerateGraphs()) {
        std::cout << getHeader(config) << std::endl;
        resetOutputDirs();

        for (const std::uint32_t &nClients: config.getNClients()) {
            for (const auto &competitor: config.getCompetitors()) {
                measureExecutionTimes(config, nClients, competitor);
                renameIntermediateFiles(config, competitor.shortName, nClients);
            }
        }
    }

    std::cout << "Generating the plots ..." << std::endl;
    createPlotFromResults(config);
    std::cout << "Benchmark finished." << std::endl;

    return 0;
}
