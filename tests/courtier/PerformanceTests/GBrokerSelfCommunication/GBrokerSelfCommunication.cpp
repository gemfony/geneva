/**
 * @file GBrokerSelfCommunication.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

#include <vector>
#include <functional>

#include <boost/thread.hpp>

#include "courtier/GCourtierEnums.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GBrokerConnector2T.hpp"
#include "courtier/GAsioTCPConsumerT.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"
#include "courtier/GSerialConsumerT.hpp"
#include "common/GExceptions.hpp"
#include "common/GThreadGroup.hpp"

#include "../../Misc/GRandomNumberContainer.hpp"
#include "../../Misc/GSimpleContainer.hpp"
#include "GArgumentParser.hpp"

std::size_t producer_counter;
boost::mutex producer_counter_mutex;

using namespace Gem::Courtier;
using namespace Gem::Courtier::Tests;

// #define WORKLOAD GRandomNumberContainer
#define WORKLOAD GSimpleContainer

/********************************************************************************/
/**
 * Produces work items and submits them through the broker connector in different,
 * user-selectable modes, then retrieves them back.
 */
void connectorProducer(
	std::uint32_t nProductionCycles
	, std::uint32_t nContainerObjects
	, std::size_t nContainerEntries
	, submissionReturnMode srm
	, std::size_t maxResubmissions
) {
	std::size_t id;

	{ // Assign a counter to this producer
		boost::mutex::scoped_lock lk(producer_counter_mutex);
		id = producer_counter++;
	}

	// Holds the broker connector (i.e. the entity that connects us to the broker)
	Gem::Courtier::GBrokerConnector2T<WORKLOAD> brokerConnector(srm);
	brokerConnector.init(); // This will particularly set up the buffer port
	brokerConnector.setMaxResubmissions(maxResubmissions);

	// Will hold the data items
	std::vector<std::shared_ptr<WORKLOAD>> data, oldWorkItems;

	// Start the loop
	std::uint32_t cycleCounter = 0;
	std::uint32_t nSentItems = 0;
	std::uint32_t nReceivedItemsNew = 0;
	std::uint32_t nReceivedItemsOld = 0;
	while(cycleCounter++ < nProductionCycles) {
		// Clear the data vector
		data.clear();
		oldWorkItems.clear();

		// Fill a std::vector<> with WORKLOAD objects
		for(std::size_t i=0; i<nContainerObjects; i++) {
			data.push_back(std::shared_ptr<WORKLOAD>(new WORKLOAD(nContainerEntries)));
		}
		nSentItems += boost::numeric_cast<std::uint32_t>(data.size());

		bool complete = brokerConnector.workOn(
			data
			, std::tuple<std::size_t,std::size_t>(0, data.size())
			, oldWorkItems
			, true // Remove unprocessed items
		);

		nReceivedItemsNew += boost::numeric_cast<std::uint32_t>(data.size());
		nReceivedItemsOld += boost::numeric_cast<std::uint32_t>(oldWorkItems.size());

		std::cout << "Cycle " << cycleCounter << " completed in producer " << id << std::endl << std::flush;
	}

	brokerConnector.finalize(); // This will reset the buffer port

	std::cout
	<< "connectorProducer " << id << " has finished." << std::endl
	<< "Sent = " << nSentItems << std::endl
	<< "Received current = " << nReceivedItemsNew << std::endl
	<< "Received older = " << nReceivedItemsOld << std::endl
	<< "Total received = " << nReceivedItemsNew + nReceivedItemsOld << std::endl
	<< "Missing = " << nSentItems-(nReceivedItemsNew+nReceivedItemsOld) << std::endl << std::flush;
}

/********************************************************************************/
/**
 * Produces work items and submits them directly to the broker, then retrieves
 * them back. By bypassing the broker connector, we can detect differences
 * between both modes.
 */
