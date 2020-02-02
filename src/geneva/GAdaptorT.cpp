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
