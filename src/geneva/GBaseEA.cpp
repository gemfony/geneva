/**
 * @file GBaseEA.cpp
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
#include "geneva/GBaseEA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBaseEA::GEAOptimizationMonitor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
G_API_GENEVA const std::string GBaseEA::nickname = "ea";

/******************************************************************************/
/**
 * The default constructor, As we do not have any individuals yet, we set the population
 * size, and number of parents to 0. It is the philosophy of this class not
 * to provide constructors for each and every use case. Instead, you should set
 * vital parameters, such as the population size or the parent individuals by hand.
 */
GBaseEA::GBaseEA()
	: GParameterSetParChild()
	, smode_(DEFAULTSMODE)
{
	// Register the default optimization monitor
	this->registerOptimizationMonitor(
			boost::shared_ptr<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(
					new GEAOptimizationMonitor()
			)
	);

	// Make sure we start with a valid population size if the user does not supply these values
	this->setPopulationSizes(100,1);
}

/******************************************************************************/
/**
 * A standard copy constructor. Note that the generation number is reset to 0 and
 * is not copied from the other object. We assume that a new optimization run will
 * be started.
 *
 * @param cp Another GBaseEA object
 */
GBaseEA::GBaseEA(const GBaseEA& cp)
	: GParameterSetParChild(cp)
	, smode_(cp.smode_)
{
	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/******************************************************************************/
/**
 * The standard destructor. All work is done in the parent class.
 */
GBaseEA::~GBaseEA()
{ /* nothing */ }

/******************************************************************************/
/**
 * Loads the data of another GBaseEA object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GBaseEA object, camouflaged as a GObject
 */
void GBaseEA::load_(const GObject * cp)
{
	const GBaseEA *p_load = gobject_conversion<GBaseEA>(cp);

	// First load the parent class'es data ...
	GParameterSetParChild::load_(cp);

	// ... and then our own data
	smode_ = p_load->smode_;
}

/******************************************************************************/
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
boost::optional<std::string> GBaseEA::checkRelationshipWith(const GObject& cp,
		const Gem::Common::expectation& e,
		const double& limit,
		const std::string& caller,
		const std::string& y_name,
		const bool& withMessages) const
{
    using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBaseEA *p_load = GObject::gobject_conversion<GBaseEA>(&cp);

	// Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GParameterSetParChild::checkRelationshipWith(cp, e, limit, "GBaseEA", y_name, withMessages));

	// ... and then our local data
	deviations.push_back(checkExpectation(withMessages, "GBaseEA", smode_, p_load->smode_, "smode_", "p_load->smode_", e , limit));

	return evaluateDiscrepancies("GBaseEA", caller, deviations, e);
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBaseEA::name() const {
   return std::string("GBaseEA");
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GBaseEA::getOptimizationAlgorithm() const {
	return "PERSONALITY_EA";
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GBaseEA::getAlgorithmName() const {
	return std::string("Evolutionary Algorithm");
}

/******************************************************************************/
/**
 * The function checks that the population size meets the requirements and does some
 * tagging. It is called from within GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::optimize(), before the
 * actual optimization cycle starts.
 */
void GBaseEA::init() {
	// To be performed before any other action
	GParameterSetParChild::init();
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GBaseEA::finalize() {
	// Last action
   GParameterSetParChild::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
boost::shared_ptr<GPersonalityTraits> GBaseEA::getPersonalityTraits() const {
   return boost::shared_ptr<GEAPersonalityTraits>(new GEAPersonalityTraits());
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GBaseEA::addConfigurationOptions (
	Gem::Common::GParserBuilder& gpb
) {
	// Call our parent class'es function
	GParameterSetParChild::addConfigurationOptions(gpb);

	gpb.registerFileParameter<sortingMode>(
		"sortingMethod" // The name of the variable
		, DEFAULTSMODE // The default value
		, boost::bind(
			&GBaseEA::setSortingScheme
			, this
			, _1
		  )
	)
	<< "The sorting scheme. Options" << std::endl
	<< "0: MUPLUSNU mode with a single evaluation criterion" << std::endl
	<< "1: MUCOMMANU mode with a single evaluation criterion" << std::endl
	<< "2: MUCOMMANU mode with single evaluation criterion," << std::endl
	<< "   the best parent of the last iteration is retained" << std::endl
	<< "   unless a better individual has been found" << std::endl
	<< "3: MUPLUSNU mode for multiple evaluation criteria, pareto selection" << std::endl
	<< "4: MUCOMMANU mode for multiple evaluation criteria, pareto selection";
}

/******************************************************************************/
/**
 * Sets the sorting scheme. In MUPLUSNU_SINGLEEVAL, new parents will be selected from the entire
 * population, including the old parents. In MUCOMMANU_SINGLEEVAL new parents will be selected
 * from children only. MUNU1PRETAIN_SINGLEEVAL means that the best parent of the last generation
 * will also become a new parent (unless a better child was found). All other parents are
 * selected from children only.
 *
 * @param smode The desired sorting scheme
 */
void GBaseEA::setSortingScheme(sortingMode smode) {
	smode_=smode;
}

/******************************************************************************/
/**
 * Retrieves information about the current sorting scheme (see
 * GBaseEA::setSortingScheme() for further information).
 *
 * @return The current sorting scheme
 */
sortingMode GBaseEA::getSortingScheme() const {
	return smode_;
}

/******************************************************************************/
/**
 * Extracts all individuals on the pareto front
 */
void GBaseEA::extractCurrentParetoIndividuals(std::vector<boost::shared_ptr<Gem::Geneva::GParameterSet> >& paretoInds) {
   // Make sure the vector is empty
   paretoInds.clear();

   GBaseEA::iterator it;
   for(it=this->begin(); it!=this->end(); ++it) {
      if((*it)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) {
         paretoInds.push_back(*it);
      }
   }
}

/******************************************************************************/
/**
 * Some error checks related to population sizes
 */
void GBaseEA::populationSanityChecks() const {
   // First check that we have been given a suitable value for the number of parents.
   // Note that a number of checks (e.g. population size != 0) has already been done
   // in the parent class.
   if(nParents_ == 0) {
      glogger
      << "In GBaseEA::populationSanityChecks(): Error!" << std::endl
      << "Number of parents is set to 0"
      << GEXCEPTION;
   }

   // In MUCOMMANU_SINGLEEVAL mode we want to have at least as many children as parents,
   // whereas MUPLUSNU_SINGLEEVAL only requires the population size to be larger than the
   // number of parents. MUNU1PRETAIN has the same requirements as MUCOMMANU_SINGLEEVAL,
   // as it is theoretically possible that all children are better than the former
   // parents, so that the first parent individual will be replaced.
   std::size_t popSize = getPopulationSize();
   if( // TODO: Why are PARETO modes missing here ?
         ((smode_==MUCOMMANU_SINGLEEVAL || smode_==MUNU1PRETAIN_SINGLEEVAL) && (popSize < 2*nParents_)) ||
         (smode_==MUPLUSNU_SINGLEEVAL && popSize<=nParents_)
   ){
      std::ostringstream error;
      error
      << "In GBaseEA::populationSanityChecks() :" << std::endl
      << "Requested size of population is too small :" << popSize << " " << nParents_ << std::endl
      << "Sorting scheme is ";

      switch(smode_) {
      case MUPLUSNU_SINGLEEVAL:
         error << "MUPLUSNU_SINGLEEVAL" << std::endl;
         break;
      case MUCOMMANU_SINGLEEVAL:
         error << "MUCOMMANU_SINGLEEVAL" << std::endl;
         break;
      case MUNU1PRETAIN_SINGLEEVAL:
         error << "MUNU1PRETAIN" << std::endl;
         break;
      case MUPLUSNU_PARETO:
         error << "MUPLUSNU_PARETO" << std::endl;
         break;
      case MUCOMMANU_PARETO:
         error << "MUCOMMANU_PARETO" << std::endl;
         break;
      };

      glogger
      << error.str()
      << GEXCEPTION;
   }

}

/******************************************************************************/
/**
 * Choose new parents, based on the selection scheme set by the user.
 */
void GBaseEA::selectBest()
{
#ifdef DEBUG
	// We require at this stage that at least the default number of
	// children is present. If individuals can get lost in your setting,
	// you must add mechanisms to "repair" the population before this
	// function is called
	if((data.size()-nParents_) < defaultNChildren_){
	   glogger
	   << "In GBaseEA::select():" << std::endl
      << "Too few children. Got " << data.size()-getNParents() << "," << std::endl
      << "but was expecting at least " << getDefaultNChildren() << std::endl
      << GEXCEPTION;
	}
#endif /* DEBUG */

	switch(smode_) {
	//----------------------------------------------------------------------------
	case MUPLUSNU_SINGLEEVAL:
      {
         sortMuPlusNuMode();
      }
		break;

	//----------------------------------------------------------------------------
	case MUNU1PRETAIN_SINGLEEVAL:
      {
         if(this->inFirstIteration()) {
            sortMuPlusNuMode();
         } else {
            sortMunu1pretainMode();
         }
      }
		break;

	//----------------------------------------------------------------------------
	case MUCOMMANU_SINGLEEVAL:
      {
         if(this->inFirstIteration()) {
            sortMuPlusNuMode();
         } else {
            sortMuCommaNuMode();
         }
      }
		break;

	//----------------------------------------------------------------------------
	case MUPLUSNU_PARETO:
		sortMuPlusNuParetoMode();
		break;

	//----------------------------------------------------------------------------
	case MUCOMMANU_PARETO:
      {
         if(this->inFirstIteration()) {
            sortMuPlusNuParetoMode();
         } else {
            sortMuCommaNuParetoMode();
         }
      }
		break;

   //----------------------------------------------------------------------------
	default:
      {
         glogger
         << "In GBaseEA::selectBest(): Error" << std::endl
         << "Incorrect sorting scheme requested: " << smode_ << std::endl
         << GEXCEPTION;
      }
	   break;
	}

	// Let parents know they are parents
	markParents();
}

/******************************************************************************/
/**
 * Adds the individuals of this iteration to a priority queue. The
 * queue will be sorted by the first evaluation criterion of the individuals
 * and may either have a limited or unlimited size, depending on user-
 * settings. The procedure is different for pareto optimization, as we only
 * want the individuals on the current pareto front to be added.
 */
void GBaseEA::addIterationBests(
   GParameterSetFixedSizePriorityQueue& bestIndividuals
) {
   const bool REPLACE = true;
   const bool CLONE = true;

#ifdef DEBUG
   if(this->empty()) {
      glogger
      << "In GBaseEA::addIterationBests() :" << std::endl
      << "Tried to retrieve the best individuals even though the population is empty." << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   switch(smode_) {
      //----------------------------------------------------------------------------
      case MUPLUSNU_SINGLEEVAL:
      case MUNU1PRETAIN_SINGLEEVAL:
      case MUCOMMANU_SINGLEEVAL:
         GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::addIterationBests(bestIndividuals);
         break;

      //----------------------------------------------------------------------------
      case MUPLUSNU_PARETO:
      case MUCOMMANU_PARETO:
         {
            // Retrieve all individuals on the pareto front
            std::vector<boost::shared_ptr<Gem::Geneva::GParameterSet> > paretoInds;
            this->extractCurrentParetoIndividuals(paretoInds);

            // We simply add all parent individuals to the queue. As we only want
            // the individuals on the current pareto front, we replace all members
            // of the current priority queue
            bestIndividuals.add(paretoInds, CLONE, REPLACE);
         }
         break;

      //----------------------------------------------------------------------------
      default:
         {
            glogger
            << "In GBaseEA::addIterationBests(): Error" << std::endl
            << "Incorrect sorting scheme requested: " << smode_ << std::endl
            << GEXCEPTION;
         }
         break;
   }
}

/******************************************************************************/
/**
 * Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
 * iteration and sorting scheme, the start point will be different. The end-point is not meant
 * to be inclusive.
 *
 * @return The range inside which evaluation should take place
 */
boost::tuple<std::size_t,std::size_t> GBaseEA::getEvaluationRange() const {
   // We evaluate all individuals in the first iteration This happens so pluggable
   // optimization monitors do not need to distinguish between algorithms
   return boost::tuple<std::size_t, std::size_t>(
         inFirstIteration()?0:getNParents()
         ,  data.size()
   );
}

/******************************************************************************/
/**
 * Selection according to the pareto tag, also taking into account the parents of a population (i.e. in MUPLUSNU
 * mode). This is used in conjunction with multi-criterion optimization. See e.g.
 * http://en.wikipedia.org/wiki/Pareto_efficiency for a discussion of this topic.
 */
void GBaseEA::sortMuPlusNuParetoMode() {
	GBaseEA::iterator it, it_cmp;

	// We fall back to the single-eval MUPLUSNU mode if there is just one evaluation criterion
	it=this->begin();
	if(!(*it)->hasMultipleFitnessCriteria()) {
		sortMuPlusNuMode();
	}

	// Mark all individuals as being on the pareto front initially
	for(it=this->begin(); it!=this->end(); ++it) {
		(*it)->getPersonalityTraits<GEAPersonalityTraits>()->resetParetoTag();
	}

	// Compare all parameters
	for(it=this->begin(); it!=this->end(); ++it) {
		for(it_cmp=it+1; it_cmp != this->end(); ++it_cmp) {
			// If we already know that this individual is *not*
			// on the front we do not have to do any tests
			if(!(*it_cmp)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) continue;

			// Check if it dominates it_cmp. If so, mark it accordingly
			if(aDominatesB(*it, *it_cmp)) {
				(*it_cmp)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
			}

			// If a it dominated by it_cmp, we mark it accordingly and break the loop
			if(aDominatesB(*it_cmp, *it)) {
				(*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
				break;
			}
		}
	}

	// At this point we have tagged all individuals according to whether or not they are
	// on the pareto front. Lets sort them accordingly, bringing individuals with the
	// pareto tag to the front of the collection.
	sort(data.begin(), data.end(), indParetoComp());

	// Count the number of individuals on the pareto front
	std::size_t nIndividualsOnParetoFront = 0;
	for(it=this->begin(); it!=this->end(); ++it) {
		if((*it)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) nIndividualsOnParetoFront++;
	}

	// If the number of individuals on the pareto front exceeds the number of parents, we
	// do not want to introduce a bias by selecting only the first nParent individuals. Hence
	// we randomly shuffle them. Note that not all individuals on the pareto front might survive,
	// as subsequent iterations will only take into account parents for the reproduction step.
	// If fewer individuals are on the pareto front than there are parents, then we want the
	// remaining parent positions to be filled up with the non-pareto-front individuals with
	// the best minOnly_fitness(0), i.e. with the best "master" fitness (transformed to take into
	// account minimization and maximization).
	if(nIndividualsOnParetoFront > getNParents()) {
		// randomly shuffle pareto-front individuals to avoid a bias
		std::random_shuffle(this->begin(), this->begin()+nIndividualsOnParetoFront);
	} else if(nIndividualsOnParetoFront < getNParents()) {
		// Sort the non-pareto-front individuals according to their master fitness
	   std::partial_sort(
         data.begin() + nIndividualsOnParetoFront
         , data.begin() + nParents_
         , data.end()
         , boost::bind(&GParameterSet::minOnly_fitness, _1) < boost::bind(&GParameterSet::minOnly_fitness, _2));
	}

	// Finally, we sort the parents only according to their master fitness. This is meant
	// to give some sense to the value recombination scheme. It won't change much in case of the
	// random recombination scheme.
   std::sort(
      data.begin()
      , data.begin() + nParents_
      , boost::bind(&GParameterSet::minOnly_fitness, _1) < boost::bind(&GParameterSet::minOnly_fitness, _2));
}

/******************************************************************************/
/**
 * Selection according to the pareto tag, not taking into account the parents of a population (i.e. in MUCOMMANU
 * mode). This is used in conjunction with multi-criterion optimization. See e.g.
 * http://en.wikipedia.org/wiki/Pareto_efficiency for a discussion of this topic.
 */
void GBaseEA::sortMuCommaNuParetoMode() {
	GBaseEA::iterator it, it_cmp;

	// We fall back to the single-eval MUCOMMANU mode if there is just one evaluation criterion
	it=this->begin();
	if(!(*it)->hasMultipleFitnessCriteria()) {
		sortMuCommaNuMode();
		return;
	}

	// Mark the last iterations parents as not being on the pareto front
	for(it=this->begin(); it!=this->begin() + nParents_; ++it) {
		(*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
	}

	// Mark all children as being on the pareto front initially
	for(it=this->begin()+nParents_; it!=this->end(); ++it) {
		(*it)->getPersonalityTraits<GEAPersonalityTraits>()->resetParetoTag();
	}

	// Compare all parameters of all children
	for(it=this->begin()+nParents_; it!=this->end(); ++it) {
		for(it_cmp=it+1; it_cmp!=this->end(); ++it_cmp) {
			// If we already know that this individual is *not*
			// on the front we do not have to do any tests
			if(!(*it_cmp)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) continue;

			// Check if it dominates it_cmp. If so, mark it accordingly
			if(aDominatesB(*it, *it_cmp)) {
				(*it_cmp)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
			}

			// If a it dominated by it_cmp, we mark it accordingly and break the loop
			if(aDominatesB(*it_cmp, *it)) {
				(*it)->getPersonalityTraits<GEAPersonalityTraits>()->setIsNotOnParetoFront();
				break;
			}
		}
	}

	// At this point we have tagged all children according to whether or not they are
	// on the pareto front. Lets sort them accordingly, bringing individuals with the
	// pareto tag to the front of the population. Note that parents have been manually
	// tagged as not being on the pareto front in the beginning of this function, so
	// sorting the individuals according to the pareto tag will move former parents out
	// of the parents section.
	sort(data.begin(), data.end(), indParetoComp());

	// Count the number of individuals on the pareto front
	std::size_t nIndividualsOnParetoFront = 0;
	for(it=this->begin(); it!=this->end(); ++it) {
		if((*it)->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront()) nIndividualsOnParetoFront++;
	}

	// If the number of individuals on the pareto front exceeds the number of parents, we
	// do not want to introduce a bias by selecting only the first nParent individuals. Hence
	// we randomly shuffle them. Note that not all individuals on the pareto front might survive,
	// as subsequent iterations will only take into account parents for the reproduction step.
	// If fewer individuals are on the pareto front than there are parents, then we want the
	// remaining parent positions to be filled up with the non-pareto-front individuals with
	// the best minOnly_fitness(0), i.e. with the best "master" fitness, transformed to take into account
	// minimization and maximization. Note that, unlike MUCOMMANU_SINGLEEVAL
	// this implies the possibility that former parents are "elected" as new parents again. This
	// might be changed in subsequent versions of Geneva (TODO).
	if(nIndividualsOnParetoFront > getNParents()) {
		// randomly shuffle pareto-front individuals to avoid a bias
		std::random_shuffle(this->begin(), this->begin()+nIndividualsOnParetoFront);
	} else if(nIndividualsOnParetoFront < getNParents()) {
		// Sort the non-pareto-front individuals according to their master fitness
	   std::partial_sort(
         data.begin() + nIndividualsOnParetoFront
         , data.begin() + nParents_
         , data.end()
         , boost::bind(&GParameterSet::minOnly_fitness, _1) < boost::bind(&GParameterSet::minOnly_fitness, _2)
	   );
	}

	// Finally, we sort the parents only according to their master fitness. This is meant
	// to give some sense to the value recombination scheme. It won't change much in case of the
	// random recombination scheme.
	std::sort(
      data.begin()
      , data.begin() + nParents_
      , boost::bind(&GParameterSet::minOnly_fitness, _1) < boost::bind(&GParameterSet::minOnly_fitness, _2)
	);
}

/******************************************************************************/
/**
 * Determines whether the first individual dominates the second.
 *
 * @param a The individual that is assumed to dominate
 * @param b The individual that is assumed to be dominated
 * @return A boolean indicating whether the first individual dominates the second
 */
bool GBaseEA::aDominatesB(
      boost::shared_ptr<GParameterSet> a
      , boost::shared_ptr<GParameterSet>b) const
{
   std::size_t nCriteriaA = a->getNumberOfFitnessCriteria();

#ifdef DEBUG
   std::size_t nCriteriaB = b->getNumberOfFitnessCriteria();
   if(nCriteriaA != nCriteriaB) {
      glogger
      << "In GBaseEA::aDominatesB(): Error!" << std::endl
      << "Number of fitness criteria differ: " << nCriteriaA << " / " << nCriteriaB << std::endl
      << GEXCEPTION;
   }
#endif

   for(std::size_t i=0; i<nCriteriaA; i++) {
      if(this->isWorse(a->transformedFitness(i), b->transformedFitness(i))) return false;
   }

   return true;
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBaseEA::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if(GParameterSetParChild::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
	condnotset("GBaseEA::modify_GUnitTests", "GEM_TESTING");
	return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Fills the collection with individuals.
 *
 * @param nIndividuals The number of individuals that should be added to the collection
 */
void GBaseEA::fillWithObjects(const std::size_t& nIndividuals) {
#ifdef GEM_TESTING
	// Clear the collection, so we can start fresh
	BOOST_CHECK_NO_THROW(this->clear());

	// Add some some
	for(std::size_t i=0; i<nIndividuals; i++) {
		this->push_back(boost::shared_ptr<Gem::Tests::GTestIndividual1>(new Gem::Tests::GTestIndividual1()));
	}

	// Make sure we have unique data items
	this->randomInit(ALLPARAMETERS);

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseEA::fillWithObjects", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseEA::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
   GParameterSetParChild::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------

	{ // Call the parent class'es function
		boost::shared_ptr<GBaseEA> p_test = this->clone<GBaseEA>();

		// Fill p_test with individuals
		p_test->fillWithObjects();

		// Run the parent class'es tests
		p_test->GParameterSetParChild::specificTestsNoFailureExpected_GUnitTests();
	}

	//------------------------------------------------------------------------------

	{ // Check setting and retrieval of the population size and number of parents/childs
		boost::shared_ptr<GBaseEA> p_test = this->clone<GBaseEA>();

		// Set the default population size and number of children to different numbers
		for(std::size_t nChildren=5; nChildren<10; nChildren++) {
			for(std::size_t nParents=1; nParents < nChildren; nParents++) {
				// Clear the collection
				BOOST_CHECK_NO_THROW(p_test->clear());

				// Add the required number of individuals
				p_test->fillWithObjects(nParents + nChildren);

				BOOST_CHECK_NO_THROW(p_test->setPopulationSizes(nParents+nChildren, nParents));

				// Check that the number of parents is as expected
				BOOST_CHECK_MESSAGE(p_test->getNParents() == nParents,
					   "p_test->getNParents() == " << p_test->getNParents()
					<< ", nParents = " << nParents
					<< ", size = " << p_test->size());

				// Check that the actual number of children has the same value
				BOOST_CHECK_MESSAGE(p_test->getNChildren() == nChildren,
						"p_test->getNChildren() = " << p_test->getNChildren()
						<< ", nChildren = " << nChildren);
			}
		}
	}

	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseEA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseEA::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
   GParameterSetParChild::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */
   condnotset("GBaseEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GBaseEA::GEAOptimizationMonitor::GEAOptimizationMonitor()
	: GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT()
	, xDim_(DEFAULTXDIMOM)
	, yDim_(DEFAULTYDIMOM)
	, nMonitorInds_(0)
   , resultFile_(DEFAULTROOTRESULTFILEOM)
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GEAOptimizationMonitor object
 */
GBaseEA::GEAOptimizationMonitor::GEAOptimizationMonitor(const GBaseEA::GEAOptimizationMonitor& cp)
	: GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT(cp)
	, xDim_(cp.xDim_)
	, yDim_(cp.yDim_)
	, nMonitorInds_(cp.nMonitorInds_)
	, resultFile_(cp.resultFile_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GBaseEA::GEAOptimizationMonitor::~GEAOptimizationMonitor()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator.
 *
 * @param cp A copy of another GEAOptimizationMonitor object
 * @return A constant reference to this object
 */
const GBaseEA::GEAOptimizationMonitor& GBaseEA::GEAOptimizationMonitor::operator=(const GBaseEA::GEAOptimizationMonitor& cp){
	GBaseEA::GEAOptimizationMonitor::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GEAOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseEA::GEAOptimizationMonitor::operator==(const GBaseEA::GEAOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GBaseEA::GEAOptimizationMonitor::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GEAOptimizationMonitor object
 *
 * @param  cp A constant reference to another GEAOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseEA::GEAOptimizationMonitor::operator!=(const GBaseEA::GEAOptimizationMonitor& cp) const {
	using namespace Gem::Common;
	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GBaseEA::GEAOptimizationMonitor::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
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
boost::optional<std::string> GBaseEA::GEAOptimizationMonitor::checkRelationshipWith(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
		, const std::string& caller
		, const std::string& y_name
		, const bool& withMessages
) const {
	using namespace Gem::Common;

	// Check that we are indeed dealing with a GParamterBase reference
	const GBaseEA::GEAOptimizationMonitor *p_load = GObject::gobject_conversion<GBaseEA::GEAOptimizationMonitor >(&cp);

	// Will hold possible deviations from the expectation, including explanations
	std::vector<boost::optional<std::string> > deviations;

	// Check our parent class'es data ...
	deviations.push_back(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::checkRelationshipWith(cp, e, limit, "GBaseEA::GEAOptimizationMonitor", y_name, withMessages));

	// ... and then our local data.
	deviations.push_back(checkExpectation(withMessages, "GBaseEA::GEAOptimizationMonitor", xDim_, p_load->xDim_, "xDim_", "p_load->xDim_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseEA::GEAOptimizationMonitor", yDim_, p_load->yDim_, "yDim_", "p_load->yDim_", e , limit));
	deviations.push_back(checkExpectation(withMessages, "GBaseEA::GEAOptimizationMonitor", nMonitorInds_, p_load->nMonitorInds_, "nMonitorInds_", "p_load->nMonitorInds_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GBaseEA::GEAOptimizationMonitor", resultFile_, p_load->resultFile_, "resultFile_", "p_load->resultFile_", e , limit));

	return evaluateDiscrepancies("GBaseEA::GEAOptimizationMonitor", caller, deviations, e);
}

/******************************************************************************/
/**
 * Allows to specify a different name for the result file
 *
 * @param resultFile The desired name of the result file
 */
void GBaseEA::GEAOptimizationMonitor::setResultFileName(
      const std::string& resultFile
) {
  resultFile_ = resultFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the result file name
 *
 * @return The current name of the result file
 */
std::string GBaseEA::GEAOptimizationMonitor::getResultFileName() const {
  return resultFile_;
}

/******************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseEA::GEAOptimizationMonitor::firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   // Perform the conversion to the target algorithm
#ifdef DEBUG
   if(goa->getOptimizationAlgorithm() != "PERSONALITY_EA") {
      glogger
      <<  "In GBaseEA::GEAOptimizationMonitor::firstInformation():" << std::endl
      << "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   // Convert the base pointer to the target type
   GBaseEA * const ea = static_cast<GBaseEA * const>(goa);

#ifdef DEBUG
   if(nMonitorInds_ > ea->size()) {
      glogger
      <<  "In GBaseEA::GEAOptimizationMonitor::firstInformation():" << std::endl
      << "Provided number of monitored individuals is larger than the population: " << std::endl
      << nMonitorInds_ << " / " << ea->size() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

   // Determine a suitable number of monitored individuals, if it hasn't already
   // been set externally. We allow a maximum of 3 monitored individuals by default
   // (or the number of parents, if <= 3).
   if(nMonitorInds_ == 0) {
      nMonitorInds_ = (std::min)(ea->getNParents(), std::size_t(3));
   }

   // Set up the plotters
   for(std::size_t ind=0; ind<nMonitorInds_; ind++) {
      boost::shared_ptr<Gem::Common::GGraph2D> graph(new Gem::Common::GGraph2D());
      graph->setXAxisLabel("Iteration");
      graph->setYAxisLabel("Fitness");
      graph->setPlotLabel(std::string("Individual ") + boost::lexical_cast<std::string>(ind));
      graph->setPlotMode(Gem::Common::CURVE);

      fitnessGraphVec_.push_back(graph);
   }
}

/******************************************************************************/
/**
 * A function that is called during each optimization cycle. It is possible to
 * extract quite comprehensive information in each iteration. For examples, see
 * the standard overloads provided for the various optimization algorithms.
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseEA::GEAOptimizationMonitor::cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   bool isDirty = false;
   double currentTransformedEvaluation = 0.;

	// Convert the base pointer to the target type
	GBaseEA * const ea = static_cast<GBaseEA * const>(goa);

   // Retrieve the current iteration
   boost::uint32_t iteration = ea->getIteration();

   for(std::size_t ind=0; ind<nMonitorInds_; ind++) {
      // Get access to the individual
      boost::shared_ptr<GParameterSet> gi_ptr = ea->individual_cast<GParameterSet>(ind);

      // Retrieve the fitness of this individual -- all individuals should be "clean" here
      currentTransformedEvaluation = gi_ptr->transformedFitness();
      // Add the data to our graph
      (fitnessGraphVec_.at(ind))->add(boost::tuple<double,double>(iteration, currentTransformedEvaluation));
   }
}

/******************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseEA::GEAOptimizationMonitor::lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
   Gem::Common::GPlotDesigner gpd(
         std::string("Fitness of ") + boost::lexical_cast<std::string>(nMonitorInds_) + std::string(" best EA individuals")
         , 1
         , nMonitorInds_
   );

   gpd.setCanvasDimensions(xDim_, yDim_);

   // Copy all plotters into the GPlotDesigner object
   std::vector<boost::shared_ptr<Gem::Common::GGraph2D> >::iterator it;
   for(it=fitnessGraphVec_.begin(); it!=fitnessGraphVec_.end(); ++it) {
      gpd.registerPlotter(*it);
   }

   // Write out the plot
   gpd.writeToFile(this->getResultFileName());

   // Clear all plotters, so they do not get added repeatedly, when
   // optimize is called repeatedly on the same (or a cloned) object.
   fitnessGraphVec_.clear();
}

/******************************************************************************/
/**
 * Allows to set the dimensions of the canvas
 *
 * @param xDim The desired dimension of the canvas in x-direction
 * @param yDim The desired dimension of the canvas in y-direction
 */
void GBaseEA::GEAOptimizationMonitor::setDims(const boost::uint32_t& xDim, const boost::uint32_t& yDim) {
	xDim_ = xDim;
	yDim_ = yDim;
}

/******************************************************************************/
/**
 * Retrieve the dimensions as a tuple
 *
 * @return The dimensions of the canvas as a tuple
 */
boost::tuple<boost::uint32_t, boost::uint32_t> GBaseEA::GEAOptimizationMonitor::getDims() const {
   return boost::tuple<boost::uint32_t, boost::uint32_t>(xDim_, yDim_);
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in x-direction
 *
 * @return The dimension of the canvas in x-direction
 */
boost::uint32_t GBaseEA::GEAOptimizationMonitor::getXDim() const {
	return xDim_;
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in y-direction
 *
 * @return The dimension of the canvas in y-direction
 */
boost::uint32_t GBaseEA::GEAOptimizationMonitor::getYDim() const {
	return yDim_;
}

/******************************************************************************/
/**
 * Sets the number of individuals in the population that should be monitored
 *
 * @oaram nMonitorInds The number of individuals in the population that should be monitored
 */
void GBaseEA::GEAOptimizationMonitor::setNMonitorIndividuals(const std::size_t& nMonitorInds) {
	nMonitorInds_ = nMonitorInds;
}

/******************************************************************************/
/**
 * Retrieves the number of individuals that are being monitored
 *
 * @return The number of individuals in the population being monitored
 */
std::size_t GBaseEA::GEAOptimizationMonitor::getNMonitorIndividuals() const {
	return nMonitorInds_;
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GBaseEA::GEAOptimizationMonitor object, camouflaged as a GObject
 */
void GBaseEA::GEAOptimizationMonitor::load_(const GObject* cp) {
	const GBaseEA::GEAOptimizationMonitor *p_load = gobject_conversion<GBaseEA::GEAOptimizationMonitor>(cp);

	// Load the parent classes' data ...
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::load_(cp);

	// ... and then our local data
	xDim_ = p_load->xDim_;
	yDim_ = p_load->yDim_;
	nMonitorInds_ = p_load->nMonitorInds_;
	resultFile_ = p_load->resultFile_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GBaseEA::GEAOptimizationMonitor::clone_() const {
	return new GBaseEA::GEAOptimizationMonitor(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GBaseEA::GEAOptimizationMonitor::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if(GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseEA::GEAOptimizationMonitor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseEA::GEAOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseEA::GEAOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseEA::GEAOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
   // Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */
   condnotset("GBaseEA::GEAOptimizationMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
