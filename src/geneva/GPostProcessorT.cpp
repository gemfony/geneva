/**
 * @file GPostProcessorT.cpp
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

#include "geneva/GPostProcessorT.hpp"

// Export of GEvolutionaryAlgorithmPostOptimizer
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GEvolutionaryAlgorithmPostOptimizer)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with the execution mode and configuration file
 */
GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer(
	execMode executionMode
	, std::string configFile
) :
	GPostProcessorBaseT<GParameterSet>()
	, m_configFile(configFile)
	, m_executionMode((executionMode == execMode::SERIAL || executionMode == execMode::MULTITHREADED) ? executionMode : execMode::SERIAL)
{
	switch (executionMode) {
		case execMode::SERIAL:
		case execMode::MULTITHREADED:
			/* nothing */
			break;

		case execMode::BROKER:
		default: {
			glogger
				<< "In GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer(execMode): Error!"
				<< std::endl
				<< "Got invalid execution mode " << executionMode << std::endl
				<< "The mode was reset to execMode::SERIAL" << std::endl
				<< GWARNING;
		}
	}
}

/******************************************************************************/
/**
 * The copy constructor
 */
GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer(const GEvolutionaryAlgorithmPostOptimizer& cp) :
	GPostProcessorBaseT<GParameterSet>(cp)
	, m_configFile(cp.m_configFile)
	, m_executionMode(cp.m_executionMode) // We assume that a valid execution mode is stored here
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GEvolutionaryAlgorithmPostOptimizer::~GEvolutionaryAlgorithmPostOptimizer()
{ /* nothing */ }

/******************************************************************************/
/**
 * Checks for equality with another GEvolutionaryAlgorithmPostOptimizer object
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
 * @return A boolean indicating whether both objects are equal
 */
