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
	: GBaseEA()
	, Gem::Courtier::GBrokerConnectorT<GIndividual>()
{ /* nothing */ }

/************************************************************************************************************/
/**
 * The standard copy constructor
 *
 * @param cp A copy of another GBrokerEA object
 */
GBrokerEA::GBrokerEA(const GBrokerEA& cp)
	: GBaseEA(cp)
	, Gem::Courtier::GBrokerConnectorT<GIndividual>(cp)
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
	const GBrokerEA *p_load = gobject_conversion<GBrokerEA>(cp);

	// Load the parent classes' data ...
	GBaseEA::load_(cp);
	Gem::Courtier::GBrokerConnectorT<GIndividual>::load(p_load);

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
    using namespace Gem::Courtier;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBrokerEA *p_load = GObject::gobject_conversion<GBrokerEA>(&cp);

	// Will hold possible deviations from the expectation, including explanations
    std::vector<boost::optional<std::string> > deviations;

	// Check our parent classes' data ...
	deviations.push_back(GBaseEA::checkRelationshipWith(cp, e, limit, "GBrokerEA", y_name, withMessages));
	deviations.push_back(GBrokerConnectorT<GIndividual>::checkRelationshipWith(*p_load, e, limit, "GBrokerEA", y_name, withMessages));

	// ... no local data

	return evaluateDiscrepancies("GBrokerEA", caller, deviations, e);
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

	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::init();

	// We want to confine re-evaluation to defined places. However, we also want to restore
	// the original flags. We thus record the previous setting when setting the flag to true.
	// The function will throw if not all individuals have the same server mode flag.

	// Set the server mode and store the original flag
	bool first = true;
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it){
		if(first){
			serverMode_ = (*it)->getServerMode();
			first = false;
		}

		if(serverMode_ != (*it)->setServerMode(true)) {
			raiseException(
				"In GBrokerEA::init():" << std::endl
				<< "Not all server mode flags have the same value!"
			);
		}
	}
}

/************************************************************************************************************/
/**
 * Performs any necessary finalization work after the end of the optimization cycle
 */
void GBrokerEA::finalize() {
	// Restore the original values
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	for(it=data.begin(); it!=data.end(); ++it) {
		(*it)->setServerMode(serverMode_);
	}

	// GBaseEA sees exactly the environment it would when called from its own class
	GBaseEA::finalize();
}

/************************************************************************************************************/
/**
 * Checks whether this algorithm communicates via the broker. This is an overload from the corresponding
 * GOptimizableI function
 *
 * @return A boolean indicating whether this algorithm communicates via the broker
 */
bool GBrokerEA::usesBroker() const {
	return true;
}

/************************************************************************************************************/
/**
 * We submit individuals  to the broker and wait for processed items. In the first iteration, in the case of
 * the MUPLUSNU_SINGLEEVAL sorting strategy, also the fitness of the parents is calculated. The type of
 * command intended to be executed on the individuals is stored in the individual.
 */
void GBrokerEA::adaptChildren() {
	using namespace Gem::Courtier;

	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	std::size_t np = getNParents(), nc=data.size()-np;
	boost::uint32_t iteration=getIteration();

	//--------------------------------------------------------------------------------
	// Start by marking the work to be done in the individuals.
	// "range" will hold the start- and end-points of the range
	// to be worked on
	boost::tuple<std::size_t, std::size_t> range = markCommands();

	//--------------------------------------------------------------------------------
	// Now submit work items and wait for results.
	Gem::Courtier::GBrokerConnectorT<GIndividual>::workOn(
			data
			, range.get<0>()
			, range.get<1>()
			, ACCEPTOLDERITEMS
	);

	//--------------------------------------------------------------------------------
	// Now fix the population
	fixAfterJobSubmission();
}

/************************************************************************************************************/
/**
 * Mark the commands each individual has to work on.
 *
 * @return A boost::tuple holding the start- and end-points for the job submission
 */
