/**
 * @file GBrokerSelfCommunication.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "courtier/GBrokerT.hpp"
#include "courtier/GBrokerConnectorT.hpp"
#include "courtier/GAsioTCPConsumerT.hpp"
#include "courtier/GAsioTCPClientT.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#include "common/GExceptions.hpp"
#include "common/GThreadGroup.hpp"

#include "GRandomNumberContainer.hpp"
#include "GArgumentParser.hpp"

std::size_t producer_counter;
boost::mutex producer_counter_mutex;

using namespace Gem::Courtier;
using namespace Gem::Courtier::Tests;

/********************************************************************************/

void producer(
	const boost::uint32_t& nProductionCycles
	, const boost::uint32_t& nContainerObjects
	, const std::size_t& nContainerEntries
	, const bool& completeReturnRequired
	, const std::size_t& maxResubmissions
) {
	std::size_t id;

	{ // Assign a counter to this producer
		boost::mutex::scoped_lock lk(producer_counter_mutex);
		id = producer_counter++;
	}

	// Holds the broker object
	Gem::Courtier::GBrokerConnectorT<GRandomNumberContainer> brokerConnector;

	// Will hold the GRandomNumberContainer objects
	std::vector<boost::shared_ptr<GRandomNumberContainer> > data(nContainerObjects);

	// Start the loop
	boost::uint32_t cycleCounter = 0;
	while(cycleCounter++ < nProductionCycles) {
		// Fill a std::vector<> with GRandomNumberContainer objects
		for(std::size_t i=0; i<nContainerObjects; i++) {
			data[i] = boost::shared_ptr<GRandomNumberContainer>(new GRandomNumberContainer(nContainerEntries));
		}

		// Submit the container to the broker
		if(completeReturnRequired) {
			bool complete = brokerConnector.workOnSubmissionOnly(
					data
					, 0
					, data.size()
					, maxResubmissions
			);

			if(!complete) {
				raiseException(
						"In producer(): Error!" << std::endl
						<< "No complete set of items received after " << maxResubmissions << " resubmissions" << std::endl
				);
			}

			// Check that the container is at its default size
			if(data.size() != nContainerObjects) {
				raiseException(
						"In producer(): Error!" << std::endl
						<< "No complete set of items received despite request for a complete return" << std::endl
				);
			}
		} else {
			brokerConnector.workOn(
					data
					, 0
					, data.size()
					, ACCEPTOLDERITEMS
			);
		}
	}

	std::cout << "Producer " << id << " has finished producing" << std::endl;
}

/********************************************************************************/

