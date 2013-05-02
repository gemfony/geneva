/**
 * @file GSwarmAlgorithmFactory.cpp
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

#include "geneva/GSwarmAlgorithmFactory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
const std::string GSwarmAlgorithmFactory::nickname = "swarm";

/******************************************************************************/
/** This will register the factory in the global factory store */
GOAInitializerT<GSwarmAlgorithmFactory> GSwarmStoreRegistrant;

/******************************************************************************/
/**
 * The default constructor
 */
GSwarmAlgorithmFactory::GSwarmAlgorithmFactory()
   : GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >("./config/GSwarmAlgorithm.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file and the default parallelization mode
 */
GSwarmAlgorithmFactory::GSwarmAlgorithmFactory(
      const std::string& configFile
)
   : GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 */
GSwarmAlgorithmFactory::GSwarmAlgorithmFactory(
	const std::string& configFile
	, const parMode& pm
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >(configFile, pm)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GSwarmAlgorithmFactory::GSwarmAlgorithmFactory(
   const std::string& configFile
   , const parMode& pm
   , boost::shared_ptr<Gem::Common::GFactoryT<GParameterSet> > contentCreatorPtr
)
   : GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet> >(configFile, pm, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GSwarmAlgorithmFactory::~GSwarmAlgorithmFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet> > GSwarmAlgorithmFactory::getObject_(
	Gem::Common::GParserBuilder& gpb
	, const std::size_t& id
) {
	// Will hold the result
	boost::shared_ptr<GBaseSwarm> target;

	// Fill the target pointer as required
	switch(pm_) {
	case PARMODE_SERIAL:
		target = boost::shared_ptr<GSerialSwarm>(new GSerialSwarm());
		break;

	case PARMODE_MULTITHREADED:
		target = boost::shared_ptr<GMultiThreadedSwarm>(new GMultiThreadedSwarm());
		break;

	case PARMODE_BROKERAGE:
		target = boost::shared_ptr<GBrokerSwarm>(new GBrokerSwarm());
		break;
	}

	// Make the local configuration options known
	target->GBaseSwarm::addConfigurationOptions(gpb, true);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GSwarmAlgorithmFactory::postProcess_(boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet> >& p_base) {
	// Convert the object to the correct target type
	switch(pm_) {
	case PARMODE_SERIAL:
		// nothing
		break;

	case PARMODE_MULTITHREADED:
		{
			boost::shared_ptr<GMultiThreadedSwarm> p
			   = Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GMultiThreadedSwarm>(p_base);
			p->setNThreads(nEvaluationThreads_);
		}
		break;

	case PARMODE_BROKERAGE:
		{
			boost::shared_ptr<GBrokerSwarm> p
			   = Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GBrokerSwarm>(p_base);

			p->setFirstTimeOut(firstTimeOut_);
			p->setWaitFactorExtremes(minWaitFactor_, maxWaitFactor_);
			p->doLogging(doLogging_);
			p->setBoundlessWait(boundlessWait_);
			p->setWaitFactorIncrement(waitFactorIncrement_);
		}
		break;
	}
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
