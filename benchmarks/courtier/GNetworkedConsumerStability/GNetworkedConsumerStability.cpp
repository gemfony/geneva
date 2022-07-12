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

    /**
     * returns the maximum point
     * @clientsTerminated true to retrieve the maximum clients terminated, false to retrieve the maximum connections lost
     */
    static std::tuple<std::uint32_t, std::uint32_t>
    max(const std::vector<StabilityStatistic> &stats, const bool &clientsTerminated) {
        std::tuple<std::uint32_t, std::uint32_t> max{0, 0};
        for (const auto &stat: stats) {
            if (clientsTerminated) {
                // vector is sorted, therefore checking the last element is sufficient
                const auto terminated = stat.m_clientsTerminated[stat.m_clientsTerminated.size() - 1];
                if (std::get<1>(terminated) > std::get<1>(max)) { // check greater y-value
                    max = terminated;
                }
            } else {
                // vector is sorted, therefore checking the last element is sufficient
                const auto lost = stat.m_connectionsLost[stat.m_connectionsLost.size() - 1];
                if (std::get<1>(lost) > std::get<1>(max)) { // check greater y-value
                    max = lost;
                }
            }
        }
        return max;
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

// TODO: use boost::asio::asnc_read to read from multiple stream buffers at once and collect their output
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
    child c(command, (std_out & std_err) > pipeStream);

    // analyse the pipe stream to check for client status
    StabilityStatistic stat{analyseClientStatus(config, competitor, pipeStream)};

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
    child server(command, (std_out & std_err) > pipeStream);

    // wait for server to be online before starting clients
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // start nClients clients and store handles in a vector
    std::vector<child> clients{};
    for (int i{0}; i < config.getNClients(); ++i) {
        // this will call the constructor of the boost::process::child class and start the process
        clients.emplace_back(command + " --client", (std_out & std_err) > pipeStream);
    }

    // check pipe-stream for connection issues for a specifies amount of time
    StabilityStatistic stat{analyseClientStatus(config, competitor, pipeStream)};

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

void addGraph(const GNetworkedConsumerStabilityConfig &config,
              const std::vector<StabilityStatistic> &stats,
              Gem::Common::GPlotDesigner &gpd,
              const bool &clientsTerminated) {
    // NOTE: multiple graphs in a single plot can only be done with GGraph2D not with GGraph2ED

    // create one first main graph
    auto mainGraph = std::make_shared<Gem::Common::GGraph2D>();

    // set labels for main graph
    mainGraph->setPlotLabel(stats[0].m_competitor.name);
    mainGraph->setXAxisLabel("Time running [min]");

    if (clientsTerminated) {
        mainGraph->setYAxisLabel("Clients terminated");
    } else {
        mainGraph->setYAxisLabel("Connection losses detected");
    }

    // set drawing arguments
    mainGraph->setDrawingArguments("ALP*");

    // set the line color for the fist iteration
    mainGraph->setLineColor(lineColors[0]);

    // set title for the legend which belongs to this graph and all subplots
    mainGraph->setLegendTitle("Configuration");

    // set the y-axis limit to the greatest y-value of all graphs including subplots.
    // This is necessary because the default would just set it to the greatest y-value of this graph.
    // This then would result in the subplot being not visible if their values are greater than the main plots values
    const auto yMax = std::get<1>(StabilityStatistic::max(stats, clientsTerminated));
    mainGraph->setYAxisLimits(0.0, yMax + yMax / 50.0);
    // x-values are equal for each plot, so we can stick to the default range of the main graph

    // notify that we want to print the legend for the main graph
    mainGraph->setLegendEntry(stats[0].m_competitor.name);
    mainGraph->setPlotLegend(true);

    // add data to main graph
    *mainGraph & (clientsTerminated ? stats[0].m_clientsTerminated : stats[0].m_connectionsLost);

    // add all following graphs as subplots
    for (int i{1} /* start from the second elem */ ; i < stats.size(); ++i) {
        auto subGraph = std::make_shared<Gem::Common::GGraph2D>();

        // add data to subplot
        *subGraph & (clientsTerminated ? stats[i].m_clientsTerminated : stats[i].m_connectionsLost);

        // set drawing options
        subGraph->setDrawingArguments("L*");

        // set line colors
        subGraph->setLineColor(lineColors[i % lineColors.size()]); // modulo to prevent out of bounds error

        // set the legend for the secondary graph
        subGraph->setLegendEntry(stats[i].m_competitor.name);

        // notify that we want to plot the legend for these graphs
        subGraph->setPlotLegend(true);

        // add the sub-graphs to the main-graph
        mainGraph->registerSecondaryPlotter(subGraph);
    }

    // add main graph containing sub-graphs to the plotter
    gpd.registerPlotter(mainGraph);
}

void plotStats(const GNetworkedConsumerStabilityConfig &config,
               std::vector<StabilityStatistic> stats) {
    // shrink all measurements to the requested resolution
    std::for_each(stats.begin(), stats.end(), [](StabilityStatistic &stat) { stat.shrink(graphResolution); });

    // Create plotter
    Gem::Common::GPlotDesigner gpd("Networked Consumer Stability Test", 1, 2);

    // plot client shutdown
    addGraph(config, stats, gpd, true);

    // plot connection losses
    addGraph(config, stats, gpd, false);

    // write to file in ROOT format
    gpd.writeToFile(std::filesystem::path(config.getResultFileName()));
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
    plotStats(config, stats);
    std::cout << "Stability test finished." << std::endl;

    return 0;
}
