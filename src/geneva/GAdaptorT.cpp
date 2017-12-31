/**
 * @file GAdaptorT.cpp
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

#include "geneva/GAdaptorT.hpp"


namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * Common interface for all adaptors to the adaption functionality. Specialization
 * for the T==bool case . Note that
 *
 * @param valVec A vector of values that need to be adapted
 * @param range A typical value range for type T
 * @return The number of adaptions that were carried out
 */
template <>
std::size_t GAdaptorT<bool,double>::adapt(
	std::vector<bool>& valVec
	, const bool& range
	, Gem::Hap::GRandomBase& gr
) {
	using namespace Gem::Common;
	using namespace Gem::Hap;

	std::size_t nAdapted = 0;

	// Update the adaption probability, if requested by the user
	if(m_adaptAdProb > double(0.)) {
		m_adProb *= gexp(
			m_normal_distribution(gr, typename std::normal_distribution<double>::param_type(0.,m_adaptAdProb))
		);
		Gem::Common::enforceRangeConstraint<double>(
			m_adProb
			, m_minAdProb
			, m_maxAdProb
			, "GAdaptorT<bool,double>::adapt()"
		);
	}

	bool dummy_val;

	if(adaptionMode::WITHPROBABILITY == m_adaptionMode) { // The most likely case is indeterminate (means: "depends")
		for (auto && val: valVec) {
			// A likelihood of m_adProb for adaption
			if(m_weighted_bool(gr, std::bernoulli_distribution::param_type(gfabs(m_adProb)))) {
				dummy_val = val;
				adaptAdaption(range, gr);
				customAdaptions(dummy_val, range, gr); // does not know about the bool-proxy of std::vector<bool>
				val=dummy_val;
				nAdapted += 1;
			}
		}
	} else if(adaptionMode::ALWAYS == m_adaptionMode) { // always adapt
		for (auto && val: valVec) {
			dummy_val = val;
			adaptAdaption(range, gr);
			customAdaptions(dummy_val, range, gr);
			val=dummy_val;
			nAdapted += 1;
		}
	}

	// No need to test for "m_adaptionMode == adaptionMode::NEVER" as no action is needed in this case

	return nAdapted;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
