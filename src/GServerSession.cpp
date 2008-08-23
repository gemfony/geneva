/**
 * @file GServerSession.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GServerSession.hpp"

namespace Gem {
namespace GenEvA {

/*********************************************************************/
/**
 * The default constructor.
 */
GServerSession::GServerSession()
{ /* nothing */ }

/*********************************************************************/
/**
 * The standard destructor. We have no local, dynamically allocated data,
 * hence it is empty.
 */
GServerSession::~GServerSession()
{ /* nothing */ }

/*********************************************************************/
/**
 * This function processes an individual request from a client.
 */
void GServerSession::processRequest() {
	boost::posix_time::time_duration timeout(boost::posix_time::milliseconds(10));

	// Every client transmission starts with a command
	std::string command = getSingleCommand();

	// Retrieve an item from the broker and
	// submit it to the client.
	if (command == "ready") {
		try{
			boost::shared_ptr<GIndividual> p(new GIndividual);
			Gem::Util::PORTIDTYPE id;

			// Retrieve an item
			id = GINDIVIDUALBROKER.get(p, timeout);

			// Store the id in the individual
			p->setAttribute("id", boost::lexical_cast<std::string>(id));

			if (!this->submit(p->toString(),"compute")) {
				std::ostringstream information;
				information << "In GServerSession::processRequest():" << std::endl
							<< "Could not submit item to client!" << std::endl;

				LOGGER.log(information.str(), Gem::GLogFramework::INFORMATIONAL);
			}
		}
		catch(Gem::Util::gem_util_condition_time_out &) {
			this->sendSingleCommand("timeout");
		}
	}
	// Retrieve an item from the client and
	// submit it to the broker
	else if (command == "result") {
		std::string itemString, portid, fitness, dirtyflag;

		if (this->retrieve(itemString, portid, fitness, dirtyflag)) {
			// Give the object back to the broker, so it can be sorted back
			// into the GBufferPort objects.
			boost::shared_ptr<GIndividual> p(new GIndividual(itemString));
			Gem::Util::PORTIDTYPE id = boost::lexical_cast<Gem::Util::PORTIDTYPE>(portid);
			GINDIVIDUALBROKER.put(id, p);
		}
		else {
			std::ostringstream information;
			information << "GServerSession::processRequest():" << std::endl
						<< "Could not retrieve item from client." << std::endl;
			LOGGER.log(information.str(), Gem::GLogFramework::INFORMATIONAL);
		}
	}
	else {
		std::ostringstream warning;
		warning << "In GServerSession::processRequest: Warning!" << std::endl
				<< "Received unkown command." << command << std::endl;
		LOGGER.log(warning.str(), Gem::GLogFramework::WARNING);

		this->sendSingleCommand("unknown");
	}
}

/*********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