void brokerProducer(
	std::uint32_t nProductionCycles
	, std::uint32_t nContainerObjects
	, std::size_t nContainerEntries
) {
	std::size_t id;
	typedef std::shared_ptr<Gem::Courtier::GBufferPortT<std::shared_ptr<WORKLOAD>> > GBufferPortT_ptr;

	{ // Assign a counter to this producer
		boost::mutex::scoped_lock lk(producer_counter_mutex);
		id = producer_counter++;
	}

	// Create a buffer port and register it with the broker
	GBufferPortT_ptr CurrentBufferPort_(new Gem::Courtier::GBufferPortT<std::shared_ptr<WORKLOAD>>());
	GBROKER(WORKLOAD)->enrol(CurrentBufferPort_);

	// Start the loop
	std::uint32_t cycleCounter = 0;
	while(cycleCounter++ < nProductionCycles) {
		// Submit the required number of items directly to the broker
		for(std::size_t i=0; i<nContainerObjects; i++) {
			CurrentBufferPort_->push_front_orig(std::shared_ptr<WORKLOAD>(new WORKLOAD(nContainerEntries)));
		}

		// Wait for all items to return
		std::shared_ptr<WORKLOAD> p;
		for(std::size_t i=0; i<nContainerObjects; i++) {
			CurrentBufferPort_->pop_back_processed(p);
			if(!p) {
				raiseException(
					"In brokerProducer: " << "got invalid item" << std::endl
				);
			}
		}

		std::cout << "Cycle " << cycleCounter << " completed in producer " << id << std::endl << std::flush;
	}

	// Get rid of the buffer port object
	CurrentBufferPort_.reset();

	std::cout << "brokerProducer " << id << " has finished producing" << std::endl << std::endl;
}

/********************************************************************************/