boost::tuple<std::size_t, std::size_t> GBrokerEA::markCommands() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	std::size_t np = getNParents(), nc=data.size()-np;
	boost::uint32_t iteration=getIteration();

	std::size_t start = np; // Where the evaluation starts
	std::size_t end = data.size(); // Where the evaluation ends

	//--------------------------------------------------------------------------------
	// Start by marking the work to be done

	// In the first iteration, depending on the selection mode, parents need to be evaluated as well
	if(inFirstIteration()) {
		switch(getSortingScheme()) {
		case SA_SINGLEEVAL:
		case MUPLUSNU_SINGLEEVAL:
		case MUPLUSNU_PARETO:
		case MUCOMMANU_PARETO: // The current setup will still allow some old parents to become new parents
		case MUNU1PRETAIN_SINGLEEVAL: // same procedure. We do not know which parent is best
			start = 0; // We want to evaluate parents as well

			// Note that we only have parents left in this iteration
			for(it=data.begin(); it!=data.begin() + np; ++it) {
				(*it)->getPersonalityTraits()->setCommand("evaluate");
			}
			break;

		case MUCOMMANU_SINGLEEVAL:
			break;
		}
	}

	// Mark children. This is the same for all sorting modes. The "adapt" command
	// comprises both mutation and evaluation
	for(it=data.begin() + np; it!=data.end(); ++it) {
		(*it)->getPersonalityTraits()->setCommand("adapt");
	}

	return boost::make_tuple<std::size_t, std::size_t>(start, end);
}

/************************************************************************************************************/
/**
 * Fixes the population after a job submission
 */
void GBrokerEA::fixAfterJobSubmission() {
	std::vector<boost::shared_ptr<GIndividual> >::iterator it;
	std::size_t np = getNParents(), nc=data.size()-np;
	boost::uint32_t iteration=getIteration();

	// Remove parents from older iterations -- we do not want them. Note that "remove_if"
	// simply moves items not satisfying the predicate to the end of the list. We thus need
	// to explicitly erase these items. remove_if returns the iterator position right after
	// the last item not satisfying the predicate.
	data.erase(std::remove_if(data.begin(), data.end(), isOldParent(iteration)), data.end());

	// Make it known to remaining old individuals that they are now part of a new iteration
	std::for_each(data.begin(), data.end(), boost::bind(&GIndividual::setAssignedIteration, _1, iteration));

	// Make sure that parents are at the beginning of the array.
	sort(data.begin(), data.end(), indParentComp());

	// Add missing individuals, as clones of the last item
	if(data.size() < getDefaultPopulationSize()){
		std::size_t fixSize= getDefaultPopulationSize() - data.size();
		for(std::size_t i=0; i<fixSize; i++){
			// This function will create a clone of its argument
			this->push_back_clone(data.back());
		}
	}

	// Mark the first nParents_ individuals as parents in the first iteration. We want to have a "sane" population.
	if(inFirstIteration()){
		for(it=this->begin(); it!=this->begin() + np; ++it) {
			(*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsParent();
		}
	}

	// We care for too many returned individuals in the select() function. Older
	// individuals might nevertheless have a better quality. We do not want to loose them.
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
	GBaseEA::select();

	////////////////////////////////////////////////////////////
	// At this point we have a sorted list of individuals and can take care of
	// too many members, so the next iteration finds a "standard" population. This
	// function will remove the last items.
	data.resize(getNParents() + getDefaultNChildren());

	// Everything should be back to normal ...
}

/************************************************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GBrokerEA::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
	, const bool& showOrigin
) {
	// Call our parent class'es function
	GBaseEA::addConfigurationOptions(gpb, showOrigin);
	Gem::Courtier::GBrokerConnectorT<GIndividual>::addConfigurationOptions(gpb, showOrigin);

	// no local data
}

#ifdef GEM_TESTING
/************************************************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBrokerEA::modify_GUnitTests() {
	bool result = false;

	// Call the parent class'es function
	if(GBaseEA::modify_GUnitTests()) result = true;

	return result;
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBrokerEA::specificTestsNoFailureExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseEA::specificTestsNoFailureExpected_GUnitTests();
}

/************************************************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBrokerEA::specificTestsFailuresExpected_GUnitTests() {
	// Call the parent class'es function
	GBaseEA::specificTestsFailuresExpected_GUnitTests();
}

/************************************************************************************************************/

#endif /* GEM_TESTING */

} /* namespace Geneva */
} /* namespace Gem */
