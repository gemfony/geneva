/**
 * @file GMultiThreadedSwarm.cpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "geneva/GMultiThreadedSwarm.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GMultiThreadedSwarm)

namespace Gem {
namespace Geneva {

/************************************************************************************************************/
/**
 * A standard constructor. No local, dynamically allocated data, hence this function is empty.
 */
GMultiThreadedEA::GMultiThreadedEA()
   : GSwarm()
   , nThreads_(DEFAULTBOOSTTHREADSSWARM)
   , tp_(nThreads_)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * Sets the number of threads for this population. If nThreads is set
 * to 0, an attempt will be made to set the number of threads to the
 * number of hardware threading units (e.g. number of cores or hyperthreading
 * units).
 *
 * @param nThreads The number of threads this class uses
 */
void GMultiThreadedSwarm::setNThreads(const boost::uint8_t& nThreads) {
	if(nThreads == 0) {
		nThreads_ = boost::numeric_cast<boost::uint8_t>(Gem::Common::getNHardwareThreads(DEFAULTBOOSTTHREADSSWARM));
	}
	else {
		nThreads_ = nThreads;
	}

	tp_.size_controller().resize(nThreads_);
}

/************************************************************************************************************/
/**
 * Retrieves the number of threads this population uses.
 *
 * @return The maximum number of allowed threads
 */
uint8_t GMultiThreadedSwarm::getNThreads() const  {
	return nThreads_;
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GMultiThreadedSwarm::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GEvolutionaryAlgorithm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GMultiThreadedSwarm::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GMultiThreadedSwarm::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
