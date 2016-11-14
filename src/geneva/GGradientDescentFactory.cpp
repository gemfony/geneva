/**
 * @file GGradientDescentFactory.cpp
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

#include "geneva/GGradientDescentFactory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GGradientDescentFactory::GGradientDescentFactory()
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>("./config/GGradientDescent.json")
   , maxResubmissions_(5)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file and the default parallelization mode
 */
GGradientDescentFactory::GGradientDescentFactory(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>(configFile)
	, maxResubmissions_(5)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 */
GGradientDescentFactory::GGradientDescentFactory(
	const std::string &configFile
	, const execMode &pm
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>(configFile, pm)
	, maxResubmissions_(5)
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GGradientDescentFactory::GGradientDescentFactory(
	const std::string &configFile
	, const execMode &pm
	, std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>(configFile, pm, contentCreatorPtr)
	, maxResubmissions_(5)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GGradientDescentFactory::~GGradientDescentFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GGradientDescentFactory::getMnemonic() const {
	return GGDPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GGradientDescentFactory::getAlgorithmName() const {
	return std::string("Gradient Descent");
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr <GOptimizationAlgorithmT<GParameterSet>> GGradientDescentFactory::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	// Will hold the result
	std::shared_ptr <GBaseGD> target;

	// Fill the target pointer as required
	switch (GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_pm) {
		case execMode::EXECMODE_SERIAL:
			target = std::shared_ptr<GSerialGD>(new GSerialGD());
			break;

		case execMode::EXECMODE_MULTITHREADED:
			target = std::shared_ptr<GMultiThreadedGD>(new GMultiThreadedGD());
			break;

		case execMode::EXECMODE_BROKERAGE:
			target = std::shared_ptr<GBrokerGD>(new GBrokerGD());
			break;
	}

	// Make the local configuration options known
	target->GBaseGD::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GGradientDescentFactory::describeLocalOptions_(Gem::Common::GParserBuilder &gpb) {
	// Describe our own options
	using namespace Gem::Courtier;

	std::string comment;

	comment = "";
	comment += "The maximum number of allowed re-submissions in an iteration;";
	gpb.registerFileParameter<std::size_t>(
		"maxResubmissions", maxResubmissions_, DEFAULTMAXRESUBMISSIONS, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	// Allow our parent class to describe its options
	GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GGradientDescentFactory::postProcess_(std::shared_ptr < GOptimizationAlgorithmT<GParameterSet>> &p_base) {
	// Convert the object to the correct target type
	switch (GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_pm) {
		case execMode::EXECMODE_SERIAL:
			// nothing
			break;

		case execMode::EXECMODE_MULTITHREADED: {
			std::shared_ptr <GMultiThreadedGD> p
				= Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GMultiThreadedGD>(p_base);
			p->setNThreads(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_nEvaluationThreads);
		}
			break;

		case execMode::EXECMODE_BROKERAGE: {
			std::shared_ptr <GBrokerGD> p
				= Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<GParameterSet>, GBrokerGD>(p_base);

			p->setWaitFactor(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_waitFactor);
			p->setInitialWaitFactor(GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::m_initialWaitFactor);

			// This differs from e.g. GEvolutionaryAlgorithmFactory
			p->setMaxResubmissions(maxResubmissions_);
		}
			break;
	}

	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<GParameterSet>>::postProcess_(p_base);
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
