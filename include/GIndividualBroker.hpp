/**
 * @file GIndividualBroker.hpp
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

// Standard headers go here

#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>

#ifndef GINDIVIDUALBROKER_HPP_
#define GINDIVIDUALBROKER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here
#include "GBrokerT.hpp"
#include "GIndividual.hpp"
#include "GSingletonT.hpp"

namespace Gem {
namespace GenEvA {

/**************************************************************************************/
/**
 * A broker class that is specialized on GIndividual objects
 */
class GIndividualBroker
	:public Gem::Util::GBrokerT<boost::shared_ptr<Gem::GenEvA::GIndividual> >
{
public:
	GIndividualBroker()
		:Gem::Util::GBrokerT<boost::shared_ptr<Gem::GenEvA::GIndividual> >()
	{ /* nothing */	}

	~GIndividualBroker()
	{
		std::cout << "GIndividualBroker has terminated" << std::endl;
	}
};

/**************************************************************************************/
/**
 * We require the global GIndividualBroker object to be a singleton. This
 * ensures that one and only one Broker object exists that is constructed
 * before main begins. All external communication should refer to GINDIVIDUALBROKER.
 */
typedef Gem::Util::GSingletonT<GIndividualBroker> gindividualbroker;
#define GINDIVIDUALBROKER gindividualbroker::getInstance()

/**************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GINDIVIDUALBROKER_HPP_ */