int main(int argc, char **argv) {
	std::string configFile;
	bool serverMode;
	std::string ip;
	unsigned short port;
	Gem::Common::serializationMode serMode;
	std::uint32_t nProducers;
	std::uint32_t nProductionCycles;
	submissionReturnMode srm;
	std::size_t maxResubmissions;
	std::uint32_t nContainerObjects;
	std::size_t nContainerEntries;
	std::uint32_t nWorkers;
	GBSCModes executionMode;
	bool useDirectBrokerConnection;
	std::vector<std::shared_ptr<GAsioTCPClientT<WORKLOAD>> > clients;

	// Initialize the global producer counter
	producer_counter = 0;

	// Some thread groups needed for producers and workers
	Gem::Common::GThreadGroup connectorProducer_gtg;
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
		, srm
		, useDirectBrokerConnection
	) || !parseConfigFile(
		configFile
		, nProducers
		, nProductionCycles
		, nContainerObjects
		, nContainerEntries
		, maxResubmissions
		, nWorkers
	)
		){ exit(0); }

	//--------------------------------------------------------------------------------
	// Initialize the broker
	GBROKER(WORKLOAD)->init();

	//--------------------------------------------------------------------------------
	// If we are in (networked) client mode, start the client code
	if((executionMode==Gem::Courtier::Tests::GBSCModes::NETWORKING || executionMode==Gem::Courtier::Tests::GBSCModes::THREAEDANDNETWORKING) && !serverMode) {
		std::shared_ptr<GAsioTCPClientT<WORKLOAD>> p(new GAsioTCPClientT<WORKLOAD>(ip, boost::lexical_cast<std::string>(port)));

		p->setMaxStalls(0); // An infinite number of stalled data retrievals
		p->setMaxConnectionAttempts(100); // Up to 100 failed connection attempts

		// Start the actual processing loop
		p->run();

		return 0;
	}

	//--------------------------------------------------------------------------------
	// Create the required number of connectorProducer threads
	if(useDirectBrokerConnection) {
		connectorProducer_gtg.create_threads(
			std::bind(
				brokerProducer
				, nProductionCycles
				, nContainerObjects
				, nContainerEntries
			)
			, nProducers
		);
	} else {
		connectorProducer_gtg.create_threads(
			std::bind(
				connectorProducer
				, nProductionCycles
				, nContainerObjects
				, nContainerEntries
				, srm
				, maxResubmissions
			)
			, nProducers
		);
	}

	//--------------------------------------------------------------------------------
	// Add the desired consumers to the broker
	switch(executionMode) {
		case Gem::Courtier::Tests::GBSCModes::SERIAL:
		{
			std::cout << "Using a serial consumer" << std::endl;

			// Create a serial consumer and enrol it with the broker
			std::shared_ptr<GSerialConsumerT<WORKLOAD>> gatc(new GSerialConsumerT<WORKLOAD>());
			GBROKER(WORKLOAD)->enrol(gatc);
		}
			break;

		case Gem::Courtier::Tests::GBSCModes::INTERNALNETWORKING:
		{
			std::cout << "Using internal networking" << std::endl;

			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioTCPConsumerT<WORKLOAD>> gatc(new GAsioTCPConsumerT<WORKLOAD>((unsigned short)10000));
			GBROKER(WORKLOAD)->enrol(gatc);

			// Start the workers
			clients.clear();
			for(std::size_t worker=0; worker<nWorkers; worker++) {
				std::shared_ptr<GAsioTCPClientT<WORKLOAD>> p(new GAsioTCPClientT<WORKLOAD>("localhost", "10000"));
				clients.push_back(p);

				worker_gtg.create_thread( [p](){ p->run(); } );
			}
		}
			break;

		case Gem::Courtier::Tests::GBSCModes::NETWORKING:
		{
			std::cout << "Using networked mode" << std::endl;

			// Create a network consumer and enrol it with the broker
			std::shared_ptr<GAsioTCPConsumerT<WORKLOAD>> gatc(new GAsioTCPConsumerT<WORKLOAD>(port));
			GBROKER(WORKLOAD)->enrol(gatc);
		}
			break;

		case Gem::Courtier::Tests::GBSCModes::MULTITHREADING:
		{
			std::cout << "Using the multithreaded mode" << std::endl;

			// Create a consumer and make it known to the global broker
			std::shared_ptr< GBoostThreadConsumerT<WORKLOAD>> gbtc(new GBoostThreadConsumerT<WORKLOAD>());
			gbtc->setNThreadsPerWorker(10);
			GBROKER(WORKLOAD)->enrol(gbtc);
		}
			break;

		case Gem::Courtier::Tests::GBSCModes::THREADANDINTERNALNETWORKING:
		{
			std::cout << "Using multithreading and internal networking" << std::endl;

			std::shared_ptr<GAsioTCPConsumerT<WORKLOAD>> gatc(new GAsioTCPConsumerT<WORKLOAD>((unsigned short)10000));
			std::shared_ptr< GBoostThreadConsumerT<WORKLOAD>> gbtc(new GBoostThreadConsumerT<WORKLOAD>());

			GBROKER(WORKLOAD)->enrol(gatc);
			GBROKER(WORKLOAD)->enrol(gbtc);

			// Start the workers
			clients.clear();
			for(std::size_t worker=0; worker<nWorkers; worker++) {
				std::shared_ptr<GAsioTCPClientT<WORKLOAD>> p(new GAsioTCPClientT<WORKLOAD>("localhost", "10000"));
				clients.push_back(p);

				worker_gtg.create_thread( [p](){ p->run(); } );
			}
		}
			break;

		case Gem::Courtier::Tests::GBSCModes::THREAEDANDNETWORKING:
		{
			std::cout << "Using multithreading and the networked mode" << std::endl;

			std::shared_ptr<GAsioTCPConsumerT<WORKLOAD>> gatc(new GAsioTCPConsumerT<WORKLOAD>(port));
			std::shared_ptr< GBoostThreadConsumerT<WORKLOAD>> gbtc(new GBoostThreadConsumerT<WORKLOAD>());

			GBROKER(WORKLOAD)->enrol(gatc);
			GBROKER(WORKLOAD)->enrol(gbtc);
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
	connectorProducer_gtg.join_all();

	if(
		executionMode == Gem::Courtier::Tests::GBSCModes::INTERNALNETWORKING ||
		executionMode == Gem::Courtier::Tests::GBSCModes::THREADANDINTERNALNETWORKING
		) {
		worker_gtg.join_all();
	}

	std::cout << "All threads have joined" << std::endl;

	// Terminate the broker
	GBROKER(WORKLOAD)->finalize();
}
