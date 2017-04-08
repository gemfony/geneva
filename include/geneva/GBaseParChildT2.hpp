/**
 * @file GBaseParChildT2.hpp
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
#include <type_traits>
#include <tuple>

// Boost headers go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/range.hpp>

#ifndef GBASEPARCHILDT2_HPP_
#define GBASEPARCHILDT2_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GOptimizationAlgorithmT2.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GBaseParChildPersonalityTraits.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GBaseParChildT2<executor_type> class adds the notion of parents and children to
 * the GOptimizationAlgorithmT2<executor_type> class. An evolutionary adaptation is realized
 * through the cycle of adaption, evaluation, and sorting, as defined in this
 * class. It forms the basis for Evolutionary Algorithms as well as Simulated Annealing.
 *
 * Populations are collections of individuals, which themselves are objects
 * exhibiting at least the GOptimizableEntity class' API, most notably the
 * GOptimizableEntity::fitness() and GOptimizableEntity::adapt() functions.
 * You must add at least one GOptimizableEntity-derivative to the class.
 */
template <typename executor_type>
class GBaseParChildT2
	: public GOptimizationAlgorithmT2<executor_type>
{
	/////////////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GOptimizationAlgorithmT2_executor_type", boost::serialization::base_object<GOptimizationAlgorithmT2<executor_type>>(*this))
		& BOOST_SERIALIZATION_NVP(m_n_parents)
		& BOOST_SERIALIZATION_NVP(m_recombination_method)
		& BOOST_SERIALIZATION_NVP(m_default_n_children)
		& BOOST_SERIALIZATION_NVP(m_growth_rate)
		& BOOST_SERIALIZATION_NVP(m_max_population_size);
	}
	/////////////////////////////////////////////////////////////////////////////

	// Make sure executor_type is a derivative of GOptimizableEntity (or is GOptimizableEntity itself)
	static_assert(
		std::is_base_of<GOptimizableEntity, executor_type>::value
		, "GOptimizableEntity is no base type of executor_type"
	);