int main(int argc, char **argv) {
	std::string configFile;
	bool serverMode;
	std::string ip;
	unsigned short port;
	Gem::Common::serializationMode serMode;
	boost::uint32_t nProducers;
  	boost::uint32_t nProductionCycles;
  	bool completeReturnRequired;
  	std::size_t maxResubmissions;
	boost::uint32_t nContainerObjects;
	std::size_t nContainerEntries;
	boost::uint32_t nWorkers;
	GBSCModes executionMode;

	// Initialize the global producer counter
	producer_counter = 0;

	// Some thread groups needed for producers and workers
	Gem::Common::GThreadGroup producer_gtg;
	Gem::Common::GThreadGroup worker_gtg;

	//--------------------------------------------------------------------------------
	// Find out about our configuration options
	if(!parseCommandLine(
			argc, argv
			, configFile
			, executionMode
			, serverMode
			, ip
			, port
			, serMode
			, completeReturnRequired
		) || !parseConfigFile(
			configFile
			, nProducers
			, nProductionCycles
			, nContainerObjects
			, nContainerEntries
			, maxResubmissions
			, nWorkers
		)
	){ exit(1); }

	//--------------------------------------------------------------------------------
	// Initialize the broker
	GBROKER(GRandomNumberContainer)->init();

	//--------------------------------------------------------------------------------
	// If we are in (networked) client mode, start the client code
	if((executionMode==Gem::Courtier::Tests::NETWORKING || executionMode==Gem::Courtier::Tests::THREAEDANDNETWORKING) && !serverMode) {
		boost::shared_ptr<GAsioTCPClientT<GRandomNumberContainer> > p(new GAsioTCPClientT<GRandomNumberContainer>(ip, boost::lexical_cast<std::string>(port)));

		p->setMaxStalls(0); // An infinite number of stalled data retrievals
		p->setMaxConnectionAttempts(100); // Up to 100 failed connection attempts

		// Start the actual processing loop
		p->run();

		return 0;
	}

	//--------------------------------------------------------------------------------
	// Create the required number of producer threads
	producer_gtg.create_threads(
		boost::bind(
			producer
			, nProductionCycles
			, nContainerObjects
			, nContainerEntries
			, completeReturnRequired
			, maxResubmissions
		)
	    , nProducers
	);

	//--------------------------------------------------------------------------------
	// Add the desired consumers to the broker
	switch(executionMode) {
	case Gem::Courtier::Tests::SERIAL:
		{
			// Create a serial consumer and enrol it with the broker
			boost::shared_ptr<GSerialConsumerT<GRandomNumberContainer> > gatc(new GSerialConsumerT<GRandomNumberContainer>());
			GBROKER(GRandomNumberContainer)->enrol(gatc);
		}
		break;

	case Gem::Courtier::Tests::INTERNALNETWORKING:
		{
			// Start the workers
			boost::shared_ptr<GAsioTCPClientT<GRandomNumberContainer> > p(new GAsioTCPClientT<GRandomNumberContainer>("localhost", "10000"));
			worker_gtg.create_threads(
				boost::bind(&GAsioTCPClientT<GRandomNumberContainer>::run,p)
				, nWorkers
			);

			// Create a network consumer and enrol it with the broker
			boost::shared_ptr<GAsioTCPConsumerT<GRandomNumberContainer> > gatc(new GAsioTCPConsumerT<GRandomNumberContainer>((unsigned short)10000));
			GBROKER(GRandomNumberContainer)->enrol(gatc);
		}
		break;

	case Gem::Courtier::Tests::NETWORKING:
		{
			// Create a network consumer and enrol it with the broker
			boost::shared_ptr<GAsioTCPConsumerT<GRandomNumberContainer> > gatc(new GAsioTCPConsumerT<GRandomNumberContainer>(port));
			GBROKER(GRandomNumberContainer)->enrol(gatc);
		}
		break;

	case Gem::Courtier::Tests::MULTITHREADING:
		{
			// Create a consumer and make it known to the global broker
			boost::shared_ptr< GBoostThreadConsumerT<GRandomNumberContainer> > gbtc(new GBoostThreadConsumerT<GRandomNumberContainer>());
			GBROKER(GRandomNumberContainer)->enrol(gbtc);
		}
		break;

	case Gem::Courtier::Tests::THREADANDINTERNALNETWORKING:
		{
			// Start the workers
			boost::shared_ptr<GAsioTCPClientT<GRandomNumberContainer> > p(new GAsioTCPClientT<GRandomNumberContainer>("localhost", "10000"));
			worker_gtg.create_threads(
				boost::bind(&GAsioTCPClientT<GRandomNumberContainer>::run,p)
				, nWorkers
			);

			boost::shared_ptr<GAsioTCPConsumerT<GRandomNumberContainer> > gatc(new GAsioTCPConsumerT<GRandomNumberContainer>((unsigned short)10000));
			boost::shared_ptr< GBoostThreadConsumerT<GRandomNumberContainer> > gbtc(new GBoostThreadConsumerT<GRandomNumberContainer>());

			GBROKER(GRandomNumberContainer)->enrol(gatc);
			GBROKER(GRandomNumberContainer)->enrol(gbtc);
		}
		break;

	case Gem::Courtier::Tests::THREAEDANDNETWORKING:
		{
			boost::shared_ptr<GAsioTCPConsumerT<GRandomNumberContainer> > gatc(new GAsioTCPConsumerT<GRandomNumberContainer>(port));
			boost::shared_ptr< GBoostThreadConsumerT<GRandomNumberContainer> > gbtc(new GBoostThreadConsumerT<GRandomNumberContainer>());

			GBROKER(GRandomNumberContainer)->enrol(gatc);
			GBROKER(GRandomNumberContainer)->enrol(gbtc);
		}
		break;

	default:
		{
			raiseException(
					"Error: Invalid execution mode requested: " << executionMode << std::endl
			);
		}
		break;
	};

	//--------------------------------------------------------------------------------
	// Wait for all threads to finish
	producer_gtg.join_all();

	if(executionMode == Gem::Courtier::Tests::INTERNALNETWORKING || executionMode == Gem::Courtier::Tests::THREADANDINTERNALNETWORKING)
	worker_gtg.join_all();

	std::cout << "All threads have joined" << std::endl;

	// Terminate the broker
	GBROKER(GRandomNumberContainer)->finalize();
}
