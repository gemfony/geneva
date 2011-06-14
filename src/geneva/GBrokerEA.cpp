/**
 * @file GBrokerEA.cpp
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

#include "geneva/GBrokerEA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBrokerEA)

namespace Gem
{
namespace Geneva
{

/************************************************************************************************************/
/**
 * The default constructor
 */
GBrokerEA::GBrokerEA()
	: GEvolutionaryAlgorithm()
	, GBrokerConnector()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerEA object
 */
GBrokerEA::GBrokerEA(const GBrokerEA& cp)
	: GEvolutionaryAlgorithm(cp)
	, GBrokerConnector(cp)
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard destructor. We have no object-wide dynamically allocated data, hence
 * this function is empty.
 */
GBrokerEA::~GBrokerEA()
{ /* nothing */}

/************************************************************************************************************/
/**
 * A standard assignment operator for GBrokerEA objects,
 *
 * @param cp A copy of another GBrokerEA object
 * @return A constant reference to this object
 */
const GBrokerEA& GBrokerEA::operator=(const GBrokerEA& cp) {
	GBrokerEA::load_(&cp);
	return *this;
}

/************************************************************************************************************/
/**
 * Loads the data of another GBrokerEA object, camouflaged as a
 * pointer to a GObject
 *
 * @param cp A pointer to another GBrokerEA object, camouflaged as a GObject
 */
void GBrokerEA::load_(const GObject * cp) {
	const GBrokerEA *p_load = conversion_cast<GBrokerEA>(cp);

	// Load the parent classes' data ...
	GEvolutionaryAlgorithm::load_(cp);
	GBrokerConnector::load(p_load);

	// no local data
}

/************************************************************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
GObject *GBrokerEA::clone_() const {
	return new GBrokerEA(*this);
}

/************************************************************************************************************/
/**
 * Checks for equality with another GBrokerEA object
 *
 * @param  cp A constant reference to another GBrokerEA object
 * @return A boolean indicating whether both objects are equal
 */
bool GBrokerEA::operator==(const GBrokerEA& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBrokerEA::operator==","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks for inequality with another GBrokerEA object
 *
 * @param  cp A constant reference to another GBrokerEA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBrokerEA::operator!=(const GBrokerEA& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBrokerEA::operator!=","cp", CE_SILENT);
}

/************************************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another object, camouflaged as a GObject
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GBrokerEA::checkRelationshipWith(
		const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages
) const {
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerEA *p_load = GObject::conversion_cast<GBrokerEA>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GEvolutionaryAlgorithm::checkRelationshipWith(cp, e, limit, "GBrokerEA", y_name, withMessages));
	deviations.push_back(GBrokerConnector::checkRelationshipWith(*p_load, e, limit, "GBrokerEA", y_name, withMessages));

	// no local data ...
	return evaluateDiscrepancies("GBrokerEA", caller, deviations, e);
}

/************************************************************************************************************/
/**
 * The actual business logic to be performed during each iteration.
 *
 * @return The best achieved fitness
 */
double GBrokerEA::cycleLogic() {
	// Let the connector know how many processable items are available
	GBrokerConnector::setBCNProcessableItems(GEvolutionaryAlgorithm::getNProcessableItems());

	// Let the GBrokerConnector know that we are starting a new iteration
	GBrokerConnector::markNewIteration();

	// Start the actual optimization cycle
	return GEvolutionaryAlgorithm::cycleLogic();
}

/************************************************************************************************************/
/**
 * Performs any necessary initialization work before the start of the optimization cycle
 */
void GBrokerEA::init() {
	// Prevent usage of this population inside another broker population - check type of individuals
	{
		std::vector<boost::shared_ptr<GIndividual> >::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			boost::shared_ptr<GBrokerEA> p = boost::dynamic_pointer_cast<GBrokerEA>(*it);

			if(p) {
				// Conversion was successful - this should not be, as there are not to supposed to be
				// any GBrokerEA objects inside itself.
				raiseException(
						"In GBrokerEA::optimize(): Error" << std::endl
						<< "GBrokerEA stored as an individual inside of" << std::endl
						<< "a population of the same type"
				);
			}
		}
	}

	// GEvolutionaryAlgorithm sees exactly the environment it would when called from its own class
	GEvolutionaryAlgorithm::init();

	// Connect to the broker
	GBrokerConnector::init();

	// We want to confine re-evaluation to defined places. However, we also want to restore
	// the original flags. We thus record the previous setting when setting the flag to true.
	sm_value_.clear(); // Make sure we do not have "left-overs"
	// Set the server mode and store the original flag
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		sm_value_.push_back((*it)->setServerMode(true));
	}
}

