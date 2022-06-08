/**
 * @file GNetworkedConsumerBenchmarkConfig.hpp
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
     * stores information about one of the competing configurations in the benchmark
     */
    struct Competitor {
        /**
         * name displayed to the user in the graphs
         */
        std::string name;
        /**
         * arguments supplied to the benchmark executable
         */
        std::string arguments;
    };

/*********************************************************************************/
/**
 * A class that parses configuration options for the GAsioMPIBenchmark test
 */
    class GNetworkedConsumerBenchmarkConfig {
    public:

        /**
         * Deleted Default Constructor
         */
        GNetworkedConsumerBenchmarkConfig() = delete; ///< Default constructor: Intentionally private and undefined


        /*****************************************************************************/
        /**
         * Constructor
         *
         * @param configFile The name of a configuration file
         * @param resultFile The name of a file to which results should be written
         */
        explicit GNetworkedConsumerBenchmarkConfig(int argc, char **argv) {
            registerCLOptions();
            registerFileOptions();

            m_cLParser.parseCommandLine(argc, argv);
            m_fileParser.parseConfigFile(m_configFile);
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
        [[nodiscard]] const std::vector<std::uint32_t> &getNClients() const {
            return m_nClients;
        }

        /**
         * returns all competitors for this benchmark
         */
        [[nodiscard]] std::vector<Competitor> getCompetitors() const {
            std::vector<Competitor> competitors{};
            // transform the vector of vectors into a more user-friendly vector of structs
            for (const auto &c: m_competitors) {
                competitors.push_back(Competitor{c[0], c[1]});
            }

            return competitors;
        }

        // TODO: doxygen comments for getters

        [[nodiscard]] std::string getMIntermediateResultFileName() const {
            return m_intermediateResultFile;
        }

        [[nodiscard]] std::string getMConfigFileName() const {
            return m_configFile;
        }

        [[nodiscard]] std::string getMBenchmarkExecutableName() const {
            return m_benchmarkExecutable;
        }

        [[nodiscard]] std::uint32_t getOnlyGenerateGraphs() const {
            return m_onlyGenerateGraphs;
        }

        [[nodiscard]] std::string getMpirunLocation() const {
            return m_mpirunLocation;
        }

    private:

        void registerFileOptions() {
            using namespace Gem::Common;
            m_fileParser.registerFileParameter(
                    "nClients",
                    m_nClients,
                    m_nClients,
                    VAR_IS_ESSENTIAL,
                    "A list of numbers of clients to test with. Each value will be used for a single test. "
                    "All those tests are run after another."
            );

            m_fileParser.registerFileParameter(
                    "competitors",
                    m_competitors,
                    m_competitors,
                    VAR_IS_ESSENTIAL,
                    "A list of configurations to run against each other in this benchmark. Each item consists of"
                    "two strings. The first string is the name displayed in the graphs. The second one is the string of arguments"
                    "to pass to the executable. The benchmark executable will take care of correctly starting clients."
            );

            m_fileParser.registerFileParameter(
                    "resultFile", m_resultFile, m_resultFile, VAR_IS_ESSENTIAL,
                    "The name of a file to which results of the benchmark should be written"
            );

            m_fileParser.registerFileParameter(
                    "intermediateResultFile", m_intermediateResultFile, m_intermediateResultFile, VAR_IS_ESSENTIAL,
                    "The name of a file where the results of the runs of the subprocesses are written to. "
                    "This should be identical with the result file name configured in the subprogram directory"
            );

            m_fileParser.registerFileParameter(
                    "mpirunLocation", m_mpirunLocation, m_mpirunLocation, VAR_IS_ESSENTIAL,
                    "The location of the mpirun executable to use."
            );
        }

        void registerCLOptions() {
            m_cLParser.registerCLParameter(
                    "configFile",
                    m_configFile,
                    m_configFile,
                    "The location of the config file for this benchmark."
            );

            m_cLParser.registerCLParameter(
                    "benchmarkExecutable",
                    m_benchmarkExecutable,
                    m_benchmarkExecutable,
                    "The location of the executable that is started."
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
         * A list of numbers of clients to test with.
         */
        std::vector<std::uint32_t> m_nClients{
                1,
                5,
                10,
                25,
                50,
                100,
                250
        };

        /**
         * The different configurations to test in this benchmark. The default is to test all networked consumers.
         * The Benchmark itself will take care of starting the clients or in case of MPI using mpirun
         */
        std::vector<std::vector<std::string>> m_competitors{
                // attributes: name, arguments
                {"Asio",  ""},
                {"beast", "--consumer beast"},
                {"MPI",   "--consumer mpi"}
        };


        /**
         * The name of a file to which results should be written
         */
        std::string m_resultFile = "GNetworkedConsumerBenchmark.C";

        /**
         * The name of the intermediate result file produced each run.
         * This should be the name of the result file in the config file for the GDelayIndividualFactory
         */
        std::string m_intermediateResultFile = "executionTimes.C";

        /**
         * location of the mpirun executable. If mpirun is in PATH you do not need to adjust this.
         */
        std::string m_mpirunLocation = "mpirun";


        /*****************************************************************
         * Options to parse from command line
         * ***************************************************************
         */

        /**
         * The location of the config file for this class
         */
        std::string m_configFile = "./config/GNetworkedConsumerBenchmarkConfig.json";

        /**
         * The location of the executable called for benchmarking
         */
        std::string m_benchmarkExecutable = "./GNetworkedConsumerBenchmark/GNetworkedConsumerBenchmarkSubProgram";

        /**
         * Flag that defines whether to only build the graphs without running the benchmark. This can be useful if
         * the benchmark files already exists from a previous run but you would like to rebuild the graphs
         */
        std::uint32_t m_onlyGenerateGraphs{0};
    };

}

