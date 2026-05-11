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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#ifndef GENEVA_LIBRARY_COLLECTION_GMPISUBCLIENTOPTIMIZER_H
#define GENEVA_LIBRARY_COLLECTION_GMPISUBCLIENTOPTIMIZER_H

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <optional>

// Boost header files go here

// Geneva headers go here
#include "geneva/GMPISubClientIndividual.hpp"
#include "geneva/Go2.hpp"

namespace Gem::Geneva {

/**
     * This is a class which supports all functionality that is given by `Go2` and additionally adds
     * the functionality of MPI-sub-clients. Each sub-client group can use their own MPI communicator in order
     * to together solve a fitnessCalculation.
     * This class only makes sense when the consumer is set to GMPIConsumerT and therefore is not allowed to use with other
     * consumers.
     */
class GMPISubClientOptimizer // NOLINT(cppcoreguidelines-special-member-functions)
  : public Go2 {
public:
    /**
         * A constructor that first parses the command line for relevant parameters and allows to specify a default config file name
         * @param argc number of command line arguments
         * @param argv vector of command line arguments
         * @param configFilePath The name and location of a configuration file
         * @param userDescriptions A vector of additional command line options (cmp. boost::program_options)
         * @param baseCommunicator MPI communicator that all processes which instantiate GMPISubClientOptimizer call.
         *  In the most frequent and less complicated case the default value of MPI_COMM_WORLD will be correct.
         */
    GMPISubClientOptimizer(
        int argc,
        char **argv,
        std::string const &configFilePath,
        boost::program_options::options_description const &userDescriptions =
            boost::program_options::options_description(),
        MPI_Comm baseCommunicator = MPI_COMM_WORLD
    );
    /** @brief Deleted copy constructor */
    GMPISubClientOptimizer(GMPISubClientOptimizer const &) = delete;

    /**
         * Registers a function to be called by sub-clients.
         *
         * The function takes an MPI_Comm communicator as an argument.
         * This is the communicator that is used by this process and all processes in the same sub-group.
         *
         * @param callback The function called by sub-clients
         * @return
         */
    GMPISubClientOptimizer &
    registerSubClientJob(std::function<int(MPI_Comm)> callback);

    [[nodiscard]] bool isSubClient() const {
        return isSubClient_;
    }

protected:
    /**
         * Triggers execution of the client job
         * @return An integer type return value for the main function indicating execution status.
         */
    int clientRun_() override;

    /**
         * Adds local configuration options to a GParserBuilder object
         *
         * @param gpb The GParserBuilder object to which configuration options should be added
         */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;

private:
    MPI_Request startAsyncBarrier() const;
    /**
         * MPI communicator used for communication between the geneva GMPIConsumerMasterNodeT and GMPIConsumerWorkerNodeT.
         */
    MPI_Comm genevaComm_{};
    /**
         * MPI communicator used for communication between sub-clients inside of their specific sub-group.
         */
    MPI_Comm subClientComm_{};
    /**
         * MPI communicator which has the same scope as subClientComm_ but is used for retrieving status information about of the current group.
         */
    MPI_Comm subClientStatusComm_{};
    /**
         * Total number of the MPI nodes which will instantiate this class
         */
    int baseCommSize_{};
    /**
         * Rank in the base communicator i.e. in the outer most communicator
         */
    int baseCommRank_{};
    /**
          * The number of sub-clients per geneva client. This means each geneva client is part of a sub-group consisting of
          * nSubClients_ processes.
          */
    std::uint16_t subClientGroupSize_{4};
    /**
         * Flag which is true if the current process is a sub-client
         */
    bool isSubClient_{};
    /**
         * Callback function which is executed by sub-clients when clientRun() is called
         */
    std::function<int(MPI_Comm)> subClientJob_{[](MPI_Comm comm) -> int {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "GMPISubClientOptimizer::subClientJob_(MPI_Comm comm): Error!" << std::endl
            << "The sub-client job has not been set. Set it using the `GMPISubClientOptimizer "
               "&GMPISubClientOptimizer::registerSubClientJob(std::function<int(MPI_Comm)> "
               "&callback)` method."
            << std::endl
        );
    }};

    /**
        * The color argument when creating the Geneva communicator.
        * The value is arbitrary but must be different from all sub-client colors.
        */
    const int M_MPI_GENEVA_COLOR{0};
};
} /* namespace Gem::Geneva */

#endif //GENEVA_LIBRARY_COLLECTION_GMPISUBCLIENTOPTIMIZER_H
