/**
 * @file GParameterSetFixedSizeSetPriorityQueue.cpp
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

#include "geneva/GParameterSetFixedSizePriorityQueue.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GParameterSetFixedSizePriorityQueue)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the maximum size
 */
GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue(const std::size_t &maxSize)
	: Gem::Common::GFixedSizePriorityQueueT<GParameterSet>(maxSize)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the maximum size and the information, whether higher or
 * lower evaluations are considered better
 */
GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue(
	const std::size_t &maxSize, const bool &higherIsBetter
)
	: Gem::Common::GFixedSizePriorityQueueT<GParameterSet>(maxSize, higherIsBetter)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GParameterSetFixedSizePriorityQueue::GParameterSetFixedSizePriorityQueue(
	const GParameterSetFixedSizePriorityQueue &cp
)
	: Gem::Common::GFixedSizePriorityQueueT<GParameterSet>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GParameterSetFixedSizePriorityQueue::~GParameterSetFixedSizePriorityQueue()
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy the data of another GParameterSetFixedSizePriorityQueue over
 */
 GParameterSetFixedSizePriorityQueue& GParameterSetFixedSizePriorityQueue::operator=(
	const GParameterSetFixedSizePriorityQueue& cp
) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GParameterSetFixedSizePriorityQueue::name() const {
	return std::string("GParameterSetFixedSizePriorityQueue");
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GParameterSetFixedSizePriorityQueue object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GParameterSetFixedSizePriorityQueue::compare(
	const Gem::Common::GFixedSizePriorityQueueT<GParameterSet>&  cp // the other object
	, const Gem::Common::expectation& e // the expectation for this object, e.g. equality
	, const double& limit // the limit for allowed deviations of floating point types
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GParameterSetFixedSizePriorityQueue reference independent of this object and convert the pointer
	const GParameterSetFixedSizePriorityQueue *p_load = Gem::Common::g_convert_and_compare(cp, this);

	GToken token("GBasGParameterSetFixedSizePriorityQueueeEA", e);

	// Compare our parent data ...
	Gem::Common::compare_base<Gem::Common::GFixedSizePriorityQueueT<GParameterSet>>(IDENTITY(*this, *p_load), token);

	// ... no local data

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GParameterSetFixedSizePriorityQueue object, camouflaged as a GFixedSizePriorityQueueT<GParameterSet>
 */
void GParameterSetFixedSizePriorityQueue::load_(const Gem::Common::GFixedSizePriorityQueueT<GParameterSet> *cp) {
	// Check that we are dealing with a GBasePlotter reference independent of this object and convert the pointer
	const GParameterSetFixedSizePriorityQueue *p_load = Gem::Common::g_convert_and_compare(cp, this);

	// Load our parent class'es data ...
	Gem::Common::GFixedSizePriorityQueueT<GParameterSet>::load_(cp);

	// ... no local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
Gem::Common::GFixedSizePriorityQueueT<GParameterSet> * GParameterSetFixedSizePriorityQueue::clone_() const {
	return new GParameterSetFixedSizePriorityQueue(*this);
}

/******************************************************************************/
/**
 * Checks whether no item has the dirty flag set
 */
bool GParameterSetFixedSizePriorityQueue::allClean(std::size_t &pos) const {
	pos = 0;
	std::deque<std::shared_ptr < GParameterSet>> ::const_iterator cit;
	for (cit = m_data.begin(); cit != m_data.end(); ++cit) {
		if (true == (*cit)->isDirty()) {
			pos = std::distance(m_data.begin(), cit);
			return false;
		}
	}

	return true;
}

/******************************************************************************/
/**
 * Emits information about the "dirty flag" of all items
 */
std::string GParameterSetFixedSizePriorityQueue::getCleanStatus() const {
	std::ostringstream oss;
	std::deque<std::shared_ptr < GParameterSet>> ::const_iterator cit;
	for (cit = m_data.begin(); cit != m_data.end(); ++cit) {
		oss << "(" << std::distance(m_data.begin(), cit) << ", " << ((*cit)->isDirty() ? "d" : "c") << ") ";
	}

	return oss.str();
}

/******************************************************************************/
/**
 * Evaluates a single work item, so that it can be sorted. Note that this function
 * will throw in DEBUG mode, if the dirty flag of item is set. Note that the function
 * uses the primary evaluation criterion only.
 */
double GParameterSetFixedSizePriorityQueue::evaluation(
	const std::shared_ptr <GParameterSet> &item
) const {
	return item->minOnly_fitness();
}

/******************************************************************************/
/**
 * Returns a unique id for a work item. This is used to uniquely identify duplicates
 * in the priority queue.
 */
std::string GParameterSetFixedSizePriorityQueue::id(
	const std::shared_ptr <GParameterSet> &item
) const {
	return item->getCurrentEvaluationID();
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
