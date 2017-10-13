/**
 * @file G_OA_ParameterScanFactory.cpp
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

#include "geneva/G_OA_ParameterScanFactory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GParameterScanFactory2::GParameterScanFactory2()
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>("./config/GParameterScan.json")
	, m_parameterSpecCL("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the name of the config file
 */
GParameterScanFactory2::GParameterScanFactory2(
	const std::string &configFile
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(configFile)
  	, m_parameterSpecCL("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode and
 * to add a content creator. It initializes a target item as needed.
 */
GParameterScanFactory2::GParameterScanFactory2(
	const std::string &configFile
	, std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> contentCreatorPtr
)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(configFile, contentCreatorPtr)
   , m_parameterSpecCL("empty")
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GParameterScanFactory2::GParameterScanFactory2(const GParameterScanFactory2& cp)
	: GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>(cp)
  	, m_parameterSpecCL(cp.m_parameterSpecCL)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterScanFactory2::~GParameterScanFactory2()
{ /* nothing */ }

/******************************************************************************/
/**
 * Gives access to the mnemonics / nickname describing an algorithm
 */
std::string GParameterScanFactory2::getMnemonic() const {
	return GPSPersonalityTraits::nickname;
}

/******************************************************************************/
/**
 * Gives access to a clear-text description of the algorithm
 */
std::string GParameterScanFactory2::getAlgorithmName() const {
	return std::string("Parameter Scan");
}

/***************************************************************************/
/**
 * Adds local command line options to a boost::program_options::options_description object.
 *
 * @param visible Command line options that should always be visible
 * @param hidden Command line options that should only be visible upon request
 */
void GParameterScanFactory2::addCLOptions(
	boost::program_options::options_description &visible
	, boost::program_options::options_description &hidden
) {
	namespace po = boost::program_options;

	hidden.add_options()(
		"parameterSpec"
		, po::value<std::string>(&m_parameterSpecCL)->default_value(std::string("empty"))
		, "\t[GParameterScanFactory] Specification of parameters to be scanned. Syntax: \"d(0, -10., 10., 100)\". Use a comma-separated list for more than one variable. A single entry \"s(1000)\" will lead to a random scan over all parameters of up to 1000 individuals"
	);

	// Add the parent class'es options
	GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>::addCLOptions(visible, hidden);
}

/******************************************************************************/
/**
 * Allows to specify the command line parameter manually for variables to be scanned
 */
void GParameterScanFactory2::setCLParameterSpecs(std::string parStr) {
	m_parameterSpecCL = parStr;
}

/******************************************************************************/
/**
 * Allows to retrieve the command line parameter settings for variables to be scanned
 */
std::string GParameterScanFactory2::getCLParameterSpecs() const {
	return m_parameterSpecCL;
}

/******************************************************************************/
/**
 * Allows to reset the command line parameter specs
 */
void GParameterScanFactory2::resetCLParameterSpecs() {
	m_parameterSpecCL = "empty";
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>> GParameterScanFactory2::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	std::shared_ptr<GParameterScan> target(
		new GParameterScan()
	);

	// Make the local configuration options known (up to the level of GParameterScan)
	target->GParameterScan::addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GParameterScanFactory2::postProcess_(
	std::shared_ptr<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>& p_base
) {
	if(m_parameterSpecCL != "empty") {
		std::shared_ptr<GParameterScan> p
			= Gem::Common::convertSmartPointer<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>, GParameterScan> (p_base);

		p->setParameterSpecs(m_parameterSpecCL);
	}

	// Call our parent class'es function
	GOptimizationAlgorithmFactoryT<GOptimizationAlgorithmT<Gem::Courtier::GBrokerExecutorT<GParameterSet>>>::postProcess_(p_base);
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
