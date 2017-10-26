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

#include "geneva/G_OptimizationAlgorithm_SimulatedAnnealing_Factory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory()
	: G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>(
	"./config/GSimulatedAnnealing.json")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory(
	const std::string &configFile
)
	: G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>(configFile)
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
	: G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>(configFile, contentCreatorPtr)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GSimulatedAnnealingFactory::GSimulatedAnnealingFactory(const GSimulatedAnnealingFactory& cp)
	: G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>(cp)
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
	return GSimulatedAnnealing_PersonalityTraits::nickname;
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
std::shared_ptr<G_OptimizationAlgorithm_Base> GSimulatedAnnealingFactory::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	std::shared_ptr<GSimulatedAnnealing> target(
		new GSimulatedAnnealing()
	);

	// Make the local configuration options known (up to the level of GSimulatedAnnealing)
	target->GSimulatedAnnealing::addConfigurationOptions(gpb);

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
	std::shared_ptr<G_OptimizationAlgorithm_Base>& p_base
) {
	// Call our parent class'es function
	G_OptimizationAlgorithm_FactoryT<G_OptimizationAlgorithm_Base>::postProcess_(p_base);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
