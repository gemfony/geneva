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
#include <boost/asio.hpp>
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
 * Name of the backup file
 */
const std::string backupFileName{"stats.ser"};

const std::uint32_t waitForServerStartupSec{15};

/**
 * Amount of data points to plot. i.e. a resolution for a test with duration 1 hour would result in one data point for each minute
 */
const std::uint32_t graphResolution{30};

// line color so be used when drawing multiple curves in the same graph
// These are ROOT constants
const std::vector<std::string> lineColors{
        "kBlack",
        "kRed",
        "kGreen",
        "kBlue",
        "kGray",
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
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & BOOST_SERIALIZATION_NVP(m_competitor)
        & BOOST_SERIALIZATION_NVP(m_connectionsLost)
        & BOOST_SERIALIZATION_NVP(m_clientsTerminated)
        & BOOST_SERIALIZATION_NVP(m_resolution);
    }

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

    /**
     * required for serialization using boost
     */
    StabilityStatistic() = default;

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
            // check if the distance of data points along the x-axis is equal to one
            return std::get<0>(m_clientsTerminated[1]) - std::get<0>(m_clientsTerminated[0]) == 1;
        }
        return true;
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
                if (stat.m_clientsTerminated.empty()) {
                    continue; // do not check empty vectors (not applicable)
                }
                // vector is sorted, therefore checking the last element is sufficient
                const auto terminated = stat.m_clientsTerminated[stat.m_clientsTerminated.size() - 1];
                if (std::get<1>(terminated) > std::get<1>(max)) { // check greater y-value
                    max = terminated;
                }
            } else {
                if (stat.m_connectionsLost.empty()) {
                    continue; // do not check empty vectors (not applicable)
                }
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

    /**
     * returns a smaller representation of the same result
     * @param stat the statistic to shrink
     * @param resolution the amount of data points in the result statistic
     * @return the result statistic with `resolution` elements
     */
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
                // for the new point use the greatest datapoint in this tick
                // because the function is monotonous, the greatest datapoint is the last data-point in this group of points
                result.emplace_back(i, std::get<1>(stat[i + stepWidth - 1]));
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
        ++std::get<1>((*vecPtr)[i]);
    }
}

void analyseClientStatus(const GNetworkedConsumerStabilityConfig &config,
                         const std::chrono::system_clock::time_point &timeStart,
                         StabilityStatistic &stat,
                         boost::asio::streambuf &streamBuf) {
    using namespace std::chrono;

    std::string line{};

    while (std::getline(std::istream(&streamBuf), line)) {
        switch (parseClientStatus(stat.m_competitor, line)) {
            case OK: // do nothing if everything is ok
                std::cout << "OK: " << line << std::endl;
                break;
            case CONNECTION_LOSS:
                std::cout << "CONNECTION_LOSS: detected at " << timeNowString(system_clock::now())
                          << std::endl;
                incrementStatNow(stat, timeStart, CONNECTION_LOSS);
                break;
            case SHUTDOWN:
                std::cout << "SHUT_DOWN: detected at " << timeNowString(system_clock::now())
                          << std::endl;
                incrementStatNow(stat, timeStart, SHUTDOWN);
                break;
            default:
                break;
        }
    }
}

void registerReadCallback(boost::asio::streambuf &streamBuf,
                          boost::process::async_pipe &pipe,
                          const GNetworkedConsumerStabilityConfig &config,
                          const std::chrono::system_clock::time_point &timeStart,
                          StabilityStatistic &stat) {
    boost::asio::async_read_until(pipe,
                                  streamBuf,
                                  '\n',
                                  [&streamBuf, &pipe, &config, &timeStart, &stat]
                                          (const boost::system::error_code &ec, std::size_t size) {
                                      if (ec && ec.value() != boost::asio::error::eof) {
                                          std::cout << "Error when reading asynchronously: " << ec.message()
                                                    << std::endl;
                                      } else {
                                          // read all available lines in this callback
                                          analyseClientStatus(config, timeStart, stat, streamBuf);

                                          if (ec.value() != boost::asio::error::eof) {
                                              // register handler again if eaf not reached yet
                                              registerReadCallback(streamBuf, pipe, config, timeStart, stat);
                                          }
                                      }
                                  });
}

