/**
 * @file GExternalEvaluatorConsumer.hpp
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


// Standard headers go here

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#ifndef GEXTERNALEVALUATORCONSUMER_HPP_
#define GEXTERNALEVALUATORCONSUMER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"
#include "courtier/GBrokerT.hpp"
#include "courtier/GConsumer.hpp"
#include "courtier/GBoostThreadConsumerT.hpp"

namespace Gem {
namespace Geneva {

/*******************************************************************/
/**
 * This class is meant to be the base of a hierarchy of classes
 * which take evaluation of individuals into their own hands. This
 * may be useful when evaluation happens through an external hardware
 * device or is, possibly, even carried out by humans. For this purpose
 * the library converts the GIndividual objects it gets from the broker
 * into GExternalEvaluatorIndividual objects and hands them over to
 * a processing function, which needs to be re-implemented by derived
 * classes. All actions can be performed in multi-threaded mode. In order
 * not to duplicate code, GExternalEvaluatorConsumer thus derives from
 * GBoostThreadConsumerT<>. Note that the parent-parent class GConsumer
 * is derived from boost::noncopyable, so we can neglect copy constructors
 * and assignment operators.
 */
class GExternalEvaluatorConsumer
	:public Gem::Courtier::GBoostThreadConsumerT<Gem::Geneva::GIndividual>
{
public:
	/** @brief The default constructor */
	GExternalEvaluatorConsumer();
	/** @brief The destructor */
	virtual ~GExternalEvaluatorConsumer();

	/** @brief An overloaded version of GBoostThreadConsumerT<>'s main processing function */
	virtual void processItems(boost::shared_ptr<Gem::Geneva::GIndividual>);

	/** @brief Identify this consumer */
	virtual std::string getConsumerName() const;

protected:
	/**
	 * Does the actual work on the GExternalEvaluatorIndividual objects. It is to be specified
	 * by derived classes. Note that it is called from within multiple threads, so it may only
	 * access thread-safe resources.
	 */
	virtual double customProcessItems(
			boost::shared_ptr<GExternalEvaluatorIndividual>
			, std::vector<double>&
	) = 0;
};

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GEXTERNALEVALUATORCONSUMER_HPP_ */
