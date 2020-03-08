/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include "geneva/GenevaHelperFunctions.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Sets the DO_PROCESS flag in a given range, the IGNORE flag in the rest of the vector
 *
 * @param workItems The items for which the processing flag should be set
 * @param range A tuple with the half-open range inside of the vector, where the flags should be set
 */
void setProcessingFlag(
	std::vector<std::shared_ptr<GParameterSet>>& workItems
	, const std::tuple<std::size_t, std::size_t>& range
) {
	std::size_t start = std::get<0>(range);
	std::size_t end   = std::get<1>(range);

	for(auto p_it = workItems.begin(); p_it != workItems.begin() + start; ++p_it) {
		if(Gem::Courtier::processingStatus::PROCESSED != (*p_it)->getProcessingStatus()) {
			(*p_it)->set_processing_status(Gem::Courtier::processingStatus::DO_IGNORE);
		}
	}
	for(auto p_it = workItems.begin() + start; p_it != workItems.begin() + end; ++p_it) {
		(*p_it)->set_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);
	}
	for(auto p_it = workItems.begin() + end; p_it != workItems.end(); ++p_it) {
		if(Gem::Courtier::processingStatus::PROCESSED != (*p_it)->getProcessingStatus()) {
			(*p_it)->set_processing_status(Gem::Courtier::processingStatus::DO_IGNORE);
		}
	}
}

/******************************************************************************/
/**
 * Transforms the individual fitness so that the optimization algorithm always
 * "sees" a minimization problem. Optimization algorithms should only use this
 * function to retrieve the fitness of individuals.
 *
 * @param item_ptr The work item for which the fitness should be retrieved
 * @param id The id of the fitness criterion (individuals may have more than one)
 */
double minOnly_transformed_fitness(
	const std::shared_ptr<GParameterSet> &item_ptr
	, std::size_t id
) {
	double f = item_ptr->transformed_fitness(id);
	maxMode m = item_ptr->getMaxMode();

#ifdef DEBUG
	if(not item_ptr) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In minOnly_transformed_fitness():" << std::endl
				<< "Got empty work item" << std::endl
		);
	}
#endif

	if (maxMode::MINIMIZE == m) {
		return f;
	} else { // MAXIMIZE
		// Negation will transform maximization problems into minimization problems
		if (boost::numeric::bounds<double>::highest() == f) {
			return boost::numeric::bounds<double>::lowest();
		} else if (boost::numeric::bounds<double>::lowest() == f) {
			return boost::numeric::bounds<double>::highest();
		} else {
			return -f;
		}
	}
}

/******************************************************************************/
/**
 * Checks whether the first individual is better than the second. The comparison
 * is done with the first (main) fitness criterion.
 */
bool isBetter(
	const std::shared_ptr<GParameterSet> x_ptr
	, const std::shared_ptr<GParameterSet> y_ptr
) {
#ifdef DEBUG
	auto x_mode = x_ptr->getMaxMode();
	auto y_mode = y_ptr->getMaxMode();
	// Cross-check that both work items have the same maxMode
	if(x_mode != y_mode) {
		// Throw an exception
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In isBetterThan(x_ptr, y_ptr):" << std::endl
				<< "Got different maxMode-settings: " << x_mode << " / " << y_mode << std::endl
		);
	}
#endif

	// We assume that both items have the same maxMode and simply compare the "minOnly-Fitness"
	if(minOnly_transformed_fitness(x_ptr) < minOnly_transformed_fitness(y_ptr)) return true;
	else return false;
}

/******************************************************************************/
/**
 * Checks whether the first individual is worse than the second. The comparison
 * is done with the first (main) fitness criterion.
 */
bool isWorse(
	const std::shared_ptr<GParameterSet> x_ptr
	, const std::shared_ptr<GParameterSet> y_ptr
) {
	return not isBetter(x_ptr, y_ptr);
}

/******************************************************************************/
/**
 * Checks whether the first value is better than the second
 */
bool isBetter(
	double x
	, double y
	, maxMode m
) {
	if(maxMode::MAXIMIZE == m) {
		if(x>y) {
			return true;
		} else {
			return false;
		}
	} else { // maxMode::MINIMIZE
		if(x<y) {
			return true;
		} else {
			return false;
		}
	}
}

/******************************************************************************/
/**
 * Checks whether the first value is worse than the second
 */
bool isWorse(
	double x
	, double y
	, maxMode m
) {
	return not isBetter(x,y,m);
}

/******************************************************************************/


} /* namespace Geneva */
} /* namespace Gem */