StabilityStatistic runTestMPI(const GNetworkedConsumerStabilityConfig &config,
                              const Competitor &competitor) {
    namespace bp = boost::process;
    namespace basio = boost::asio;

    const auto timeStart{std::chrono::system_clock::now()};

    // result statistic
    StabilityStatistic resultStat{competitor, config.getDuration().totalMinutes()};

    // objects needed for reading from pipe asynchronously
    basio::io_service ios{};
    bp::async_pipe pipe{ios};
    basio::streambuf streamBuf{};

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
    bp::child c(command, (bp::std_out & bp::std_err) > pipe);

    // register handler for asynchronous read on stream buffer
    registerReadCallback(streamBuf, pipe, config, timeStart, resultStat);

    // run the io_service on a new thread to handle the pipe events
    auto ioRun = std::async(std::launch::async, [&ios]() { ios.run(); });

    // sleep for the duration of the run
    std::this_thread::sleep_for(std::chrono::minutes(config.getDuration().totalMinutes()));

    // stop the io service and wait for its termination
    ios.stop();
    ioRun.wait();

    // kill processes after analyzing has been finished
    std::cout << "Time for testing this competitor configuration has elapsed. Killing child process..." << std::endl;
    c.terminate();

    // wait until child processes have exited
    c.wait();

    std::cout << "Test for this configuration completed." << std::endl;

    return resultStat;
}

void registerReadCallback(boost::asio::streambuf &streamBuf,
                          boost::process::async_pipe &pipe,
                          const GNetworkedConsumerStabilityConfig &config,
                          const std::chrono::system_clock::time_point &timeStart,
                          StabilityStatistic &stat,
                          std::mutex &lock) {
    boost::asio::async_read_until(pipe,
                                  streamBuf,
                                  '\n',
                                  [&streamBuf, &pipe, &config, &timeStart, &stat, &lock]
                                          (const boost::system::error_code &ec, std::size_t size) {
                                      if (ec && ec.value() != boost::asio::error::eof) {
                                          std::cout << "Error when reading asynchronously: " << ec.message()
                                                    << std::endl;
                                      } else {
                                          // protect access to statistic when reading from multiple handlers concurrently
                                          std::lock_guard<std::mutex> guard{lock};

                                          // read all available lines in this callback
                                          analyseClientStatus(config, timeStart, stat, streamBuf);

                                          if (ec.value() != boost::asio::error::eof) {
                                              // register handler again if eaf not reached yet
                                              registerReadCallback(streamBuf, pipe, config, timeStart, stat, lock);
                                          }
                                      }
                                  });
}

StabilityStatistic runTestWithClients(const GNetworkedConsumerStabilityConfig &config,
                                      const Competitor &competitor) {
    namespace bp = boost::process;
    namespace basio = boost::asio;

    const auto timeStart{std::chrono::system_clock::now()};

    // result statistic
    StabilityStatistic resultStat{competitor, config.getDuration().totalMinutes()};
    // mutex to protect the statistic when accessing from multiple concurrent reader callbacks
    std::mutex statLock{};

    // io service handling network events
    basio::io_service ios{};

    // vector of child processes, one for the server and one for each client
    std::vector<bp::child> processes{};
    // vector of buffers, one for each process
    std::vector<std::shared_ptr<basio::streambuf>> buffers{};
    // vector of pipes, one for each process
    std::vector<std::shared_ptr<bp::async_pipe>> pipes{};

    std::string command = config.getTestExecutableName()
                          + " " + competitor.arguments;

    std::cout << getCommandBanner(command, config.getNClients()) << std::endl;

    for (std::uint32_t i{0}; i < config.getNClients() + 1; ++i) {
        buffers.push_back(std::make_shared<basio::streambuf>());
        pipes.push_back(std::make_shared<bp::async_pipe>(ios));
        processes.emplace_back(command + (i == 0 ? "" : " --client"), (bp::std_out & bp::std_err) > *pipes[i]);

        // register read handler for this pipe
        registerReadCallback(*buffers[i], *pipes[i], config, timeStart, resultStat, statLock);

        if (i == 0) { // server
            std::this_thread::sleep_for(std::chrono::seconds(waitForServerStartupSec)); // wait until server is up before starting clients
        }
    }

    // run the io_service on a new thread to handle the pipe events
    auto ioRun = std::async(std::launch::async, [&ios]() { ios.run(); });

    // sleep for the duration of the run
    std::this_thread::sleep_for(std::chrono::minutes(config.getDuration().totalMinutes()));

    // stop the io service and wait for its termination
    ios.stop();
    ioRun.wait();

    // kill all processes after analyzing has been finished
    std::cout << "Time for testing this competitor configuration has elapsed. Killing child processes..." << std::endl;
    std::for_each(processes.begin(), processes.end(), [](bp::child &client) { client.terminate(); });

    // wait for the completion of all processes
    std::for_each(processes.begin(), processes.end(), [](bp::child &client) { client.wait(); });

    std::cout << "Test for this configuration completed." << std::endl;

    return resultStat;
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
    mainGraph->setPlotLabel(clientsTerminated ? "Client Termination" : "Client Connection Loss");
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

    // only show the legend if this graph has any data points
    mainGraph->setPlotLegend((clientsTerminated && !stats[0].m_clientsTerminated.empty()) ||
                              (!clientsTerminated && !stats[0].m_connectionsLost.empty()));

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

        // only show the legend if this graph has any data points
        subGraph->setPlotLegend((clientsTerminated && !stats[i].m_clientsTerminated.empty()) ||
                                 (!clientsTerminated && !stats[i].m_connectionsLost.empty()));

        // add the sub-graphs to the main-graph
        mainGraph->registerSecondaryPlotter(subGraph);
    }

    // add main graph containing sub-graphs to the plotter
    gpd.registerPlotter(mainGraph);
}

