/**
 * @file GNetworkedConsumerStabilityConfig.hpp
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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

// Boost header files go here

// Geneva header files go here

#include "common/GParserBuilder.hpp"

namespace Gem::Tests {

    /**
     * Stores the duration to run each competitor
     */
    struct Duration {
        std::uint32_t hours;
        std::uint32_t minutes;

        [[nodiscard]] std::uint32_t totalMinutes() const {
            return hours * 60 + minutes;
        }
    };

    std::ostream &operator<<(std::ostream &os, const Duration &d) {
        os << std::setfill('0') << std::setw(2) << d.hours
           << ":" << std::setfill('0') << std::setw(2) << d.minutes;

        return os;
    }

    std::istream &operator>>(std::istream &is, Duration &d) {

        is >> d.hours;
        is.ignore(1); // ignore ':' separator
        is >> d.minutes;

        return is;
    }

    /**
     * stores information about one of the competing configurations in the test
     */
    struct Competitor {
        friend class boost::serialization::access;

        template<typename Archive>
        void serialize(Archive &ar, const unsigned int) {
            using boost::serialization::make_nvp;
            ar
            & BOOST_SERIALIZATION_NVP(name)
            & BOOST_SERIALIZATION_NVP(shortName)
            & BOOST_SERIALIZATION_NVP(arguments)
            & BOOST_SERIALIZATION_NVP(connectionIssuesSubstring)
            & BOOST_SERIALIZATION_NVP(terminationSubString);
        }

        /**
         * name displayed to the user in the graphs
         */
        std::string name;
        /**
         * a short specifier e.g. to prefix/suffix result-files
         */
        std::string shortName;
        /**
         * arguments supplied to the test executable
         */
        std::string arguments;
        /**
         * A one-line substring in the output of the client which indicates an issue in connecting with the server
         */
        std::string connectionIssuesSubstring;
        /**
         * A one-line substring in the output of the client which indicates that this client has terminated
         */
        std::string terminationSubString;

        /**
         * create an order depending on the shortName (which is also used as ID)
         */
        bool operator<(const Competitor &other) const {
            return this->shortName < other.shortName;
        }
    };

    std::ostream &operator<<(std::ostream &os, const Competitor &c) {
        os << "'" << c.name << "', "
           << "'" << c.shortName << "', "
           << "'" << c.arguments << "', "
           << "'" << c.connectionIssuesSubstring << "', "
           << "'" << c.terminationSubString << "'";

        return os;
    }

    std::istream &operator>>(std::istream &is, Competitor &c) {
        std::getline(is, c.name, '\''); // skip until first tic
        std::getline(is, c.name, '\''); // overwrite until second tic

        std::getline(is, c.shortName, '\''); // skip until first tic
        std::getline(is, c.shortName, '\''); // overwrite until second tic

        std::getline(is, c.arguments, '\''); // skip until first tic
        std::getline(is, c.arguments, '\''); // overwrite until second tic

        std::getline(is, c.connectionIssuesSubstring, '\''); // skip until first tic
        std::getline(is, c.connectionIssuesSubstring, '\''); // overwrite until second tic

        std::getline(is, c.terminationSubString, '\''); // skip until first tic
        std::getline(is, c.terminationSubString, '\''); // overwrite until second tic

        return is;
    }

