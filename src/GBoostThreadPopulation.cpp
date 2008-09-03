/**
 * @file GBoostThreadPopulation.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "GBoostThreadPopulation.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoostThreadPopulation)

namespace Gem {
namespace GenEvA {

/********************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data,
 * hence this function is empty.
 */
GBoostThreadPopulation::GBoostThreadPopulation() :
	GBasePopulation(),
	nThreads_(DEFAULTBOOSTTHREADS),
	tp_(nThreads_)
{ /* nothing */ }

/********************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Reference to another GBoostThreadPopulation object
 */
GBoostThreadPopulation::GBoostThreadPopulation(const GBoostThreadPopulation& cp) :
	GBasePopulation(cp),
	nThreads_(cp.nThreads_),
	tp_(nThreads_)
{ /* nothing */ }

/********************************************************************/
/**
 * The standard destructor. We clear remaining work items in the
 * thread pool and wait for active tasks to finish.
 */
GBoostThreadPopulation::~GBoostThreadPopulation() {
	tp_.clear();
	tp_.wait();
}

/********************************************************************/
/**
 * A standard assignment operator for GBoostThreadPopulation objects
 *
 * @param cp Reference to another GBoostThreadPopulation object
 * @return A constant reference to this object
 */
const GBoostThreadPopulation& GBoostThreadPopulation::operator=(const GBoostThreadPopulation& cp) {
	GBoostThreadPopulation::load(&cp);
	return *this;
}

/********************************************************************/
/**
 * Loads the data from another GBoostThreadPopulation object.
 *
 * @param vp Pointer to another GBoostThreadPopulation object, camouflaged as a GObject
 */
void GBoostThreadPopulation::load(const GObject *cp) {
	// Convert GObject pointer to local format
	const GBoostThreadPopulation *gbp = this->conversion_cast(cp, this);

	// First load our parent class'es data ...
	GBasePopulation::load(cp);

	// ... and then our own
	nThreads_ = gbp->nThreads_;
	tp_.clear();
	tp_.size_controller().resize(nThreads_);
}

/********************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GBoostThreadPopulation::clone() {
	return new GBoostThreadPopulation(*this);
}

/********************************************************************/
/**
 * We want to do all fitness calculation in the threads. Hence lazy
 * evaluation is not allowed.
 */
void GBoostThreadPopulation::optimize() {
	std::vector<bool> le_value;

	std::vector<bool>::iterator b_it;
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	// We want to prevent lazy evaluation, as all value calculation
	// shall take place in the threads. Simultaneously we want to be
	// able to restore the original values.
	for(it=data.begin(); it!=data.end(); ++it){
		le_value.push_back((*it)->setAllowLazyEvaluation(false));
	}

	// Do the actual optimization. Thus function will use our overloaded
	// mutateChildren class.
	GBasePopulation::optimize();

	// Restore the original values
	for(it=data.begin(), b_it=le_value.begin();
		it!=data.end(), b_it != le_value.end();
		++it, ++b_it){
		(*it)->setAllowLazyEvaluation(*b_it);
	}
}

/********************************************************************/
/**
 * An overloaded version of GBasePopulation::mutateChildren() . Mutation
 * and evaluation of children is handled by threads in a thread pool. The maximum
 * number of threads is DEFAULTBOOSTTHREADS (possibly 2) and can be overridden
 * with the GBoostThreadPopulation::setMaxThreads() function.
 */
void GBoostThreadPopulation::mutateChildren() {
	std::size_t nParents = GBasePopulation::getNParents();
	boost::uint32_t generation = GBasePopulation::getGeneration();
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;

	// We start with the parents, if this is generation 0. Their
	// initial fitness needs to be determined, if this is the MUPLUSNU
	// selection model.
	if(generation==0 && (this->getSortingScheme()==MUPLUSNU)) {
		for(it=data.begin(); it!=data.begin() + nParents; ++it) {
			tp_.schedule(boost::bind(&GIndividual::checkedFitness, it->get()));
		}
	}

	// Next we mutate the children
	for(it=data.begin() + nParents; it!=data.end(); ++it) {
		tp_.schedule(boost::bind(&GIndividual::checkedMutate, it->get()));
	}

	// ... and wait for the pool to become empty
	tp_.wait();
}

/********************************************************************/
/**
 * Sets the number of threads for this population. Standard
 * value is DEFAULTBOOSTTHREADS.
 *
 * @param nThreads The number of threads this class uses
 */
void GBoostThreadPopulation::setNThreads(boost::uint8_t nThreads) {
	nThreads_ = nThreads;
	tp_.size_controller().resize(nThreads_);
}

/********************************************************************/
/**
 * Retrieves the number of threads this population uses.
 *
 * @return The maximum number of allowed threads
 */
uint8_t GBoostThreadPopulation::getNThreads() const throw() {
	return nThreads_;
}

/********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