bool GEvolutionaryAlgorithmPostOptimizer::operator==(const GEvolutionaryAlgorithmPostOptimizer &cp) const
{
	using namespace Gem::Common;
	try {
		this->compare(
			cp
			, Gem::Common::expectation::CE_EQUALITY
			, CE_DEF_SIMILARITY_DIFFERENCE
		);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GEvolutionaryAlgorithmPostOptimizer object
 *
 * @param  cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
 * @return A boolean indicating whether both objects are inequal
 */
bool GEvolutionaryAlgorithmPostOptimizer::operator!=(const GEvolutionaryAlgorithmPostOptimizer &cp) const
{
	using namespace Gem::Common;
	try {
		this->compare(
			cp
			, Gem::Common::expectation::CE_INEQUALITY
			, CE_DEF_SIMILARITY_DIFFERENCE
		);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Returns the name of this class
 */
std::string GEvolutionaryAlgorithmPostOptimizer::name() const
{
	return std::string("GEvolutionaryAlgorithmPostOptimizer");
}

/******************************************************************************/
/**
 * Checks for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GEvolutionaryAlgorithmPostOptimizer object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GEvolutionaryAlgorithmPostOptimizer::compare(
	const Gem::Common::GSerializableFunctionObjectT<GParameterSet> &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const  {
	using namespace Gem::Common;

	// Check that we are dealing with a Gem::Common::GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
	const GEvolutionaryAlgorithmPostOptimizer *p_load
		= Gem::Common::g_convert_and_compare<Gem::Common::GSerializableFunctionObjectT<GParameterSet>, GEvolutionaryAlgorithmPostOptimizer>(
			cp
			, this
		);

	GToken token(
		"GEvolutionaryAlgorithmPostOptimizer"
		, e
	);

	// Compare our parent data ...
	Gem::Common::compare_base<GPostProcessorBaseT < GParameterSet>>
	(IDENTITY(
		*this
		, *p_load
	), token);

	// ... and then our local data
	compare_t(
		IDENTITY(
			m_configFile
			, p_load->m_configFile
		)
		, token
	);
	compare_t(
		IDENTITY(
			m_executionMode
			, p_load->m_executionMode
		)
		, token
	);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Allows to set the execution mode for this post-processor (serial vs. multi-threaded)
 */
void GEvolutionaryAlgorithmPostOptimizer::setExecMode(execMode executionMode)
{
	switch (executionMode) {
		case execMode::SERIAL:
		case execMode::MULTITHREADED: {
			m_executionMode = executionMode;
		}
			break;

		case execMode::BROKER:
		default: {
			glogger
				<< "In GEvolutionaryAlgorithmPostOptimizer::setExecMode(): Error!" << std::endl
				<< "Got invalid execution mode " << executionMode << std::endl
				<< GEXCEPTION;
		}
	}
}

/******************************************************************************/
/**
 * Allows to retrieve the current execution mode
 */
execMode GEvolutionaryAlgorithmPostOptimizer::getExecMode() const
{
	return m_executionMode;
}

/******************************************************************************/
/**
 * Allows to specify the name of a configuration file
 */
void GEvolutionaryAlgorithmPostOptimizer::setConfigFile(std::string configFile)
{
	m_configFile = configFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the configuration file
 */
std::string GEvolutionaryAlgorithmPostOptimizer::getConfigFile() const
{
	return m_configFile;
}

/******************************************************************************/
/**
 * Loads the data of another GEvolutionaryAlgorithmPostOptimizer object
 */
void GEvolutionaryAlgorithmPostOptimizer::load_(const Gem::Common::GSerializableFunctionObjectT<GParameterSet> *cp)
{
	// Check that we are dealing with a GEvolutionaryAlgorithmPostOptimizer reference independent of this object and convert the pointer
	const GEvolutionaryAlgorithmPostOptimizer *p_load
		= Gem::Common::g_convert_and_compare<Gem::Common::GSerializableFunctionObjectT<GParameterSet>, GEvolutionaryAlgorithmPostOptimizer>(
			cp
			, this
		);

	// Load our parent class'es data ...
	GPostProcessorBaseT<GParameterSet>::load_(cp);

	// ... and then our local data
	m_configFile = p_load->m_configFile;
	m_executionMode = p_load->m_executionMode;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
Gem::Common::GSerializableFunctionObjectT<GParameterSet> *GEvolutionaryAlgorithmPostOptimizer::clone_() const
{
	return new GEvolutionaryAlgorithmPostOptimizer(*this);
}

/******************************************************************************/
/**
 * The actual post-processing takes place here (no further checks)
 */
bool GEvolutionaryAlgorithmPostOptimizer::raw_processing_(GParameterSet &p)
{

	// Make sure p is clean
	if (p.isDirty()) {
		glogger
			<< "In GEvolutionaryAlgorithmPostOptimizer::raw_processing_: Error!" << std::endl
			<< "Provided base_type has dirty flag set." << std::endl
			<< GEXCEPTION;
	}

	switch(m_executionMode) {
		case execMode::MULTITHREADED:
		{
			// Clone the individual for post-processing
			std::shared_ptr <GParameterSet> p_unopt_ptr = p.template clone<GParameterSet>();

			// Make sure the post-optimization does not trigger post-optimization recursively ...
			p_unopt_ptr->vetoPostProcessing(true);

			G_MT_EvolutionaryAlgorithmFactory eaFactory(m_configFile);
			std::shared_ptr<GMTEvolutionaryAlgorithm> ea_ptr = eaFactory.get<GMTEvolutionaryAlgorithm>();

			// Add our individual to the algorithm
			ea_ptr->push_back(p_unopt_ptr);

			// Perform the actual (sub-)optimization
			ea_ptr->optimize();

			// Retrieve the best individual
			std::shared_ptr<GParameterSet> p_opt_ptr = ea_ptr->getBestGlobalIndividual<GParameterSet>();

			// Make sure subsequent optimization cycles may generally perform post-optimization again.
			// THis needs to be done on the optimized individual, as it will be loaded into the
			// original individual.
			p_opt_ptr->vetoPostProcessing(false);

			// Load the parameter data into the argument base_type (will also clear the dirty flag)
			p.cannibalize(*p_opt_ptr);

			return true;
		}
			break;

		case execMode::SERIAL:
		{
			// Clone the individual for post-processing
			std::shared_ptr <GParameterSet> p_unopt_ptr = p.template clone<GParameterSet>();

			// Make sure the post-optimization does not trigger post-optimization recursively ...
			p_unopt_ptr->vetoPostProcessing(true);

			G_Serial_EvolutionaryAlgorithmFactory eaFactory(m_configFile);
			std::shared_ptr<GSerialEvolutionaryAlgorithm> ea_ptr = eaFactory.get<GSerialEvolutionaryAlgorithm>();

			// Add our individual to the algorithm
			ea_ptr->push_back(p_unopt_ptr);

			// Perform the actual (sub-)optimization
			ea_ptr->optimize();

			// Retrieve the best individual
			std::shared_ptr<GParameterSet> p_opt_ptr = ea_ptr->getBestGlobalIndividual<GParameterSet>();

			// Make sure subsequent optimization cycles may generally perform post-optimization again.
			// THis needs to be done on the optimized individual, as it will be loaded into the
			// original individual.
			p_opt_ptr->vetoPostProcessing(false);

			// Load the parameter data into the argument base_type (will also clear the dirty flag)
			p.cannibalize(*p_opt_ptr);

			return true;
		}
			break;

		default:
		{
			glogger
				<< "In GEvolutionaryAlgorithmPostOptimizer::raw_processing_: Error!" << std::endl
				<< "Got invalid execution mode " << m_executionMode << std::endl
				<< GEXCEPTION;
		}
			break;
	}
}

/******************************************************************************/
/**
 * The standard constructor. Intentionally private, as it is only needed
 * for de-serialization purposes.
 */
GEvolutionaryAlgorithmPostOptimizer::GEvolutionaryAlgorithmPostOptimizer()
{ /* nothing */ }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
