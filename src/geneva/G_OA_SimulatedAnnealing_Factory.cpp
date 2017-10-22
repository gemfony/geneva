/**
 * @file G_OA_SimulatedAnnealing_Factory.cpp
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

#include "geneva/G_OA_SimulatedAnnealing_Factory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory()
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(
	"./config/GSimulatedAnnealing.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory(
	const std::string &configFile
	, std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(configFile, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory(const GSimulatedAnnealingFactory& cp)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GSimulatedAnnealingFactory::~GSimulatedAnnealingFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GSimulatedAnnealingFactory::getMnemonic() const {
	return GSAPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GSimulatedAnnealingFactory::getAlgorithmName() const {
	return std::string("Simulated Annealing");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>> GSimulatedAnnealingFactory::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	std::shared_ptr<GBrokerSimulatedAnnealing> target(
		new GBrokerSimulatedAnnealing()
	);

	// Make the local configuration options known (up to the level of GBrokerSimulatedAnnealing)
	target->GBrokerSimulatedAnnealing::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GSimulatedAnnealingFactory::postProcess_(
	std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>& p_base
) {
	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>::postProcess_(p_base);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
G_MT_SimulatedAnnealingFactory::G_MT_SimulatedAnnealingFactory()
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>(
	"./config/GSimulatedAnnealing.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
G_MT_SimulatedAnnealingFactory::G_MT_SimulatedAnnealingFactory(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
G_MT_SimulatedAnnealingFactory::G_MT_SimulatedAnnealingFactory(
	const std::string &configFile
	, std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>(configFile, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
G_MT_SimulatedAnnealingFactory::G_MT_SimulatedAnnealingFactory(const G_MT_SimulatedAnnealingFactory& cp)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
G_MT_SimulatedAnnealingFactory::~G_MT_SimulatedAnnealingFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string G_MT_SimulatedAnnealingFactory::getMnemonic() const {
	return GSAPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string G_MT_SimulatedAnnealingFactory::getAlgorithmName() const {
	return std::string("Simulated Annealing");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>> G_MT_SimulatedAnnealingFactory::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	std::shared_ptr<GMTSimulatedAnnealing> target(
		new GMTSimulatedAnnealing()
	);

	// Make the local configuration options known (up to the level of GBrokerSimulatedAnnealing)
	target->GMTSimulatedAnnealing::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void G_MT_SimulatedAnnealingFactory::postProcess_(
	std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>& p_base
) {
	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>::postProcess_(p_base);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
G_Serial_SimulatedAnnealingFactory::G_Serial_SimulatedAnnealingFactory()
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>(
	"./config/GSimulatedAnnealing.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
G_Serial_SimulatedAnnealingFactory::G_Serial_SimulatedAnnealingFactory(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
G_Serial_SimulatedAnnealingFactory::G_Serial_SimulatedAnnealingFactory(
	const std::string &configFile
	, std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>(configFile, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
G_Serial_SimulatedAnnealingFactory::G_Serial_SimulatedAnnealingFactory(const G_Serial_SimulatedAnnealingFactory& cp)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
G_Serial_SimulatedAnnealingFactory::~G_Serial_SimulatedAnnealingFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string G_Serial_SimulatedAnnealingFactory::getMnemonic() const {
	return GSAPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string G_Serial_SimulatedAnnealingFactory::getAlgorithmName() const {
	return std::string("Simulated Annealing");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>> G_Serial_SimulatedAnnealingFactory::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	std::shared_ptr<GSerialSimulatedAnnealing> target(
		new GSerialSimulatedAnnealing()
	);

	// Make the local configuration options known (up to the level of GBrokerSimulatedAnnealing)
	target->GSerialSimulatedAnnealing::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void G_Serial_SimulatedAnnealingFactory::postProcess_(
	std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>& p_base
) {
	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>::postProcess_(p_base);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
