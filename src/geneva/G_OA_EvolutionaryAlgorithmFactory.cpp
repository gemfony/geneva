/**
 * @file G_OA_EvolutionaryAlgorithmFactory.cpp
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

#include "geneva/G_OA_EvolutionaryAlgorithmFactory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GEvolutionaryAlgorithmFactory2::GEvolutionaryAlgorithmFactory2()
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(
	"./config/GEvolutionaryAlgorithm.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GEvolutionaryAlgorithmFactory2::GEvolutionaryAlgorithmFactory2(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GEvolutionaryAlgorithmFactory2::GEvolutionaryAlgorithmFactory2(
	const std::string &configFile
	, std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(configFile, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GEvolutionaryAlgorithmFactory2::GEvolutionaryAlgorithmFactory2(const GEvolutionaryAlgorithmFactory2& cp)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GEvolutionaryAlgorithmFactory2::~GEvolutionaryAlgorithmFactory2()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GEvolutionaryAlgorithmFactory2::getMnemonic() const {
	return GEAPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GEvolutionaryAlgorithmFactory2::getAlgorithmName() const {
	return std::string("Evolutionary Algorithm");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>> GEvolutionaryAlgorithmFactory2::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	std::shared_ptr<GEvolutionaryAlgorithm> target(
		new GEvolutionaryAlgorithm()
	);

	// Make the local configuration options known (up to the level of GEvolutionaryAlgorithm)
	target->GEvolutionaryAlgorithm::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GEvolutionaryAlgorithmFactory2::postProcess_(
	std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>& p_base
) {
	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>::postProcess_(p_base);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
G_MT_EvolutionaryAlgorithmFactory::G_MT_EvolutionaryAlgorithmFactory()
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>(
	"./config/GEvolutionaryAlgorithm.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
G_MT_EvolutionaryAlgorithmFactory::G_MT_EvolutionaryAlgorithmFactory(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
G_MT_EvolutionaryAlgorithmFactory::G_MT_EvolutionaryAlgorithmFactory(
	const std::string &configFile
	, std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>(configFile, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
G_MT_EvolutionaryAlgorithmFactory::G_MT_EvolutionaryAlgorithmFactory(const G_MT_EvolutionaryAlgorithmFactory& cp)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
G_MT_EvolutionaryAlgorithmFactory::~G_MT_EvolutionaryAlgorithmFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string G_MT_EvolutionaryAlgorithmFactory::getMnemonic() const {
	return GEAPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string G_MT_EvolutionaryAlgorithmFactory::getAlgorithmName() const {
	return std::string("Evolutionary Algorithm");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>> G_MT_EvolutionaryAlgorithmFactory::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	std::shared_ptr<GMTEvolutionaryAlgorithm> target(
		new GMTEvolutionaryAlgorithm()
	);

	// Make the local configuration options known (up to the level of GMTEvolutionaryAlgorithm)
	target->GMTEvolutionaryAlgorithm::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void G_MT_EvolutionaryAlgorithmFactory::postProcess_(
	std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>& p_base
) {
	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GMTExecutorT<GParameterSet>>>::postProcess_(p_base);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
G_Serial_EvolutionaryAlgorithmFactory::G_Serial_EvolutionaryAlgorithmFactory()
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>(
	"./config/GEvolutionaryAlgorithm.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
G_Serial_EvolutionaryAlgorithmFactory::G_Serial_EvolutionaryAlgorithmFactory(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>(configFile)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
G_Serial_EvolutionaryAlgorithmFactory::G_Serial_EvolutionaryAlgorithmFactory(
	const std::string &configFile
	, std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>(configFile, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
G_Serial_EvolutionaryAlgorithmFactory::G_Serial_EvolutionaryAlgorithmFactory(const G_Serial_EvolutionaryAlgorithmFactory& cp)
	: GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
G_Serial_EvolutionaryAlgorithmFactory::~G_Serial_EvolutionaryAlgorithmFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string G_Serial_EvolutionaryAlgorithmFactory::getMnemonic() const {
	return GEAPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string G_Serial_EvolutionaryAlgorithmFactory::getAlgorithmName() const {
	return std::string("Evolutionary Algorithm");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>> G_Serial_EvolutionaryAlgorithmFactory::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	std::shared_ptr<GSerialEvolutionaryAlgorithm> target(
		new GSerialEvolutionaryAlgorithm()
	);

	// Make the local configuration options known (up to the level of GMTEvolutionaryAlgorithm)
	target->GSerialEvolutionaryAlgorithm::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void G_Serial_EvolutionaryAlgorithmFactory::postProcess_(
	std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>& p_base
) {
	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT2<GOptimizationAlgorithmT<Gem::Courtier::GSerialExecutorT<GParameterSet>>>::postProcess_(p_base);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