/************************************************************************************************************/
/**
 * Performs any necessary finalization work after the end of the optimization cycle
 */
void GBrokerEA::finalize() {
#ifdef DEBUG
	if(data.size() != sm_value_.size()) {
		raiseException(
				"In GBrokerEA::finalize():" << std::endl
				<< "Invalid number of serverMode flags: " << data.size() << "/" << sm_value_.size()
		);
	}
#endif /* DEBUG */

	// Restore the original values
	std::vector<bool>::iterator b_it;
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(), b_it=sm_value_.begin(); it!=data.end(); ++it, ++b_it) {
		(*it)->setServerMode(*b_it);
	}
	sm_value_.clear(); // Make sure we have no "left-overs"

	// Invalidate our local broker connection
	GBrokerConnector::finalize();

	// GEvolutionaryAlgorithm sees exactly the environment it would when called from its own class
	GEvolutionaryAlgorithm::finalize();
}

/************************************************************************************************************/
/**
 * Starting from the end of the children's list, we submit individuals  to the
 * broker. In the first generation, in the case of the MUPLUSNU_SINGLEEVAL sorting strategy,
 * also the fitness of the parents is calculated. The type of command intended to
 * be executed on the individuals is stored in the individual. The function
 * then waits for the first individual to come back. The time frame for all other
 * individuals to come back is a multiple of this time frame.
 */