public:
	/***************************************************************************/
	/**
	 * The default constructor, As we do not have any individuals yet, we set the population
	 * size, and number of parents to a predefined value. It is the philosophy of this class not
	 * to provide constructors for each and every use case. Instead, you should set
	 * vital parameters, such as the population size or the parent individuals by hand
	 * or do so through a configuration file.
	 */
	GBaseParChildT2() : Gem::Geneva::GOptimizationAlgorithmT2<executor_type>()
	{
		// Make sure we start with a valid population size if the user does not supply these values
		this->setPopulationSizes(
			DEFPARCHILDPOPSIZE// overall population size
			, DEFPARCHILDNPARENTS // number of parents
		);
	}

	/***************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GBaseParChildT2<executor_type> object
	 */
	GBaseParChildT2(const GBaseParChildT2<executor_type>& cp)
		: GOptimizationAlgorithmT2<executor_type>(cp)
		, m_n_parents(cp.m_n_parents)
		, m_recombination_method(cp.m_recombination_method)
		, m_default_n_children(cp.m_default_n_children)
		, m_growth_rate(cp.m_growth_rate)
		, m_max_population_size(cp.m_max_population_size)
	{
		// Copying of individuals is done by the parent class
	}

	/***************************************************************************/
	/**
	 * The standard destructor. All work is done in the parent class.
	 */
	virtual ~GBaseParChildT2()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard assignment operator
	 */
	const GBaseParChildT2<executor_type>& operator=(
		const GBaseParChildT2<executor_type>& cp
	) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GBaseParChildT2<executor_type> object
	 *
	 * @param  cp A constant reference to another GBaseParChildT2<executor_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GBaseParChildT2<executor_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Checks for inequality with another GBaseParChildT2<executor_type> object
	 *
	 * @param  cp A constant reference to another GBaseParChildT2<executor_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GBaseParChildT2<executor_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
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
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GBaseParChildT2<executor_type>  reference independent of this object and convert the pointer
		const GBaseParChildT2<executor_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseParChildT2<executor_type>>(cp, this);

		GToken token("GBaseParChildT2<executor_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GOptimizationAlgorithmT2<executor_type>>(IDENTITY(*this, *p_load), token);

		// ... and then the local data
		compare_t(IDENTITY(m_n_parents, p_load->m_n_parents), token);
		compare_t(IDENTITY(m_recombination_method, p_load->m_recombination_method), token);
		compare_t(IDENTITY(m_default_n_children, p_load->m_default_n_children), token);
		compare_t(IDENTITY(m_max_population_size, p_load->m_max_population_size), token);
		compare_t(IDENTITY(m_growth_rate, p_load->m_growth_rate), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Specifies the default size of the population plus the number of parents.
	 * The population will be filled with additional individuals later, as required --
	 * see GBaseParChildT2<executor_type>::adjustPopulation() . Also, all error checking is done in
	 * that function.
	 *
	 * @param popSize The desired size of the population
	 * @param nParents The desired number of parents
	 */
	void setPopulationSizes(
		std::size_t popSize
		, std::size_t nParents
	) {
		GOptimizationAlgorithmT2<executor_type>::setDefaultPopulationSize(popSize);
		m_n_parents = nParents;
	}

	/***************************************************************************/
	/**
	 * Retrieve the number of parents as set by the user. This is a fixed parameter and
	 * should not be changed after it has first been set. Note that, if the size of the
	 * population is smaller than the alleged number of parents, the function will return
	 * the size of the population instead, thus interpreting its individuals as parents.
	 *
	 * @return The number of parents in the population
	 */
	std::size_t getNParents() const {
		return (std::min)(this->size(), m_n_parents);
	}

	/***************************************************************************/
	/**
	 * Calculates the current number of children from the number of parents and the
	 * size of the vector.
	 *
	 * @return The number of children in the population
	 */
	std::size_t getNChildren() const {
		if(this->size() <= m_n_parents) {
			// This will happen, when only the default population size has been set,
			// but no individuals have been added yet
			return 0;
		} else {
			return this->size() - m_n_parents;
		}
	}

	/***************************************************************************/
	/**
	 * Retrieves the m_default_n_children parameter. This factor may control when a
	 * population is considered to be complete.
	 *
	 * @return The m_default_n_children parameter
	 */
	std::size_t getDefaultNChildren() const {
		return m_default_n_children;
	}

	/**************************************************************************/
	/**
	 * Retrieve the number of processible items in the current iteration.
	 *
	 * @return The number of processible items in the current iteration
	 */
	virtual std::size_t getNProcessableItems() const override {
		std::tuple<std::size_t,std::size_t> range = this->getEvaluationRange();

#ifdef DEBUG
      if(std::get<1>(range) <= std::get<0>(range)) {
         glogger
         << "In GBaseParChildT2<>::getNProcessableItems(): Error!" << std::endl
         << "Upper boundary of range <= lower boundary: " << std::get<1>(range) << "/" << std::get<0>(range) << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

		return std::get<1>(range) - std::get<0>(range);
	}

	/***************************************************************************/
	/**
	 * Lets the user set the desired recombination method.
	 *
	 * @param recombinationMethod The desired recombination method
	 */
	void setRecombinationMethod(duplicationScheme recombinationMethod) {
#ifdef DEBUG
		if(duplicationScheme::DUPLICATIONSCHEME_LAST < recombinationMethod) {
			glogger
				<< "In GBaseParChildT2<>::setRecombinationMethod(): Error!" << std::endl
				<< "Got invalid duplication scheme " << recombinationMethod << std::endl
				<< GEXCEPTION;
		}
#endif

		m_recombination_method = recombinationMethod;
	}

	/***************************************************************************/
	/**
	 * Retrieves the value of the recombinationMethod_ variable
	 *
	 * @return The value of the recombinationMethod_ variable
	 */
	duplicationScheme getRecombinationMethod() const {
		return m_recombination_method;
	}

	/***************************************************************************/
	/**
	 * Adds the option to increase the population by a given amount per iteration
	 *
	 * @param growthRate The amount of individuals to be added in each iteration
	 * @param maxPopulationSize The maximum allowed size of the population
	 */
	void setPopulationGrowth(
		std::size_t growthRate
		, std::size_t maxPopulationSize
	) {
#ifdef DEBUG
		if(maxPopulationSize <= growthRate) {
			glogger
				<< "In GBaseParChildT2<>::setPopulationGrowth(): Error!" << std::endl
				<< "Got invalid growth rade " << growthRate << ", with maxPopulationSize = " << maxPopulationSize << std::endl
				<< GEXCEPTION;
		}
#endif

		m_growth_rate = growthRate;
		m_max_population_size = maxPopulationSize;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the growth rate of the population
	 *
	 * @return The growth rate of the population per iteration
	 */
	std::size_t getGrowthRate() const {
		return m_growth_rate;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the maximum population size when growth is enabled
	 *
	 * @return The maximum population size allowed, when growth is enabled
	 */
	std::size_t getMaxPopulationSize() const {
		return m_max_population_size;
	}

	/***************************************************************************/
	/**
	 * Adds local configuration options to a GParserBuilder object
	 *
	 * @param gpb The GParserBuilder object to which configuration options should be added
	 */
	virtual void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	)  override {
		// Call our parent class'es function
		GOptimizationAlgorithmT2<executor_type>::addConfigurationOptions(gpb);

		// Add local data

		gpb.registerFileParameter<std::size_t, std::size_t>(
			"size" // The name of the first variable
			, "nParents" // The name of the second variable
			, DEFAULTEAPOPULATIONSIZE
			, DEFAULTEANPARENTS
			, [this](std::size_t ps, std::size_t np){ this->setPopulationSizes(ps, np); }
			, "population"
		)
		<< "The total size of the population "
		<< Gem::Common::nextComment()
		<< "The number of parents in the population";

		gpb.registerFileParameter<duplicationScheme>(
			"recombinationMethod" // The name of the variable
			, duplicationScheme::DEFAULTDUPLICATIONSCHEME // The default value
			, [this](duplicationScheme d){ this->setRecombinationMethod(d); }
		)
		<< "The recombination method. Options" << std::endl
		<< "0: default" << std::endl
		<< "1: random selection from available parents" << std::endl
		<< "2: selection according to the parent's value";

		gpb.registerFileParameter<std::size_t, std::size_t>(
			"growthRate" // The name of the variable
			, "maxPopulationSize" // The name of the variable
			, 0 // The default value of the first variable
			, 0 // The default value of the second variable
			, [this](std::size_t gr, std::size_t ms){ this->setPopulationGrowth(gr,ms); }
			, "populationGrowth"
		)
		<< "Specifies the number of individuals added per iteration" << Gem::Common::nextComment()
		<< "Specifies the maximum amount of individuals in the population" << std::endl
		<< "if growth is enabled";
	}

	/***************************************************************************/
	/**
	 * Retrieves a specific parent individual and casts it to the desired type. Note that this
	 * function will only be accessible to the compiler if parent_type is a derivative of
	 * GOptimizableEntity, thanks to the magic of the std::enable_if and type_traits.
	 *
	 * @param parent The id of the parent that should be returned
	 * @return A converted shared_ptr to the parent
	 */
	template <typename parent_type>
	std::shared_ptr<parent_type> getParentIndividual(
		std::size_t parentId
		, typename std::enable_if<std::is_base_of<GOptimizableEntity, parent_type>::value>::type *dummy = nullptr
	){
#ifdef DEBUG
      // Check that the parent id is in a valid range
      if(parentId >= this->getNParents()) {
         glogger
         << "In GBaseParChildT2::getParentIndividual<>() : Error" << std::endl
         << "Requested parent id which does not exist: " << parentId << " / " << this->getNParents() << std::endl
         << GEXCEPTION;

         // Make the compiler happy
         return std::shared_ptr<parent_type>();
      }
#endif /* DEBUG */

		// Does error checks on the conversion internally
		return Gem::Common::convertSmartPointer<GOptimizableEntity, parent_type>(*(this->begin() + parentId));
	}

	/***************************************************************************/
	/** @brief Returns the name of this optimization algorithm */
	virtual std::string getAlgorithmName() const override = 0;
	/** @brief Returns a mnemonic for the optimization algorithm */
	virtual std::string getOptimizationAlgorithm() const override = 0;

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GBaseParChildT2");
	}

protected:
	/***************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone_() const override = 0;
	/** @brief Adapts all children of this population */
	virtual void adaptChildren() = 0;
	/** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
	virtual void runFitnessCalculation() override = 0;
	/** @brief Choose new parents, based on the selection scheme set by the user */
	virtual void selectBest() = 0;
	/** @brief Retrieves the evaluation range in a given iteration and sorting scheme */
	virtual std::tuple<std::size_t,std::size_t> getEvaluationRange() const = 0; // Depends on selection scheme
	/** @brief Some error checks related to population sizes */
	virtual void populationSanityChecks() const = 0;

	/***************************************************************************/
	/**
	 * Loads the data of another GBaseParChildT2<executor_type> object, camouflaged as a GObject.
	 *
	 * @param cp A pointer to another GBaseParChildT2<executor_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject * cp) override {
		// Check that we are dealing with a GBaseParChildT2<executor_type>  reference independent of this object and convert the pointer
		const GBaseParChildT2<executor_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseParChildT2<executor_type>>(cp, this);

		// First load the parent class'es data ...
		GOptimizationAlgorithmT2<executor_type>::load_(cp);

		// ... and then our own data
		m_n_parents = p_load->m_n_parents;
		m_recombination_method = p_load->m_recombination_method;
		m_default_n_children = p_load->m_default_n_children;
		m_max_population_size = p_load->m_max_population_size;
		m_growth_rate = p_load->m_growth_rate;
	}

	/***************************************************************************/
	/**
	 * This function assigns a new value to each child individual according to the chosen
	 * recombination scheme. Note that this function may be overloaded in derived classes,
	 * to e.g. add features such as cross-over.
	 */
	virtual void doRecombine() {
		std::size_t i;
		std::vector<double> threshold(m_n_parents);
		double thresholdSum=0.;
		// The number of parents may change, e.g. in the case of pareto-optimization. Hence we may
		// need to recalculate the weight vector.
		if(duplicationScheme::VALUEDUPLICATIONSCHEME == m_recombination_method && m_n_parents > 1) {          // Calculate a weight vector
			for(i=0; i<m_n_parents; i++) {
				thresholdSum += 1./(static_cast<double>(i)+2.);
			}
			for(i=0; i<m_n_parents-1; i++) {
				// Normalizing the sum to 1
				threshold[i] = (1./(static_cast<double>(i)+2.)) / thresholdSum;

				// Make sure the subsequent range is in the right position
				if(i>0) threshold[i] += threshold[i-1];
			}
			threshold[m_n_parents-1] = 1.; // Necessary due to rounding errors
		}

		for(auto item: boost::make_iterator_range(GOptimizationAlgorithmT2<executor_type>::data.begin()+m_n_parents, GOptimizationAlgorithmT2<executor_type>::data.end())) {
			switch(m_recombination_method){
				case duplicationScheme::DEFAULTDUPLICATIONSCHEME: // we want the RANDOMDUPLICATIONSCHEME behavior
				case duplicationScheme::RANDOMDUPLICATIONSCHEME:
				{
					randomRecombine(item);
				}
					break;

				case duplicationScheme::VALUEDUPLICATIONSCHEME:
				{
					if(m_n_parents == 1) {
						item->GObject::load(GOptimizationAlgorithmT2<executor_type>::data.begin());
						item->GOptimizableEntity::template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setParentId(0);
					} else {
						// A recombination taking into account the value does not make
						// sense in the first iteration, as parents might not have a suitable
						// value. Instead, this function might accidently trigger value
						// calculation. Hence we fall back to random recombination in iteration 0.
						// No value calculation takes place there.
						if(GOptimizationAlgorithmT2<executor_type>::inFirstIteration()) {
							randomRecombine(item);
						} else {
							valueRecombine(item, threshold);
						}
					}
				}
					break;

				default:
				{
					glogger
						<< "In GBaseParChildT2<executor_type>::doRecombine(): Error!" << std::endl
						<< "Got invalid duplication scheme: " << m_recombination_method << std::endl
						<< GEXCEPTION;
				}
					break;
			}
		}
	}

	/***************************************************************************/
	/**
	 * This function is called from GOptimizationAlgorithmT2<executor_type>::optimize() and performs the
	 * actual recombination, based on the recombination schemes defined by the user.
	 *
	 * Note that, in DEBUG mode, this implementation will enforce a minimum number of children,
	 * as implied by the initial sizes of the population and the number of parents
	 * present. If individuals can get lost in your setting, you must add mechanisms
	 * to "repair" the population.
	 */
	virtual void recombine() {
#ifdef DEBUG
      // We require at this stage that at least the default number of
      // children is present. If individuals can get lost in your setting,
      // you must add mechanisms to "repair" the population.
      if((this->size()-m_n_parents) < m_default_n_children){
         glogger
         << "In GBaseParChildT2<executor_type>::recombine():" << std::endl
         << "Too few children. Got " << this->size()-m_n_parents << "," << std::endl
         << "but was expecting at least " << m_default_n_children << std::endl
         << GEXCEPTION;
      }
#endif

		// Do the actual recombination
		doRecombine();

		// Let children know they are children
		markChildren();

		// Tell individuals about their ids
		markIndividualPositions();
	}

	/***************************************************************************/
	/**
	 * Retrieves the adaption range in a given iteration and sorting scheme.
	 *
	 * @return The range inside which adaption should take place
	 */
	std::tuple<std::size_t,std::size_t> getAdaptionRange() const {
		return std::tuple<std::size_t, std::size_t>(m_n_parents, this->size());
	}

	/***************************************************************************/
	/**
	 * This helper function marks parents as parents
	 */
	void markParents() {
		for(auto item: boost::make_iterator_range(GOptimizationAlgorithmT2<executor_type>::data.begin(), GOptimizationAlgorithmT2<executor_type>::data.end()+m_n_parents)) {
			item->GOptimizableEntity::template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setIsParent();
		}
	}

	/***************************************************************************/
	/**
	 * This helper function marks children as children
	 */
	void markChildren() {
		for(auto item: boost::make_iterator_range(GOptimizationAlgorithmT2<executor_type>::data.begin()+m_n_parents, GOptimizationAlgorithmT2<executor_type>::data.end())) {
			item->GOptimizableEntity::template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setIsChild();
		}
	}

	/***************************************************************************/
	/**
	 * This helper function lets all individuals know about their position in the
	 * population.
	 */
	void markIndividualPositions() {
		std::size_t pos = 0;
		for(auto ind: GOptimizationAlgorithmT2<executor_type>::data) {
			ind->GOptimizableEntity::template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setPopulationPosition(pos++);
		}
	}

	/***************************************************************************/
	/**
	 * This function implements the logic that constitutes evolutionary algorithms. The
	 * function is called by GOptimizationAlgorithmT2<executor_type> for each cycle of the optimization,
	 *
	 * @return The value of the best individual found
	 */
	virtual std::tuple<double, double> cycleLogic() override {
		// If this is not the first iteration, check whether we need to increase the population
		if(GOptimizationAlgorithmT2<executor_type>::afterFirstIteration()) {
			performScheduledPopulationGrowth();
		}

		// create new children from parents
		recombine();

		// adapt children
		adaptChildren();

		// calculate the children's (and possibly their parents' values)
		runFitnessCalculation();

		// Perform post-evaluation updates (mostly of individuals)
		GOptimizationAlgorithmT2<executor_type>::postEvaluationWork();

		// find out the best individuals of the population
		selectBest();

#ifdef DEBUG
      // The dirty flag of the fiest individual shouldn't be set
      if(!this->at(0)->isClean()) {
         glogger
         << "In GBaseParChildT2<>::cycleLogic(): Error!" << std::endl
         << "Expected clean individual in best position" << std::endl
         << GEXCEPTION;
      }

#endif /* DEBUG */

		// Return the primary fitness of the best individual in the collection
		return this->at(0)->getFitnessTuple();
	}

	/***************************************************************************/
	/**
	 * The function checks that the population size meets the requirements and does some
	 * tagging. It is called from within GOptimizationAlgorithmT2<executor_type>::optimize(), before the
	 * actual optimization cycle starts.
	 */
	virtual void init() override {
		// To be performed before any other action
		GOptimizationAlgorithmT2<executor_type>::init();

		// Perform some checks regarding population sizes
		populationSanityChecks();

		// Let parents know they are parents
		markParents();
		// Let children know they are children

		// Make sure derived classes have a way of finding out what the desired number of children is.
		// This is particularly important, if, in a network environment, some individuals might not
		// return and some individuals return late. The factual size of the population then changes and we
		// need to take action.
		m_default_n_children = GOptimizationAlgorithmT2<executor_type>::getDefaultPopulationSize() - m_n_parents;


	}

	/***************************************************************************/
	/**
	 * Does any necessary finalization work
	 */
	virtual void finalize() override {
		// Last action
		GOptimizationAlgorithmT2<executor_type>::finalize();
	}

	/***************************************************************************/
	/**
	 * The function checks that the population size meets the requirements and resizes the
	 * population to the appropriate size, if required. An obvious precondition is that at
	 * least one individual has been added to the population. Individuals that have already
	 * been added will not be replaced. This function is called once before the optimization
	 * cycle from within GOptimizationAlgorithmT2<executor_type>::optimize()
	 */
	virtual void adjustPopulation() override {
		// Has the population size been set at all ?
		if(GOptimizationAlgorithmT2<executor_type>::getDefaultPopulationSize() == 0) {
			glogger
			<< "In GBaseParChildT2<executor_type>::adjustPopulation() :" << std::endl
			<< "The population size is 0." << std::endl
			<< "Did you call GOptimizationAlgorithmT2<executor_type>::setParentsAndPopulationSize() ?" << std::endl
			<< GEXCEPTION;
		}

		// Check how many individuals have been added already. At least one is required.
		std::size_t this_sz = this->size();
		if(this_sz == 0) {
			glogger
			<< "In GBaseParChildT2<executor_type>::adjustPopulation() :" << std::endl
			<< "size of population is 0. Did you add any individuals?" << std::endl
			<< "We need at least one local individual" << std::endl
			<< GEXCEPTION;
		}

		// Do the smart pointers actually point to any objects ?
		for(auto item: GOptimizationAlgorithmT2<executor_type>::data) {
			if(!item) { // shared_ptr can be implicitly converted to bool
				glogger
					<< "In GBaseParChildT2<executor_type>::adjustPopulation() :" << std::endl
					<< "Found empty smart pointer." << std::endl
					<< GEXCEPTION;
			}
		}

		// Fill up as required. We are now sure we have a suitable number of individuals to do so
		if(this_sz < GOptimizationAlgorithmT2<executor_type>::getDefaultPopulationSize()) {
			this->resize_clone(
				GOptimizationAlgorithmT2<executor_type>::getDefaultPopulationSize()
				, GOptimizationAlgorithmT2<executor_type>::data[0]
			);

			// Randomly initialize new items
			for(auto item: boost::make_iterator_range(GOptimizationAlgorithmT2<executor_type>::data.begin()+this_sz, GOptimizationAlgorithmT2<executor_type>::data.end())) {
				item->randomInit(activityMode::ACTIVEONLY);
			}
		}
	}

	/***************************************************************************/
	/**
	 * Increases the population size if requested by the user. This will happen until the population size exceeds
	 * a predefined value, set with setPopulationGrowth() .
	 */
	void performScheduledPopulationGrowth() {
		if(
			m_growth_rate != 0
			&& (this->getDefaultPopulationSize() + m_growth_rate <= m_max_population_size)
			&& (this->size() < m_max_population_size)
		) {
			// Set a new default population size
			this->setPopulationSizes(this->getDefaultPopulationSize() + m_growth_rate, this->getNParents());

			// Add missing items as copies of the last individual in the list
			this->resize_clone(GOptimizationAlgorithmT2<executor_type>::getDefaultPopulationSize(), GOptimizationAlgorithmT2<executor_type>::data[0]);
		}
	}

	/***************************************************************************/
	/**
	 * This function implements the RANDOMDUPLICATIONSCHEME scheme.
	 *
	 * @param pos The position of the individual for which a new value should be chosen
	 */
	void randomRecombine(std::shared_ptr<executor_type>& child) {
		std::size_t parent_pos;

		if(m_n_parents==1) {
			parent_pos = 0;
		} else {
			// Choose a parent to be used for the recombination. uniform_int(0, max)
			// returns integer values in the range [0,max]. As we want to have values in the range
			// 0,1, ... m_n_parents-1, we need to subtract one from the argument.
			parent_pos = m_uniform_int_distribution(
				GOptimizationAlgorithmT2<executor_type>::m_gr
				, std::uniform_int_distribution<std::size_t>::param_type(0, m_n_parents-1)
			);
		}

		// Load the parent data into the individual
		child->GObject::load(GOptimizationAlgorithmT2<executor_type>::data.begin() + parent_pos);

		// Let the individual know the id of the parent
		child->GOptimizableEntity::template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setParentId(parent_pos);
	}

	/***************************************************************************/
	/**
	 * This function implements the VALUEDUPLICATIONSCHEME scheme. The range [0.,1.[ is divided
	 * into nParents_ sub-areas with different size (the largest for the first parent,
	 * the smallest for the last). Parents are chosen for recombination according to a
	 * random number evenly distributed between 0 and 1. This way parents with higher
	 * fitness are more likely to be chosen for recombination.
	 *
	 * @param pos The child individual for which a parent should be chosen
	 * @param threshold A std::vector<double> holding the recombination likelihoods for each parent
	 */
	void valueRecombine(
		std::shared_ptr<executor_type>& p
		, const std::vector<double>& threshold
	) {
		bool done=false;
		double randTest // get the test value
            = GOptimizationAlgorithmT2<executor_type>::m_uniform_real_distribution(GOptimizationAlgorithmT2<executor_type>::m_gr);

		for(std::size_t par=0; par<m_n_parents; par++) {
			if(randTest<threshold[par]) {
				// Load the parent's data
				p->GObject::load(GOptimizationAlgorithmT2<executor_type>::data.begin() + par);
				// Let the individual know the parent's id
				p->GOptimizableEntity::template getPersonalityTraits<GBaseParChildPersonalityTraits>()->setParentId(par);
				done = true;

				break;
			}
		}

		if(!done) {
			glogger
			<< "In GBaseParChildT2<executor_type>::valueRecombine():" << std::endl
			<< "Could not recombine." << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * Selection, MUPLUSNU_SINGLEEVAL style. Note that not all individuals of the population (including parents)
	 * are sorted -- only the nParents best individuals are identified. The quality of the population can only
	 * increase, but the optimization will stall more easily in MUPLUSNU_SINGLEEVAL mode.
	 */
	void sortMuPlusNuMode() {
#ifdef DEBUG
      // Check that we do not accidently trigger value calculation
      std::size_t pos = 0;
      for(auto ind_ptr: *this) { // std::shared_ptr may be copied
         if(ind_ptr->isDirty()) {
            glogger
            << "In GBaseParChildT2<executor_type>::sortMuplusnuMode(): Error!" << std::endl
            << "In iteration " << GOptimizationAlgorithmT2<executor_type>::getIteration() << ": Found individual in position " << pos << std::endl
            << " whose dirty flag is set." << std::endl
            << GEXCEPTION;
         }
         pos++;
      }
#endif /* DEBUG */

		// Only partially sort the arrays
		std::partial_sort(
			GOptimizationAlgorithmT2<executor_type>::data.begin()
			, GOptimizationAlgorithmT2<executor_type>::data.begin() + m_n_parents
			, GOptimizationAlgorithmT2<executor_type>::data.end()
			, [](std::shared_ptr<executor_type> x, std::shared_ptr<executor_type> y) -> bool {
				return x->minOnly_fitness() < y->minOnly_fitness();
			}
		);
	}

	/***************************************************************************/
	/**
	 * Selection, MUCOMMANU_SINGLEEVAL style. New parents are selected from children only. The quality
	 * of the population may decrease occasionally from generation to generation, but the
	 * optimization is less likely to stall.
	 */
	void sortMuCommaNuMode() {
#ifdef DEBUG
		if (GOptimizationAlgorithmT2<executor_type>::inFirstIteration()) {
			// Check that we do not accidently trigger value calculation -- check the whole range
			typename GBaseParChildT2<executor_type>::iterator it;
			for (it = this->begin(); it != this->end(); ++it) {
				if ((*it)->isDirty()) {
					glogger
						<< "In GBaseParChildT2<executor_type>::sortMucommanuMode(): Error!" << std::endl
						<< "In iteration " << GOptimizationAlgorithmT2<executor_type>::getIteration() << ": Found individual in position " << std::distance(
						this->begin()
						, it
					) << std::endl
						<< " whose dirty flag is set." << std::endl
						<< GEXCEPTION;
				}
			}
		} else {
			// Check that we do not accidently trigger value calculation -- check children only
			typename GBaseParChildT2<executor_type>::iterator it;
			for (it = this->begin() + m_n_parents; it != this->end(); ++it) {
				if ((*it)->isDirty()) {
					glogger
						<< "In GBaseParChildT2<executor_type>::sortMucommanuMode(): Error!" << std::endl
						<< "In iteration " << GOptimizationAlgorithmT2<executor_type>::getIteration() << ": Found individual in position " << std::distance(
						this->begin()
						, it
					) << std::endl
						<< " whose dirty flag is set." << std::endl
						<< GEXCEPTION;
				}
			}
		}
#endif /* DEBUG */

		if (GOptimizationAlgorithmT2<executor_type>::inFirstIteration()) {
			// We fall back to MUPLUSNU mode in the first iteration,
			// as parents are new as well.
			this->sortMuPlusNuMode();
			return;
		} else {
			// Only sort the children
			std::partial_sort(
				GOptimizationAlgorithmT2<executor_type>::data.begin() + m_n_parents
				, GOptimizationAlgorithmT2<executor_type>::data.begin() + 2 * m_n_parents
				, GOptimizationAlgorithmT2<executor_type>::data.end()
				, [](std::shared_ptr<executor_type> x, std::shared_ptr<executor_type> y) -> bool {
					return x->minOnly_fitness() < y->minOnly_fitness();
				}
			);

			std::swap_ranges(
				GOptimizationAlgorithmT2<executor_type>::data.begin()
				, GOptimizationAlgorithmT2<executor_type>::data.begin() + m_n_parents
				, GOptimizationAlgorithmT2<executor_type>::data.begin() + m_n_parents
			);
		}
	}

	/***************************************************************************/
	/**
	 * Selection, MUNU1PRETAIN_SINGLEEVAL style. This is a hybrid between MUPLUSNU_SINGLEEVAL and MUCOMMANU_SINGLEEVAL
	 * mode. If a better child was found than the best parent of the last generation,
	 * all former parents are replaced. If no better child was found than the best
	 * parent of the last generation, then this parent stays in place. All other parents
	 * are replaced by the (nParents_-1) best children. The scheme falls back to MUPLUSNU_SINGLEEVAL
	 * mode, if only one parent is available, or if this is the first generation (so we
	 * do not accidentally trigger value calculation).
	 */
	void sortMunu1pretainMode() {
#ifdef DEBUG
      // Check that we do not accidently trigger value calculation
		std::size_t pos = 0;
		for(auto item: boost::make_iterator_range(this->begin()+m_n_parents, this->end())) {
			if(item->isDirty()) {
				glogger
					<< "In GBaseParChildT2<executor_type>::sortMunu1pretainMode(): Error!" << std::endl
					<< "In iteration " << GOptimizationAlgorithmT2<executor_type>::getIteration() << ": Found individual in position " << pos << std::endl
					<< " whose dirty flag is set." << std::endl
					<< GEXCEPTION;
			}

			pos++;
		}
#endif /* DEBUG */

		if(m_n_parents==1 || GOptimizationAlgorithmT2<executor_type>::inFirstIteration()) { // Falls back to MUPLUSNU_SINGLEEVAL mode
			sortMuPlusNuMode();
		} else {
			// Sort the children
			std::partial_sort(
				GOptimizationAlgorithmT2<executor_type>::data.begin() + m_n_parents
				, GOptimizationAlgorithmT2<executor_type>::data.begin() + 2*m_n_parents
				, GOptimizationAlgorithmT2<executor_type>::data.end()
				, [](std::shared_ptr<executor_type> x, std::shared_ptr<executor_type> y) -> bool {
					return x->minOnly_fitness() < y->minOnly_fitness();
				}
			);

			// Retrieve the best child's and the last generation's best parent's fitness
			double bestTranformedChildFitness_MinOnly  = (*(GOptimizationAlgorithmT2<executor_type>::data.begin() + m_n_parents))->minOnly_fitness();
			double bestTranformedParentFitness_MinOnly = (*(GOptimizationAlgorithmT2<executor_type>::data.begin()))->minOnly_fitness();

			// Leave the best parent in place, if no better child was found
			if(bestTranformedChildFitness_MinOnly < bestTranformedParentFitness_MinOnly) { // A better child was found. Overwrite all parents
				std::swap_ranges(
					GOptimizationAlgorithmT2<executor_type>::data.begin()
					,GOptimizationAlgorithmT2<executor_type>::data.begin()+m_n_parents
					,GOptimizationAlgorithmT2<executor_type>::data.begin()+m_n_parents
				);
			} else {
				std::swap_ranges(
					GOptimizationAlgorithmT2<executor_type>::data.begin()+1
					, GOptimizationAlgorithmT2<executor_type>::data.begin()+m_n_parents
					, GOptimizationAlgorithmT2<executor_type>::data.begin()+m_n_parents
				);
			}
		}
	}

	/***************************************************************************/

	std::size_t m_n_parents = DEFPARCHILDNPARENTS; ///< The number of parents
	duplicationScheme m_recombination_method = duplicationScheme::DEFAULTDUPLICATIONSCHEME; ///< The chosen recombination method
	std::size_t m_default_n_children = DEFPARCHILDNCHILDREN; ///< Expected number of children
	std::size_t m_growth_rate = 0; ///< Specifies the amount of individuals added per iteration
	std::size_t m_max_population_size = 0; ///< Specifies the maximum amount of individuals in the population if growth is enabled

private:
   std::uniform_int_distribution<std::size_t> m_uniform_int_distribution; ///< Access to uniformly distributed random numbers

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING

      bool result = false;

      // Call the parent class'es function
      if(GOptimizationAlgorithmT2<executor_type>::modify_GUnitTests()) result = true;

      return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GBaseParChildT2<executor_type>::modify_GUnitTests", "GEM_TESTING");
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
      GOptimizationAlgorithmT2<executor_type>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GBaseParChildT2<executor_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
      // Call the parent class'es function
      GOptimizationAlgorithmT2<executor_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */
		condnotset("GBaseParChildT2<executor_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename executor_type>
struct is_abstract<Gem::Geneva::GBaseParChildT2<executor_type>> : public boost::true_type {};
template<typename executor_type>
struct is_abstract< const Gem::Geneva::GBaseParChildT2<executor_type>> : public boost::true_type {};
}
}

/******************************************************************************/

#endif /* GBASEPARCHILDT2_HPP_ */
