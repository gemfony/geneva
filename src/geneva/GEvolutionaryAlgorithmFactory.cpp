/**
 * @file GEvolutionaryAlgorithmFactory.cpp
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

#include "geneva/GEvolutionaryAlgorithmFactory.hpp"

namespace Gem {
namespace Geneva {

/*******************************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 */
GEvolutionaryAlgorithmFactory::GEvolutionaryAlgorithmFactory(
	const std::string& configFile
	, const parMode& pm
)
	: GFactoryT(configFile)
	, pm_(pm)
	, nEvaluationThreads_(FACT_DEF_NEVALUATIONTHREADS)
{ /* nothing */ }

/*******************************************************************************************/
/**
 * The destructor
 */
GEvolutionaryAlgorithmFactory::~GEvolutionaryAlgorithmFactory()
{ /* nothing */ }

/*******************************************************************************************/
/**
 * Allows to describe specific configuration options, as assigned to this object
 */
void GEvolutionaryAlgorithmFactory::describeLocalOptions_(
	Gem::Common::GParserBuilder& gpb
) {
	using namespace Gem::Courtier;

	std::string comment;

	comment = "";
	comment += "Determines the number of threads simultaneously running;";
	comment += "evaluations in multi-threaded mode. 0 means \"automatic\";";
	gpb.registerFileParameter<boost::uint16_t>(
		"nEvaluationThreads"
		, nEvaluationThreads_
		, FACT_DEF_NEVALUATIONTHREADS
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);


	// add local data
	comment = ""; // Reset the comment string
	comment += "The timeout for the retrieval of an;";
	comment += "iteration's first timeout;";
	gpb.registerFileParameter<boost::posix_time::time_duration>(
		"firstTimeOut" // The name of the variable
		, firstTimeOut_
		, boost::posix_time::duration_from_string(DEFAULTBROKERFIRSTTIMEOUT) // The default value
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = ""; // Reset the first comment string
	comment += "The lower boundary for the adaption;";
	comment += "of the waitFactor variable;";
	gpb.registerFileParameter<double>(
		"minWaitFactor" // The name of the first variable
		, minWaitFactor_
		, DEFAULTMINBROKERWAITFACTOR // The first default value
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = ""; // Reset the second comment string
	comment += "The upper boundary for the adaption;";
	comment += "of the waitFactor variable;";
	gpb.registerFileParameter<double>(
		"maxWaitFactor" // The name of the second variable
		, maxWaitFactor_
		, DEFAULTMAXBROKERWAITFACTOR // The second default value
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment = ""; // Reset the comment string
	comment += "Activates (1) or de-activates (0) logging;";
	comment += "iteration's first timeout;";
	gpb.registerFileParameter<bool>(
		"doLogging" // The name of the variable
		, doLogging_
		, false // The default value
		, Gem::Common::VAR_IS_SECONDARY
		, comment
	);

	comment = ""; // Reset the comment string
	comment += "Indicates that the broker connector should wait endlessly;";
	comment += "for further arrivals of individuals in an iteration;";
	gpb.registerFileParameter<bool>(
		"boundlessWait" // The name of the variable
		, boundlessWait_
		, false // The default value
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);

	comment =  ""; // 	Reset the comment string
	comment += "Specifies the amount by which the wait factor gets;";
	comment += "incremented or decremented during automatic adaption;";
	gpb.registerFileParameter<double>(
		"waitFactorIncrement" // The name of the variable
		, waitFactorIncrement_
		, DEFAULTBROKERWAITFACTORINCREMENT // The default value
		, Gem::Common::VAR_IS_ESSENTIAL
		, comment
	);
}

/*******************************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
boost::shared_ptr<GBaseEA> GEvolutionaryAlgorithmFactory::getObject_(
	Gem::Common::GParserBuilder& gpb
	, const std::size_t& id
) {
	// Will hold the result
	boost::shared_ptr<GBaseEA> target;

	// Fill the target pointer as required
	switch(pm_) {
	case PARMODE_SERIAL:
		target = boost::shared_ptr<GSerialEA>(new GSerialEA());
		break;

	case PARMODE_MULTITHREADED:
		target = boost::shared_ptr<GMultiThreadedEA>(new GMultiThreadedEA());
		break;

	case PARMODE_BROKERAGE:
		target = boost::shared_ptr<GBrokerEA>(new GBrokerEA());
		break;
	}

	// Make the local configuration options known
	target->addConfigurationOptions(gpb);

	return target;
}

/*******************************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object.
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GEvolutionaryAlgorithmFactory::postProcess_(boost::shared_ptr<GBaseEA>& p_base) {
	// Convert the object to the correct target type
	switch(pm_) {
	case PARMODE_SERIAL:
		// nothing
		break;

	case PARMODE_MULTITHREADED:
		{
			boost::shared_ptr<GMultiThreadedEA> p = boost::dynamic_pointer_cast<GMultiThreadedEA>(p_base);
			p->setNThreads(nEvaluationThreads_);
		}
		break;

	case PARMODE_BROKERAGE:
		{
			boost::shared_ptr<GBrokerEA> p = boost::dynamic_pointer_cast<GBrokerEA>(p_base);

			p->setFirstTimeOut(firstTimeOut_);
			p->setWaitFactorExtremes(minWaitFactor_, maxWaitFactor_);
			p->doLogging(doLogging_);
			p->setBoundlessWait(boundlessWait_);
			p->setWaitFactorIncrement(waitFactorIncrement_);
		}
		break;
	}
}

/*******************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