void GBrokerEA::adaptChildren() {
	using namespace boost::posix_time;

	std::vector<boost::shared_ptr<GIndividual> >::reverse_iterator rit;
	std::size_t np = getNParents(), nc=data.size()-np;
	boost::uint32_t iteration=getIteration();

	//--------------------------------------------------------------------------------
	// First we send all individuals abroad

	// Start with the children from the back of the population
	// This is the same for all sorting modes
	for(rit=data.rbegin(); rit!=data.rbegin()+nc; ++rit) {
		(*rit)->getPersonalityTraits()->setCommand("adapt");
		GBrokerConnector::submit(*rit);
	}

	// We can remove children, so only parents remain in the population
	data.resize(np);

	// Make sure we also evaluate the parents in the first iteration, if needed.
	// This is only applicable to the SA, MUPLUSNU_SINGLEEVAL and MUNU1PRETAIN modes.
	if(iteration==0) {
		switch(getSortingScheme()) {
		//--------------------------------------------------------------
		case SA:
		case MUPLUSNU_SINGLEEVAL:
		case MUPLUSNU_PARETO:
		case MUCOMMANU_PARETO: // The current setup will still allow some old parents to become new parents
		case MUNU1PRETAIN: // same procedure. We do not know which parent is best
			// Note that we only have parents left in this iteration
			for(rit=data.rbegin(); rit!=data.rend(); ++rit) {
				(*rit)->getPersonalityTraits()->setCommand("evaluate");
				GBrokerConnector::submit(*rit);
			}

			// Next we clear the population. We do not loose anything,
			// as at least one shared_ptr to our individuals remains
			data.clear();
			break;

		case MUCOMMANU_SINGLEEVAL:
			break; // nothing
		}
		//--------------------------------------------------------------
		// If we are running in SA, MUPLUSNU_SINGLEEVAL or MUNU1PRETAIN mode, we now have an empty population,
		// as parents have been sent away for evaluation. If this is the MUCOMMANU_SINGLEEVAL mode, parents
		// do not participate in the sorting and can be ignored.
	}

	//--------------------------------------------------------------------------------
	// We can now wait for the individuals to return from their journey.
	std::size_t nReceivedParent       = 0;
	std::size_t nReceivedChildCurrent = 0;
	std::size_t nReceivedChildOlder   = 0;

	// Will hold returned items
	boost::shared_ptr<GIndividual> p;

	// First wait for the first individual of the current iteration to arrive.
	// Individuals from older iterations will also be accepted in this loop,
	// unless they are parents. Note that we can thus have a situation where less
	// genuine parents are in the population than have originally been sent away.
	while(true) {
		// Note: the following call will throw if a timeout has been reached.
		p = GBrokerConnector::retrieveFirstItem<GIndividual>();

		// If it is from the current iteration, break the loop, otherwise
		// continue until the first item of the current iteration has been
		// received.
		if(p->getParentAlgIteration() == iteration) {
			// Add the individual to our list.
			this->push_back(p);

			// Give the GBrokerConnector the opportunity to perform logging
			GBrokerConnector::log();

			// Update the counter.
			if(p->getPersonalityTraits<GEAPersonalityTraits>()->isParent()) nReceivedParent++;
			else nReceivedChildCurrent++;

			break;
		} else {
			if(!p->getPersonalityTraits<GEAPersonalityTraits>()->isParent()){
				// Make it known to the individual that it is now part of a new iteration
				p->setParentAlgIteration(iteration);

				// Add the individual to our list.
				this->push_back(p);

				// Update the counter
				nReceivedChildOlder++;
			}
			else { // We do not accept parents from older iterations
				p.reset();
			}
		}
	}

	//--------------------------------------------------------------------------------
	// Wait for further arrivals until the population is complete or a timeout has been reached.
	bool complete=false;

	// retrieveItem will return an empty pointer, if a timeout has been reached
	while(!complete && (p=GBrokerConnector::retrieveItem<GIndividual>())) {
		// Count the number of items received.
		if(p->getParentAlgIteration() == iteration) { // First count items of the current iteration
			// Add the individual to our list.
			this->push_back(p);

			// Give the GBrokerConnector the opportunity to perform logging
			GBrokerConnector::log();

			// Update the counter
			if(p->getPersonalityTraits<GEAPersonalityTraits>()->isParent()) nReceivedParent++;
			else nReceivedChildCurrent++;
		} else { // Now count items of older iterations
			if(!p->getPersonalityTraits<GEAPersonalityTraits>()->isParent()){  // Parents from older iterations will be ignored, as there is no else clause
				// Make it known to the individual that it is now part of a new iteration
				p->setParentAlgIteration(iteration);

				// Add the individual to our list.
				this->push_back(p);

				// Update the counter
				nReceivedChildOlder++;
			}
		}

		// Mark as complete, if a full set of children (and parents in iteration 0 / MUPLUSNU_SINGLEEVAL / MUNU1PRETAIN)
		// of the current iteration has returned. Older individuals may return in the next iterations, unless
		// they are parents.
		if(iteration == 0 && (getSortingScheme()==SA || getSortingScheme()==MUPLUSNU_SINGLEEVAL || getSortingScheme()==MUNU1PRETAIN)) {
			if(nReceivedParent+nReceivedChildCurrent==np+getDefaultNChildren()) {
				complete=true;
			}
		} else {
			if(nReceivedChildCurrent>=getDefaultNChildren()) { // Note the >=
				complete=true;
			}
		}
	}

	// If parents have been evaluated, make sure they are at the beginning of the array.
	if(iteration==0 && (getSortingScheme()==SA || getSortingScheme()==MUPLUSNU_SINGLEEVAL || getSortingScheme()==MUNU1PRETAIN)){
		// Have any individuals returned at all ?
		if(data.size()==0) { // No way out ...
			raiseException(
					"In GBrokerEA::adaptChildren() :" << std::endl
					<< "Population is empty when it shouldn't be."
			);
		}

		// Sort according to parent/child tag. We do not know in what order individuals have returned.
		// Hence we need to sort them. Parents need to be in front, children in the back.
		sort(data.begin(), data.end(), indParentComp());
	}

	//--------------------------------------------------------------------------------
	// We are done, if a full set of individuals has returned.
	// The population size is at least at nominal values.
	if(complete) return;

	//--------------------------------------------------------------------------------
	// O.k., so we are missing individuals from the current population.
	// Do some fixing, if necessary and let the audience know in DEBUG mode.

#ifdef DEBUG
	std::ostringstream information;
	information << "Note that in GBrokerEA::adaptChildren()" << std::endl
				<< "some individuals of the current population did not return" << std::endl
				<< "in iteration " << iteration << "." << std::endl;

	if(iteration==0 && (getSortingScheme()==SA || getSortingScheme()==MUPLUSNU_SINGLEEVAL || getSortingScheme()==MUNU1PRETAIN)){
		information << "We have received " << nReceivedParent << " parents." << std::endl
			        << "where " << np << " parents were expected." << std::endl;
	}

	information << nReceivedChildCurrent << " children of the current population returned" << std::endl
			    << "plus " << nReceivedChildOlder << " older children," << std::endl
			    << "where the default number of children is " << getDefaultNChildren() << "." << std::endl
			    << "The current size of the population is now " << this->size() << std::endl
			    << "where a minimal size of " << getDefaultPopulationSize() << " is needed." << std::endl;
#endif /* DEBUG*/


	if(data.size() < np+getDefaultNChildren()){
		std::size_t fixSize=np+getDefaultNChildren() - data.size();

#ifdef DEBUG
		information << fixSize << " individuals thus need to be added to the population." << std::endl
					<< "Note that children may still return in later iterations." << std::endl;
#endif /* DEBUG */

		for(std::size_t i=0; i<fixSize; i++){
			// This function will create a clone of its argument
			this->push_back_clone(data.back());
		}
	}

#ifdef DEBUG
	std::cout << information.str();
#endif /* DEBUG */

	// Mark the first nParents_ individuals as parents, if they aren't parents yet. We want
	// to have a "sane" population.
	if(iteration==0 && (getSortingScheme()==SA || getSortingScheme()==MUPLUSNU_SINGLEEVAL || getSortingScheme()==MUNU1PRETAIN)){
		GEvolutionaryAlgorithm::iterator it;
		for(it=this->begin(); it!=this->begin() + getNParents(); ++it) {
			if(!(*it)->getPersonalityTraits<GEAPersonalityTraits>()->isParent()) {
				(*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsParent();
			}
		}
	}

	// We care for too many returned individuals in the select() function. Older
	// individuals might nevertheless have a better quality. We do not want to loose them.

	// We might theoretically at this point have a population that (in iteration 0 / MUPLUSNU_SINGLEEVAL / MUNU1PRETAIN)
	// consists of only parents. This is not a problem, as the entire population will get sorted
	// in this case, and new parents and children will be tagged after the select function.
}

/************************************************************************************************************/
/**
 * We will at this point have a population with at least the default number
 * of individuals. More individuals are allowed. the population will be
 * resized to nominal values at the end of this function.
 */
void GBrokerEA::select() {
	////////////////////////////////////////////////////////////
	// Great - we are at least at the default level and are
	// ready to call the actual select() function. This will
	// automatically take care of the selection modes.
	GEvolutionaryAlgorithm::select();

	////////////////////////////////////////////////////////////
	// At this point we have a sorted list of individuals and can take care of
	// too many members, so the next iteration finds a "standard" population. This
	// function will remove the last items.
	data.resize(getNParents() + getDefaultNChildren());

	// Everything should be back to normal ...
}

#ifdef GENEVATESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerEA::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GEvolutionaryAlgorithm::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerEA::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GEvolutionaryAlgorithm::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerEA::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GEvolutionaryAlgorithm::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
