/**
 * @file G_OA_SimulatedAnnealing.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <tuple>

// Boost headers go here

#ifndef G_OA_SIMULATEDANNEALING_HPP_
#define G_OA_SIMULATEDANNEALING_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OA_ParChildT.hpp"
#include "geneva/GSAPersonalityTraits.hpp"
#include "geneva/G_OA_ParChildT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This is a specialization of the GParameterSetParChild class. The class adds
 * an infrastructure for simulated annealing (Geneva-style, i.e. with larger populations).
 */
template<typename executor_type>
class GSimulatedAnnealingT : public G_OA_ParChildT<executor_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("G_OA_ParChildT<executor_type>", boost::serialization::base_object<G_OA_ParChildT<executor_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_t0)
		 & BOOST_SERIALIZATION_NVP(m_t)
		 & BOOST_SERIALIZATION_NVP(m_alpha)
		 & BOOST_SERIALIZATION_NVP(m_n_threads);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor. All initialization work of member variable
	  * is done in the class body.
	  */
	 GSimulatedAnnealingT()
		 : G_OA_ParChildT<executor_type>()
	 {
		 // Make sure we start with a valid population size if the user does not supply these values
		 this->setPopulationSizes(100, 1);
	 }

	 /***************************************************************************/
	 /**
	  * A standard copy constructor
	  *
	  * @param cp Another GSimulatedAnnealingT object
	  */
	 GSimulatedAnnealingT(const GSimulatedAnnealingT<executor_type>& cp)
		 : G_OA_ParChildT<executor_type>(cp)
		 , m_t0(cp.m_t0)
		 , m_t(cp.m_t)
		 , m_alpha(cp.m_alpha)
		 , m_n_threads(cp.m_n_threads)
	 {
		 // Copying / setting of the optimization algorithm id is done by the parent class. The same
		 // applies to the copying of optimization monitors.
	 }

	 /***************************************************************************/
	 /**
     * The standard destructor. All work is done in the parent class.
     */
	 virtual ~GSimulatedAnnealingT()
	 { /* nothing */ }

	 /***************************************************************************/
 	 /**
	  * The standard assignment operator
	  */
	 const GSimulatedAnnealingT& operator=(const GSimulatedAnnealingT<executor_type>& cp) {
		 this->load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GSimulatedAnnealingT object
	  *
	  * @param  cp A constant reference to another GSimulatedAnnealingT object
	  * @return A boolean indicating whether both objects are equal
	  */
	 virtual bool operator==(const GSimulatedAnnealingT<executor_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GSimulatedAnnealingT object
	  *
	  * @param  cp A constant reference to another GSimulatedAnnealingT object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 virtual bool operator!=(const GSimulatedAnnealingT<executor_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch (g_expectation_violation &) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
 	  * Searches for compliance with expectations with respect to another object
 	  * of the same type
 	  *
 	  * @param cp A constant reference to another GObject object
 	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GObject& cp // the other object
		 , const Gem::Common::expectation& e // the expectation for this object, e.g. equality
		 , const double& limit// the limit for allowed deviations of floating point types
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference independent of this object and convert the pointer
		 const GSimulatedAnnealingT<executor_type> *p_load
			 = Gem::Common::g_convert_and_compare<GObject, GSimulatedAnnealingT<executor_type>>(cp, this);

		 GToken token("GSimulatedAnnealingT", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<G_OA_ParChildT<executor_type>>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 compare_t(IDENTITY(m_t0, p_load->m_t0), token);
		 compare_t(IDENTITY(m_t, p_load->m_t), token);
		 compare_t(IDENTITY(m_alpha, p_load->m_alpha), token);
		 compare_t(IDENTITY(m_n_threads, p_load->m_n_threads), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Resets the settings of this population to what was configured when
	  * the optimize()-call was issued
	  */
	 virtual void resetToOptimizationStart() {
		 // Reset the temperature
		 m_t = m_t0;

		 // Remove any remaining old work items
		 m_old_work_items.clear();

		 // There is no more work to be done here, so we simply call the
		 // function of the parent class
		 G_OA_ParChildT<executor_type>::resetToOptimizationStart();
	 }

	 /***************************************************************************/
	 /**
	  * Returns information about the type of optimization algorithm. This function needs
	  * to be overloaded by the actual algorithms to return the correct type.
	  *
	  * @return The type of optimization algorithm
	  */
	 virtual std::string getAlgorithmPersonalityType() const override {
		 return std::string("PERSONALITY_SA");
	 }

	 /***************************************************************************/
	 /**
 	  * Returns the name of this optimization algorithm
 	  *
 	  * @return The name assigned to this optimization algorithm
 	  */
	 virtual std::string getAlgorithmName() const override {
		 return std::string("Simulated Annealing");
	 }

	 /***************************************************************************/
	 /**
 	  * Adds local configuration options to a GParserBuilder object
 	  *
 	  * @param gpb The GParserBuilder object to which configuration options should be added
 	  */
	 virtual void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override {
		 // Call our parent class'es function
		 G_OA_ParChildT<executor_type>::addConfigurationOptions(gpb);

		 // Add local data
		 gpb.registerFileParameter<std::uint16_t>(
			 "nAdaptionThreads" // The name of the variable
			 , DEFAULTNSTDTHREADS // The default value
			 , [this](std::uint16_t nt) { this->setNThreads(nt); }
		 )
			 << "The number of threads used to simultaneously adapt individuals" << std::endl
			 << "0 means \"automatic\"";

		 gpb.registerFileParameter<double>(
			 "t0" // The name of the variable
			 , SA_T0 // The default value
			 , [this](double sat0) { this->setT0(sat0); }
		 )
			 << "The start temperature used in simulated annealing";

		 gpb.registerFileParameter<double>(
			 "alpha" // The name of the variable
			 , SA_ALPHA // The default value
			 , [this](double ds) { this->setTDegradationStrength(ds); }
		 )
			 << "The degradation strength used in the cooling" << std::endl
			 << "schedule in simulated annealing;";
	 }

	 /***************************************************************************/
	 /**
     * Sets the number of threads this population uses for adaption. If nThreads is set
     * to 0, an attempt will be made to set the number of threads to the number of hardware
     * threading units (e.g. number of cores or hyperthreading units).
 	  *
 	  * @param nThreads The number of threads this class uses
 	  */
	 void setNThreads(std::uint16_t nThreads) {
		 if (nThreads == 0) {
			 m_n_threads = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(DEFAULTNSTDTHREADS));
		 }
		 else {
			 m_n_threads = nThreads;
		 }
	 }

	/***************************************************************************/
	/**
 	 * Retrieves the number of threads this population uses for adaption
 	 *
 	 * @return The maximum number of allowed threads
 	 */
	 std::uint16_t getNThreads() const {
		 return m_n_threads;
	 }

	 /***************************************************************************/
	 /**
 	  * Determines the strength of the temperature degradation. This function is used for simulated annealing.
 	  *
 	  * @param alpha The degradation speed of the temperature
 	  */
	 void setTDegradationStrength(double alpha) {
		 if (alpha <= 0.) {
			 glogger
				 << "In GSimulatedAnnealingT<executor_type>::setTDegradationStrength(const double&):" << std::endl
				 << "Got negative alpha: " << alpha << std::endl
				 << GEXCEPTION;
		 }

		 m_alpha = alpha;
	 }

	 /***************************************************************************/
	 /**
 	  * Retrieves the temperature degradation strength. This function is used for simulated annealing.
 	  *
 	  * @return The temperature degradation strength
 	  */
	 double getTDegradationStrength() const {
		 return m_alpha;
	 }

	 /***************************************************************************/
	 /**
 	  * Sets the start temperature. This function is used for simulated annealing.
 	  *
 	  * @param t0 The start temperature
 	  */
	 void setT0(double t0) {
		 if (t0 <= 0.) {
			 glogger
				 << "In GSimulatedAnnealingT<executor_type>::setT0(const double&):" << std::endl
				 << "Got negative start temperature: " << t0 << std::endl
				 << GEXCEPTION;
		 }

		 m_t0 = t0;
	 }

	 /***************************************************************************/
	 /**
 	  * Retrieves the start temperature. This function is used for simulated annealing.
 	  *
 	  * @return The start temperature
 	  */
	 double getT0() const {
		 return m_t0;
	 }

	 /***************************************************************************/
	 /**
 	  * Retrieves the current temperature. This function is used for simulated annealing.
 	  *
 	  * @return The current temperature
 	  */
	 double getT() const {
		 return m_t;
	 }

	 /***************************************************************************/
    /**
 	  * Emits a name for this class / object
 	  */
	 virtual std::string name() const override {
		 return std::string("GSimulatedAnnealingT");
	 }

protected:
	 /***************************************************************************/
	 /**
 	  * Loads the data of another GSimulatedAnnealingT object, camouflaged as a GObject.
     *
 	  * @param cp A pointer to another GSimulatedAnnealingT object, camouflaged as a GObject
 	  */
	 virtual void load_(const GObject *cp) override {
		 // Check that we are dealing with a GSimulatedAnnealingT<executor_type> reference independent
		 // of this object and convert the pointer
		 const GSimulatedAnnealingT<executor_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GSimulatedAnnealingT<executor_type>>(cp, this);

		 // First load the parent class'es data ...
		 G_OA_ParChildT<executor_type>::load_(cp);

		 // ... and then our own data
		 m_t0 = p_load->m_t0;
		 m_t = p_load->m_t;
		 m_alpha = p_load->m_alpha;
		 m_n_threads = p_load->m_n_threads;
	 }

	 /***************************************************************************/
	 /**
 	  * Creates a deep copy of this object
 	  *
 	  * @return A deep copy of this object
 	  */
	 virtual GObject *clone_() const override {
		 return new GSimulatedAnnealingT<executor_type>(*this);
	 }

	 /***************************************************************************/
	 /**
 	  * Some error checks related to population sizes
 	  */
	 virtual void populationSanityChecks() const override {
		 // First check that we have been given a suitable value for the number of parents.
		 // Note that a number of checks (e.g. population size != 0) has already been done
		 // in the parent class.
		 if (this->m_n_parents == 0) {
			 glogger
				 << "In GSimulatedAnnealingT<executor_type>::populationSanityChecks(): Error!" << std::endl
				 << "Number of parents is set to 0"
				 << GEXCEPTION;
		 }

		 // We need at least as many children as parents
		 std::size_t popSize = this->getPopulationSize();
		 if (popSize <= this->m_n_parents) {
			 glogger
				 << "In GSimulatedAnnealingT<executor_type>::populationSanityChecks() :" << std::endl
				 << "Requested size of population is too small :" << popSize << " " << this->m_n_parents << std::endl
				 << GEXCEPTION;
		 }
	 }

	 /***************************************************************************/
	 /**
 	  * Adapt all children in parallel. Evaluation is done in a separate function (runFitnessCalculation).
 	  */
	 virtual void adaptChildren() override {
		 // Retrieve the range of individuals to be adapted
		 std::tuple<std::size_t, std::size_t> range = this->getAdaptionRange();

		 // Loop over all requested individuals and perform the adaption
		 for (auto it = (this->begin() + std::get<0>(range)); it != (this->begin() + std::get<1>(range)); ++it) {
			 m_tp_ptr->async_schedule(
				 // Note: may not pass it as a reference, as it is a local variable in the loop and might
				 // vanish of have been altered once the thread has started and adaption is requested.
				 [it]() {
					 (*it)->adapt();
				 }
			 );
		 }

		 // Wait for all threads in the pool to complete their work
		 m_tp_ptr->wait();
	 }

	 /***************************************************************************/
	 /**
 	  * We submit individuals to the broker connector and wait for processed items.
     */
	 virtual void runFitnessCalculation() override {
		 //--------------------------------------------------------------------------------
		 // Start by marking the work to be done in the individuals.
		 // "range" will hold the start- and end-points of the range
		 // to be worked on
		 std::tuple<std::size_t, std::size_t> range = getEvaluationRange();

#ifdef DEBUG
		 // There should be no situation in which a "clean" child is submitted
		 // through this function. There MAY be situations, where in the first iteration
		 // parents are clean, e.g. when they were extracted from another optimization.
		 for(std::size_t i=this->getNParents(); i<this->size(); i++) {
			 if(!this->at(i)->isDirty()) {
			 	 glogger
				 << "In GSimulatedAnnealingT<executor_type>::runFitnessCalculation(): Error!" << std::endl
				 << "Tried to evaluate children in range " << std::get<0>(range) << " - " << std::get<1>(range) << std::endl
				 << "but found \"clean\" individual in position " << i << std::endl
				 << GEXCEPTION;
			 }
		 }
#endif

		 //--------------------------------------------------------------------------------
		 // Retrieve a vector describing the items to be modified
		 std::vector<bool> workItemPos = Gem::Courtier::getBooleanMask(this->size(), std::get<0>(range), std::get<1>(range));

		 //--------------------------------------------------------------------------------
		 // Now submit work items and wait for results.
		 this->workOn(
			 this->data
			 , workItemPos
			 , m_old_work_items
			 , false // do not resubmit unprocessed items
			 , "GSimulatedAnnealingT<executor_type>::runFitnessCalculation()"
		 );

		 //--------------------------------------------------------------------------------
		 // Take care of unprocessed items
		 Gem::Common::erase_according_to_flags(this->data, workItemPos, Gem::Courtier::GBC_UNPROCESSED, 0, this->size());

		 // Remove items for which an error has occurred during processing
		 Gem::Common::erase_if(
			 this->data
			 , [this](std::shared_ptr<GParameterSet> p) -> bool {
				 return p->processing_was_unsuccessful();
			 }
		 );

		 //--------------------------------------------------------------------------------
		 // Now fix the population -- it may be smaller than its nominal size
		 fixAfterJobSubmission();
	 }

	 /***************************************************************************/
	 /**
     * Fixes the population after a job submission
     */
	 void fixAfterJobSubmission() {
		 std::size_t np = this->getNParents();
		 std::uint32_t iteration = this->getIteration();

		 // Remove parents from older iterations from old work items -- we do not want them.
		 // Note that "remove_if" simply moves items not satisfying the predicate to the end of the list.
		 // We thus need to explicitly erase these items. remove_if returns the iterator position right after
		 // the last item not satisfying the predicate.
		 m_old_work_items.erase(
			 std::remove_if(
				 m_old_work_items.begin()
				 , m_old_work_items.end()
				 , [iteration](std::shared_ptr<GParameterSet> x) -> bool {
					 return x->getPersonalityTraits<GSAPersonalityTraits>()->isParent() && x->getAssignedIteration() != iteration;
				 }
			 )
			 , m_old_work_items.end()
		 );

		 // Make it known to remaining old individuals that they are now part of a new iteration
		 std::for_each(
			 m_old_work_items.begin(), m_old_work_items.end(),
			 [iteration](std::shared_ptr <GParameterSet> p) { p->setAssignedIteration(iteration); }
		 );

		 // Make sure that parents are at the beginning of the array.
		 sort(
			 this->begin()
			 , this->end()
			 , [](std::shared_ptr<GParameterSet> x, std::shared_ptr<GParameterSet> y) -> bool {
				 return (x->getPersonalityTraits<GSAPersonalityTraits>()->isParent() > y->getPersonalityTraits<GSAPersonalityTraits>()->isParent());
			 }
		 );

		 // Attach all old work items to the end of the current population and clear the array of old items
		 for(auto item: m_old_work_items) {
			 this->push_back(item);
		 }
		 m_old_work_items.clear();

		 // Check that individuals do exist in the population. We cannot continue, if this is not the case
		 if(this->empty()) {
			 glogger
				 << "In GSimulatedAnnealingT<executor_type>::fixAfterJobSubmission(): Error!" << std::endl
				 << "Population holds no data" << std::endl
				 << GEXCEPTION;
		 } else {
			 // Emit a warning if no children have returned
			 if(this->size() <= this->getNParents()) {
				 glogger
					 << "In GSimulatedAnnealingT<executor_type>::fixAfterJobSubmission(): Warning!" << std::endl
					 << "No child individuals have returned" << std::endl
					 << "We need to fill up the population with clones from parent individuals" << std::endl
					 << GWARNING;
			 }
		 }

		 // Check that the dirty flag of the last individual isn't set. This is a severe error.
		 if(this->back()->isDirty()) {
			 glogger
				 << "In GSimulatedAnnealingT<executor_type>::fixAfterJobSubmission(): Error!" << std::endl
				 << "The last individual in the population has the dirty" << std::endl
				 << "flag set, so we cannot use it for cloning" << std::endl
				 << GEXCEPTION;
		 }


		 // Add missing individuals, as clones of the last item
		 if (this->size() < this->getDefaultPopulationSize()) {
			 std::size_t fixSize = this->getDefaultPopulationSize() - this->size();
			 for (std::size_t i = 0; i < fixSize; i++) {
				 // This function will create a clone of its argument
				 this->push_back_clone(this->back());
			 }
		 }

		 // Mark the first this->m_n_parents individuals as parents and the rest of the individuals as children.
		 // We want to have a sane population.
		 typename GOptimizationAlgorithmT<executor_type>::iterator it;
		 for (it = this->begin(); it != this->begin() + np; ++it) {
			 (*it)->GParameterSet::template getPersonalityTraits<GSAPersonalityTraits>()->setIsParent();
		 }
		 for (it = this->begin() + np; it != this->end(); ++it) {
			 (*it)->GParameterSet::template getPersonalityTraits<GSAPersonalityTraits>()->setIsChild();
		 }

		 // We care for too many returned individuals in the selectBest() function. Older
		 // individuals might nevertheless have a better quality. We do not want to loose them.
	 }

	 /***************************************************************************/
	 /**
 	  * Choose new parents, based on the SA selection scheme.
 	  */
	 virtual void selectBest() override {
		 // Sort according to the "Simulated Annealing" scheme
		 sortSAMode();

		 // Let parents know they are parents
		 this->markParents();

#ifdef DEBUG
		 // Make sure our population is not smaller than its nominal size -- this
   	 // should have been taken care of in fixAfterJobSubmission() .
   	 if(this->size() < this->getDefaultPopulationSize()) {
      	 glogger
      	 << "In GSimulatedAnnealingT<executor_type>::selectBest(): Error!" << std::endl
      	 << "Size of population is smaller than expected: " << this->size() << " / " << this->getDefaultPopulationSize() << std::endl
      	 << GEXCEPTION;
   	 }
#endif /* DEBUG */

		 ////////////////////////////////////////////////////////////
		 // At this point we have a sorted list of individuals and can take care of
		 // too many members, so the next iteration finds a "standard" population. This
		 // function will remove the last items.
		 this->resize(this->getNParents() + this->getDefaultNChildren());

		 // Let children know they are children
		 this->markChildren();

		 // Everything should be back to normal ...
	 }

	 /***************************************************************************/
	 /**
 	  * Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
 	  * iteration and sorting scheme, the start point will be different. The end-point is not meant
 	  * to be inclusive.
 	  *
 	  * @return The range inside which evaluation should take place
 	  */
	 virtual std::tuple<std::size_t,std::size_t> getEvaluationRange() const override {
		 // We evaluate all individuals in the first iteration This happens so pluggable
		 // optimization monitors do not need to distinguish between algorithms
		 return std::tuple<std::size_t, std::size_t>(
			 this->inFirstIteration() ? 0 : this->getNParents(), this->size()
		 );
	 }

	 /***************************************************************************/
	 /**
     * Does any necessary initialization work
 	  */
	 virtual void init() override {
		 // To be performed before any other action. Place any further work after this call.
		 G_OA_ParChildT<executor_type>::init();

		 // Initialize our thread pool
		 m_tp_ptr.reset(new Gem::Common::GThreadPool(m_n_threads));
	 }

	 /***************************************************************************/
	 /**
	  * Does any necessary finalization work
	  */
	 virtual void finalize() override {
		 // Check whether there were any errors during thread execution
		 if (m_tp_ptr->hasErrors()) {
			 std::vector<std::string> errors = m_tp_ptr->getErrors();

			 glogger
				 << "========================================================================" << std::endl
				 << "In GSimulatedAnnealingT<executor_type>::finalize():" << std::endl
				 << "There were errors during thread execution:" << std::endl
				 << std::endl;

			 for(auto e: errors) {
				 glogger << e << std::endl;
			 }

			 glogger
				 << std::endl
				 << "========================================================================" << std::endl
				 << GEXCEPTION;
		 }

		 // Terminate our thread pool
		 m_tp_ptr.reset();

		 // Last action. Place any "local" finalization action before this call.
		 G_OA_ParChildT<executor_type>::finalize();
	 }

	 /***************************************************************************/
	 /**
 	  * Retrieve a GPersonalityTraits object belonging to this algorithm
 	  */
	 virtual std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override {
		 return std::shared_ptr<GSAPersonalityTraits>(new GSAPersonalityTraits());
	 }

private:
	 /***************************************************************************/
	 /**
	  * Performs a simulated annealing style sorting and selection
	  */
	 void sortSAMode() {
		 // Position the nParents best children of the population right behind the parents
		 std::partial_sort(
			 this->begin() + this->m_n_parents, this->begin() + 2 * this->m_n_parents, this->end(),
			 [](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
				 return x->minOnly_fitness() < y->minOnly_fitness();
			 }
		 );

		 // Check for each parent whether it should be replaced by the corresponding child
		 for (std::size_t np = 0; np < this->m_n_parents; np++) {
			 double pPass = saProb(this->at(np)->minOnly_fitness(), this->at(this->m_n_parents + np)->minOnly_fitness());
			 if (pPass >= 1.) {
				 this->at(np)->GObject::load(this->at(this->m_n_parents + np));
			 } else {
				 double challenge =
					 this->m_uniform_real_distribution(this->m_gr, std::uniform_real_distribution<double>::param_type(0.,1.));
				 if (challenge < pPass) {
					 this->at(np)->GObject::load(this->at(this->m_n_parents + np));
				 }
			 }
		 }

		 // Sort the parents -- it is possible that a child with a worse fitness has replaced a parent
		 std::sort(
			 this->begin(), this->begin() + this->m_n_parents,
			 [](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
				 return x->minOnly_fitness() < y->minOnly_fitness();
			 }
		 );

		 // Make sure the temperature gets updated
		 updateTemperature();
	 }

	 /***************************************************************************/
	 /**
 	  * Calculates the simulated annealing probability for a child to replace a parent
 	  *
 	  * @param qParent The fitness of the parent
 	  * @param qChild The fitness of the child
 	  * @return A double value in the range [0,1[, representing the likelihood for the child to replace the parent
 	  */
	 double saProb(const double &qParent, const double &qChild) {
		 // We do not have to do anything if the child is better than the parent
		 if (this->at(0)->isBetter(qChild, qParent)) {
			 return 2.;
		 }

		 double result = 0.;
		 if (this->at(0)->getMaxMode()) {
			 result = exp(-(qParent - qChild) / m_t);
		 } else {
			 result = exp(-(qChild - qParent) / m_t);
		 }

		 return result;
	 }

	 /***************************************************************************/
	 /**
 	  * Updates the temperature. This function is used for simulated annealing.
 	  */
	 void updateTemperature() {
		 m_t *= m_alpha;
	 }

	 /***************************************************************************/
	 // Data

	 double m_t0 = SA_T0; ///< The start temperature, used in simulated annealing
	 double m_t = m_t0; ///< The current temperature, used in simulated annealing
	 double m_alpha = SA_ALPHA; ///< A constant used in the cooling schedule in simulated annealing

	 std::uint16_t m_n_threads = boost::numeric_cast<std::uint16_t>(Gem::Common::getNHardwareThreads(
		 Gem::Common::DEFAULTNHARDWARETHREADS
		 , Gem::Common::DEFAULTMAXNHARDWARETHREADS
	 )); ///< The number of threads

	 std::shared_ptr<Gem::Common::GThreadPool> m_tp_ptr; ///< Temporarily holds a thread pool

	 std::vector<std::shared_ptr<GParameterSet>> m_old_work_items; ///< Temporarily holds old returned work items

public:
	 /***************************************************************************/
	 /**
 	  * Applies modifications to this object. This is needed for testing purposes
 	  *
 	  * @return A boolean which indicates whether modifications were made
 	  */
	 bool modify_GUnitTests() override {
#ifdef GEM_TESTING

		 bool result = false;

		 // Call the parent class'es function
		 if(G_OA_ParChildT<executor_type>::modify_GUnitTests()) result = true;

	    return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GSimulatedAnnealingT<executor_type>::modify_GUnitTests", "GEM_TESTING");
		 return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
 	  * Performs self tests that are expected to succeed. This is needed for testing purposes
 	  */
	 virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent class'es function
		 G_OA_ParChildT<executor_type>::specificTestsNoFailureExpected_GUnitTests();

		 //------------------------------------------------------------------------------
		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GSimulatedAnnealingT<executor_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
 	  * Performs self tests that are expected to fail. This is needed for testing purposes
 	  */
	 virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent class'es function
		 G_OA_ParChildT<executor_type>::specificTestsFailuresExpected_GUnitTests();

		 //------------------------------------------------------------------------------
		 //------------------------------------------------------------------------------

#else /* GEM_TESTING */
		 condnotset("GSimulatedAnnealingT<executor_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
};

/******************************************************************************/
// Some typedefs for the different execution modes
using GBrokerSimulatedAnnealing = GSimulatedAnnealingT<Gem::Courtier::GBrokerExecutorT<Gem::Geneva::GParameterSet>>;
using GSerialSimulatedAnnealing = GSimulatedAnnealingT<Gem::Courtier::GSerialExecutorT<Gem::Geneva::GParameterSet>>;
using GMTSimulatedAnnealing = GSimulatedAnnealingT<Gem::Courtier::GMTExecutorT<Gem::Geneva::GParameterSet>>;

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBrokerSimulatedAnnealing)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSerialSimulatedAnnealing)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMTSimulatedAnnealing)

#endif /* G_OA_SIMULATEDANNEALING_HPP_ */