/*********************************************************************************/
/**
 * A class that parses configuration options for the GNetworkedConsumerStabilityConfig test
 */
    class GNetworkedConsumerStabilityConfig {
    public:

        /**
         * Deleted Default Constructor
         */
        GNetworkedConsumerStabilityConfig() = delete; ///< Default constructor: Intentionally private and undefined


        /*****************************************************************************/
        /**
         * Constructor
         *
         * @param configFile The name of a configuration file
         * @param resultFile The name of a file to which results should be written
         */
        explicit GNetworkedConsumerStabilityConfig(int argc, char **argv) {
            registerCLOptions();
            registerFileOptions();

            m_cLParser.parseCommandLine(argc, argv);
            m_fileParser.parseConfigFile(m_configFile);
        }

        /**
         * sorts all collections that are stored in this class using the '<'-operator of the elements
         */
        GNetworkedConsumerStabilityConfig &sortAll() {
            // sort competitors alphabetically by their short name (ID)
            std::sort(m_competitors.begin(), m_competitors.end());

            return *this;
        }

        [[nodiscard]] const Duration &getDuration() const {
            return m_duration;
        }

        /**
         * Retrieval of the name of the result file
         *
         * @return The name of the result file
         */
        [[nodiscard]] const std::string &getResultFileName() const {
            return m_resultFile;
        }

        /*****************************************************************************/
        /**
         * Retrieval of the vector holding the numbers of clients
         *
         * @return A vector holding the numbers of clients
         */
        [[nodiscard]] const std::uint32_t &getNClients() const {
            return m_nClients;
        }

        /**
         * returns all competitors for this test
         */
        [[nodiscard]] const std::vector<Competitor> &getCompetitors() const {
            return m_competitors;
        }

        /**
         * returns the name of the config file for this class
         */
        [[nodiscard]] std::string getMConfigFileName() const {
            return m_configFile;
        }

        /**
         * returns the name of the test executable to run
         */
        [[nodiscard]] std::string getTestExecutableName() const {
            return m_testExecutable;
        }

        /**
         * returns the location where the mpirun executable is expected on this machine
         */
        [[nodiscard]] std::string getMpirunLocation() const {
            return m_mpirunLocation;
        }

        /**
         * Retrieves the delay between one call to the test executable and the next call.
         * This delay might be helpful to give the OS time to free up resources.
         */
        [[nodiscard]] std::uint32_t getInterMeasurementDelaySecs() const {
            return m_interMeasurementDelaySecs;
        }

        /**
         * returns a flag that is true if the benchmark should not be run, but only the graphs should be generated from
         * already existing result files.
         */
        [[nodiscard]] std::uint32_t getOnlyGenerateGraphs() const {
            return m_onlyGenerateGraphs;
        }

    private:

        void registerFileOptions() {
            using namespace Gem::Common;

            m_fileParser.registerFileParameter(
                    "duration",
                    m_duration,
                    m_duration,
                    "Duration to run each competitor for"
            );

            m_fileParser.registerFileParameter(
                    "nClients",
                    m_nClients,
                    m_nClients,
                    VAR_IS_ESSENTIAL,
                    "The number of clients to run this test with"
            );

            m_fileParser.registerFileParameter(
                    "competitors",
                    m_competitors,
                    m_competitors,
                    VAR_IS_ESSENTIAL,
                    "A list of configurations to run against each other in this test. Each item consists of"
                    "two strings. The first string is the name displayed in the graphs. The second one is the string of arguments"
                    "to pass to the executable. The test executable will take care of correctly starting clients."
            );

            m_fileParser.registerFileParameter(
                    "resultFile", m_resultFile, m_resultFile, VAR_IS_ESSENTIAL,
                    "The name of a file to which results of the test should be written"
            );

            m_fileParser.registerFileParameter(
                    "mpirunLocation", m_mpirunLocation, m_mpirunLocation, VAR_IS_ESSENTIAL,
                    "The location of the mpirun executable to use."
            );

            m_fileParser.registerFileParameter(
                    "interMeasurementDelaySecs", m_interMeasurementDelaySecs, m_interMeasurementDelaySecs,
                    VAR_IS_ESSENTIAL,
                    "Delay in between starting test executables, which might be helpful to give the OS time to free up resources."
            );
        }

        void registerCLOptions() {
            m_cLParser.registerCLParameter(
                    "configFile",
                    m_configFile,
                    m_configFile,
                    "The location of the config file for this test."
            );

            m_cLParser.registerCLParameter(
                    "testExecutable",
                    m_testExecutable,
                    m_testExecutable,
                    "The location of the executable that is started for each competitor."
            );

            m_cLParser.registerCLParameter(
                    "onlyGenerateGraphs",
                    m_onlyGenerateGraphs,
                    m_onlyGenerateGraphs,
                    "Flag that defines whether to only build the graphs without running the benchmark."
            );
        }

        /*
         * Parses the file configuration
         */
        Gem::Common::GParserBuilder m_fileParser;
        /**
         * Parses the command line configuration
         */
        Gem::Common::GParserBuilder m_cLParser;


        /*****************************************************************
         * Options to parse from config file
         * ***************************************************************
         */

        /**
         * The amount of clients to test with
         */
        std::uint32_t m_nClients{250};

        /**
         * Duration to run each competitor for
         */
        Duration m_duration{1, 0};

        /**
         * The different configurations to test in this test. The default is to test all networked consumers.
         * The Benchmark itself will take care of starting the clients or in case of MPI using mpirun
         */
        std::vector<Competitor> m_competitors{
                // attributes: name, arguments
                {"Boost.Asio",  "asio",  "--consumer asio --asio_port 10000 --asio_nProcessingThreads=1",  "We will try to reconnect", "GAsioConsumerClientT<processable_type>::run_(): Client has terminated"},
                {"Boost.Beast", "beast", "--consumer beast --beast_port 10001 --beast_nListenerThreads=1", "",                         "GWebsocketClientT<processable_type>::run_(): Client session has terminated"},
                {"MPI",         "mpi",   "--consumer mpi --mpi_master_nIOThreads=1",                       "",                         "GMPIConsumerWorkerNodeT<processable_type>::run(): Worker has terminated"}
        };


        /**
         * The name of a file to which results should be written
         */
        std::string m_resultFile = "GNetworkedConsumerStability.C";

        /**
         * location of the mpirun executable. If mpirun is in PATH you do not need to adjust this.
         */
        std::string m_mpirunLocation = "mpirun";

        /**
         * Delay in between starting test executables, which might be helpful to give the OS time to free up resources.
         */
        std::uint32_t m_interMeasurementDelaySecs = 60;

        /*****************************************************************
         * Options to parse from command line
         * ***************************************************************
         */

        /**
         * The location of the config file for this class
         */
        std::string m_configFile = "./config/GNetworkedConsumerStabilityConfig.json";

        /**
         * The location of the executable called for testing
         */
        std::string m_testExecutable = "./GNetworkedConsumerStabilitySubProgram/GNetworkedConsumerStabilitySubProgram";

        /**
         * Flag that defines whether to only build the graphs without running the benchmark. This can be useful if
         * the benchmark files already exists from a previous run but you would like to rebuild the graphs
         */
        std::uint32_t m_onlyGenerateGraphs{0};
    };

    std::ostream &operator<<(std::ostream &os, const GNetworkedConsumerStabilityConfig &config) {
        os << "Competitors:" << std::endl;

        for (const auto &c: config.getCompetitors()) {
            os << c.name << ": " << c.arguments << std::endl;
        }

        os << "Duration per configuration: " << config.getDuration() << " [hh:mm]" << std::endl;

        os << "Clients: " << config.getNClients() << std::endl;

        return os;
    }
}

