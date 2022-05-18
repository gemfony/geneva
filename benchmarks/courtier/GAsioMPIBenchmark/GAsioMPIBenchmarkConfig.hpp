/**
 * @file GOptimizationBenchmarkConfig.hpp
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

/*********************************************************************************/
/**
 * A class that parses configuration options for the GAsioMPIBenchmark test
 */
    class GAsioMPIBenchmarkConfig {
    public:

        /**
         * Deleted Default Constructor
         */
        GAsioMPIBenchmarkConfig() = delete; ///< Default constructor: Intentionally private and undefined


        /*****************************************************************************/
        /**
         * Constructor
         *
         * @param configFile The name of a configuration file
         * @param resultFile The name of a file to which results should be written
         */
        explicit GAsioMPIBenchmarkConfig(int argc, char **argv) {
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

        // TODO: doxygen comments for getters

        [[nodiscard]] const std::string &getMIntermediateResultFileName() const {
            return m_intermediateResultFile;
        }

        [[nodiscard]] const std::string &getMConfigFileName() const {
            return m_configFile;
        }

        [[nodiscard]] const std::string &getMBenchmarkExecutableName() const {
            return m_benchmarkExecutable;
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
                    "resultFile", m_resultFile, m_resultFile, VAR_IS_ESSENTIAL,
                    "The name of a file to which results of the benchmark should be written"
            );

            m_fileParser.registerFileParameter(
                    "intermediateResultFile", m_intermediateResultFile, m_intermediateResultFile, VAR_IS_ESSENTIAL,
                    "The name of a file where the results of the runs of the subprocesses are written to. "
                    "This should be identical with the result file name configured in the subprogram directory"
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
                4,
                10,
                50,
                100,
                200,
                300,
                400,
                500,
                750,
                1000
        };

        /**
         * The name of a file to which results should be written
         */
        std::string m_resultFile = "GAsioMPIBenchmarkResult.C";

        /**
         * The name of the intermediate result file produced each run.
         * This should be the name of the result file in the config file for the GDelayIndividualFactory
         */
        std::string m_intermediateResultFile = "executionTimes.C";

        /*****************************************************************
         * Options to parse from command line
         * ***************************************************************
         */

        /**
         * The location of the config file for this class
         */
        std::string m_configFile = "./config/GAsioMPIBenchmarkConfig.json";

        /**
         * The location of the executable called for benchmarking
         */
        std::string m_benchmarkExecutable = "./GAsioMPIBenchmarkSubProgram/GAsioMPIBenchmarkSubProgram";
    };

}

