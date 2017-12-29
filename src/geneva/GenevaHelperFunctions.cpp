/**
 * @file GenevaHelperFunctions.hpp
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
		(*p_it)->reset_processing_status(Gem::Courtier::processingStatus::IGNORE);
	}
	for(auto p_it = workItems.begin() + start; p_it != workItems.begin() + end; ++p_it) {
		(*p_it)->reset_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);
	}
	for(auto p_it = workItems.begin() + end; p_it != workItems.end(); ++p_it) {
		(*p_it)->reset_processing_status(Gem::Courtier::processingStatus::IGNORE);
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
double minOnly_cached_fitness(
	const std::shared_ptr<GParameterSet>& item_ptr
	, std::size_t id
) {
	double f = item_ptr->transformed_fitness(id);
	maxMode m = item_ptr->getMaxMode();

#ifdef DEBUG
	if(!item_ptr) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In minOnly_cached_fitness():" << std::endl
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
	if(minOnly_cached_fitness(x_ptr) < minOnly_cached_fitness(y_ptr)) return true;
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
	return !isBetter(x_ptr, y_ptr);
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
	return !isBetter(x,y,m);
}

/******************************************************************************/


} /* namespace Geneva */
} /* namespace Gem */