void plotStats(const GNetworkedConsumerStabilityConfig &config,
               std::vector<StabilityStatistic> stats) {
    // shrink all measurements to the requested resolution
    if (graphResolution < config.getDuration().totalMinutes()) {
        std::for_each(stats.begin(), stats.end(), [](StabilityStatistic &stat) {
            stat.shrink(graphResolution);
        });
    }

    // one graph for terminations and one graph for connection issues
    const std::uint32_t nRows{2};

    // Create plotter
    Gem::Common::GPlotDesigner gpd("Networked Consumer Stability Test", 1, nRows);

    // plot client shutdown
    addGraph(config, stats, gpd, true);

    // plot connection losses
    addGraph(config, stats, gpd, false);

    gpd.setCanvasDimensions(1920, 1163 * nRows);

    // write to file in ROOT format
    gpd.writeToFile(std::filesystem::path(config.getResultFileName()));
}

std::string getHeader(const GNetworkedConsumerStabilityConfig &config) {
    std::stringstream sout{};
    sout << "-----------------------------------------" << std::endl
         << "Starting stability test for the following configuration:" << std::endl
         << config;

    return sout.str();
}

void statsToFile(const std::vector<StabilityStatistic> &stats) {
    std::ofstream ofs(backupFileName);
    boost::archive::text_oarchive oa(ofs);
    oa & stats;
}

/**
 * loads a vector of execution times from a specified file
 */
void statsFromFile(std::vector<StabilityStatistic> &stats, const std::string &path) {
    std::ifstream ifs(path);
    boost::archive::text_iarchive ia(ifs);
    ia & stats;
}

int main(int argc, char **argv) {
    GNetworkedConsumerStabilityConfig config{argc, argv};

    // statistics to create
    std::vector<StabilityStatistic> stats{};

    // Sort all collections. Later on we can therefore assume that e.g. the competitors are alphabetically sorted
    config.sortAll();

    if (!config.getOnlyGenerateGraphs()) { // run the tests

        std::cout << getHeader(config) << std::endl;

        for (const Competitor &c: config.getCompetitors()) {
            stats.push_back(runTest(config, c));

            std::cout << "Waiting for the configured amount of " << config.getInterMeasurementDelaySecs()
                      << " seconds before testing the next configuration..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(config.getInterMeasurementDelaySecs()));
        }

        // write stats to file in case we want to restore from this point in a later run
        statsToFile(stats);
    } else { // load results of previous test run from file
        statsFromFile(stats, backupFileName);
    }

    std::cout << "Generating the plots ..." << std::endl;
    plotStats(config, stats);
    std::cout << "Stability test finished." << std::endl;

    return 0;
}