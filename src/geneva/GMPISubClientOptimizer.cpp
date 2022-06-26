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

#include "geneva/GMPISubClientOptimizer.hpp"

// TODO: change std::cout to glogger

namespace Gem::Geneva {
    GMPISubClientOptimizer::GMPISubClientOptimizer(int argc,
                                                   char **argv,
                                                   std::string const &configFilePath,
                                                   boost::program_options::options_description const &userDescriptions,
                                                   MPI_Comm baseCommunicator)
            : Go2{argc, argv, configFilePath, userDescriptions} {
        if (Go2::getConsumerName() != "mpi") { // only allow using MPI
            throw gemfony_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                            << "GMPISubClientOptimizer::optimize_(std::uint32_t offset): Error!" << std::endl
                            << "GMPISubClientOptimizer may only be used with the GMPIConsumerT, but the consumer is `"
                            << Go2::getConsumerName() << "`" << std::endl
                            << "Set the consumer to GMPIConsumerT by using the command line argument `--consumer mpi`"
                            << std::endl
            );
        }

        // If the base communicator is already a sub communicator, this means MPI must already have been initialized by the user
        if (baseCommunicator == MPI_COMM_WORLD) {
            // initialize MPI in the way this is required by GMPIConsumerT
            Gem::Courtier::GMPIConsumerT<GParameterSet>::initializeMPI();
        }

        // initialize position in MPI world e.g. in the outermost communicator
        MPI_Comm_rank(baseCommunicator, &m_baseCommRank);
        MPI_Comm_size(baseCommunicator, &m_baseCommSize);

        // All processes but the geneva server and the geneva clients are sub-clients.
        // As an example: In case of 17 processes with one server, 4 clients and 4 sub-clients the ranks
        // [0, 4, 8, 12, 16] are server and geneva clients. All other processes will be sub-clients.
        m_isSubClient = m_baseCommRank % (m_subClientGroupSize) != 0;
        const int subCommColor = (m_baseCommRank - 1) / (m_subClientGroupSize) + M_MPI_GENEVA_COLOR;

        std::cout << "baseRank=" << m_baseCommRank << std::endl;

        if (m_baseCommRank != 0) { // the server is in no sub-client group
            std::cout << "baseRank=" << m_baseCommRank << " is in sub-client-group " << subCommColor << std::endl;
        }


        // create new communicators based on the current position in the baseRank
        if (m_isSubClient) { // process is a sub-client

            // create the genevaCommunicator, because the call is collective.
            // But as this process is a sub-client we do not want to communicate inside that subgroup and therefore
            // pass MPI_UNDEFINED as color
            MPI_Comm_split(baseCommunicator, MPI_UNDEFINED, m_baseCommRank, &m_genevaComm);

            // putting nSubClients in one communicator by using down-rounding integer division
            MPI_Comm_split(baseCommunicator, subCommColor, m_baseCommRank, &m_subClientComm);

        } else { // process is a geneva client

            // Create a communicator to be used by GMPIConsumerT
            MPI_Comm_split(baseCommunicator, M_MPI_GENEVA_COLOR, m_baseCommRank, &m_genevaComm);

            if (m_baseCommRank == 0) { // process is the geneva server (master node)
                // The geneva server has rank 0 and does not need to be in any sub-client communicator.
                // It only communicates with the geneva clients and does not communicate inside of subgroups.
                MPI_Comm_split(baseCommunicator, MPI_UNDEFINED, m_baseCommRank, &m_subClientComm);
            } else {
                // all geneva clients are in separate communicators by using down-rounding integer division
                // This process (geneva client) receives rank 0 inside the subgroup. All other processes in this subgroup
                // will definitely pass numbers greater 0 as key and therefore get the ranks 1 - (m_subClientGroupSize - 1)
                MPI_Comm_split(baseCommunicator, subCommColor, 0, &m_subClientComm);
            }
        }

        // Notify the GMPIConsumerT to use this inter-communicator
        Gem::Courtier::GMPIConsumerT<GParameterSet>::setMPICommunicator(m_genevaComm);

        // Notify the individual to use this inter-communicator
        GMPISubClientIndividual::setCommunicator(m_subClientComm);
    }

    void GMPISubClientOptimizer::addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) {
        // let parent class `Go2` add its options first
        Go2::addConfigurationOptions_(gpb);

        // add additional options specific to this class

        gpb.registerFileParameter<std::uint16_t>(
                "subClientGroupSize",
                m_subClientGroupSize,
                m_subClientGroupSize)
                << "The amount of processes in each sub-group. Each sub group works together on one individual.";
    }

    GMPISubClientOptimizer &GMPISubClientOptimizer::registerSubClientJob(std::function<int(MPI_Comm)> callback) {
        m_subClientJob = std::move(callback);

        // return reference to self for chaining calls
        return *this;
    }

    int GMPISubClientOptimizer::clientRun_() {
        if (m_isSubClient) {
            return m_subClientJob(m_subClientComm);
        } else {
            return Go2::clientRun_();
        }
    }

}
