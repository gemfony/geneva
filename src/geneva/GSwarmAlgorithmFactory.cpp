/**
 * @file GSwarmAlgorithmFactory.cpp
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

#include "geneva/GSwarmAlgorithmFactory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GSwarmAlgorithmFactory::GSwarmAlgorithmFactory()
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>("./config/GSwarmAlgorithm.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file and the default parallelization mode
 */
GSwarmAlgorithmFactory::GSwarmAlgorithmFactory(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 */
GSwarmAlgorithmFactory::GSwarmAlgorithmFactory(
	const std::string &configFile, const execMode &pm
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>(configFile, pm)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GSwarmAlgorithmFactory::GSwarmAlgorithmFactory(
	const std::string &configFile, const execMode &pm,
	std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>(configFile, pm, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GSwarmAlgorithmFactory::~GSwarmAlgorithmFactory() { /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GSwarmAlgorithmFactory::getMnemonic() const {
	return GSwarmPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GSwarmAlgorithmFactory::getAlgorithmName() const {
	return std::string("Swarm Algorithm");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr <GOptimizationAlgorithmT<GParameterSet>> GSwarmAlgorithmFactory::getObject_(
	Gem::Common::GParserBuilder &gpb, const std::size_t &id
) {
	// Will hold the result
	std::shared_ptr <GBaseSwarm> target;

	// Fill the target pointer as required
	switch (GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_pm) {
		case execMode::EXECMODE_SERIAL:
			target = std::shared_ptr<GSerialSwarm>(new GSerialSwarm());
			break;

		case execMode::EXECMODE_MULTITHREADED:
			target = std::shared_ptr<GMultiThreadedSwarm>(new GMultiThreadedSwarm());
			break;

		case execMode::EXECMODE_BROKERAGE:
			target = std::shared_ptr<GBrokerSwarm>(new GBrokerSwarm());
			break;
	}

	// Make the local configuration options known
	target->GBaseSwarm::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GSwarmAlgorithmFactory::postProcess_(std::shared_ptr < GOptimizationAlgorithmT<GParameterSet>> &p_base) {
	// Convert the object to the correct target type
	switch (GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_pm) {
		case execMode::EXECMODE_SERIAL: {
			// nothing
		} break;

		case execMode::EXECMODE_MULTITHREADED: {
			std::shared_ptr <GMultiThreadedSwarm> p
				= Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GMultiThreadedSwarm>(p_base);
			p->setNThreads(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_nEvaluationThreads);
		} break;

		case execMode::EXECMODE_BROKERAGE: {
			std::shared_ptr <GBrokerSwarm> p
				= Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GBrokerSwarm>(p_base);

			p->setWaitFactor(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_waitFactor);
			p->setInitialWaitFactor(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_initialWaitFactor);
		} break;
	}

	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::postProcess_(p_base);
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
