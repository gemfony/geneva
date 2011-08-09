/**
 * @file GSerialEA.hpp
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



// Standard headers go here

// Boost headers go here

#ifndef GEVOLUTIONARYALGORITHM_HPP_
#define GEVOLUTIONARYALGORITHM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "geneva/GIndividual.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"

#ifdef GENEVATESTING
#include "geneva-individuals/GTestIndividual1.hpp"
#endif /* GENEVATESTING */

namespace Gem {
namespace Geneva {

// Forward declaration
class GEAOptimizationMonitor;

/**
 * The default sorting mode
 */
const sortingMode DEFAULTSMODE=MUPLUSNU_SINGLEEVAL;

/**
 * The default number of generations without improvement after which
 * a micro-training should be started. A value of 0 means that no
 * micro-training will take place.
 */
const boost::uint32_t DEFAULTMICROTRAININGINTERVAL=0;

/*********************************************************************************/
/**
 * The GSerialEA class adds the notion of parents and children to
 * the GOptimizationAlgorithmT class. The evolutionary adaptation is realized
 * through the cycle of adaption, evaluation, and sorting, as defined in this
 * class.
 *
 * Populations are collections of individuals, which themselves are objects
 * exhibiting the GIndividual class' API, most notably the GIndividual::fitness() and
 * GIndividual::adapt() functions. Individuals can thus themselves be populations,
 * which can again contain populations, and so on.
 *
 * In order to add parents to an instance of this class use the default constructor,
 * then add at least one GIndividual to it, and call setDefaultPopulationSize(). The population
 * will then be "filled up" with missing individuals as required, before the optimization
 * starts. Note that this class will enforce a minimum, default number of children,
 * as implied by the population size and the number of parents set at the beginning.
 */
class GSerialEA
	:public GOptimizationAlgorithmT<GIndividual>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("GOptimizationAlgorithmT_GIndividual", boost::serialization::base_object<GOptimizationAlgorithmT<GIndividual> >(*this))
		   & BOOST_SERIALIZATION_NVP(nParents_)
		   & BOOST_SERIALIZATION_NVP(microTrainingInterval_)
		   & BOOST_SERIALIZATION_NVP(recombinationMethod_)
		   & BOOST_SERIALIZATION_NVP(smode_)
		   & BOOST_SERIALIZATION_NVP(defaultNChildren_)
		   & BOOST_SERIALIZATION_NVP(oneTimeMuCommaNu_)
		   & BOOST_SERIALIZATION_NVP(growthRate_)
		   & BOOST_SERIALIZATION_NVP(maxPopulationSize_)
		   & BOOST_SERIALIZATION_NVP(logOldParents_)
		   & BOOST_SERIALIZATION_NVP(t0_)
		   & BOOST_SERIALIZATION_NVP(t_)
		   & BOOST_SERIALIZATION_NVP(alpha_)
		   & BOOST_SERIALIZATION_NVP(oldParents_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GSerialEA();
	/** @brief A standard copy constructor */
	GSerialEA(const GSerialEA&);
	/** @brief The destructor */
	virtual ~GSerialEA();

	/** @brief A standard assignment operator */
	const GSerialEA& operator=(const GSerialEA&);

	/** @brief Checks for equality with another GSerialEA object */
	bool operator==(const GSerialEA&) const;
	/** @brief Checks for inequality with another GSerialEA object */
	bool operator!=(const GSerialEA&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Sets the default population size and number of parents */
	void setDefaultPopulationSize(std::size_t, std::size_t);

	/** @brief Retrieve the number of parents in this population */
	std::size_t getNParents() const;
	/** @brief Retrieve the number of children in this population */
	std::size_t getNChildren() const;
	/** @brief Retrieves the defaultNChildren_ parameter */
	std::size_t getDefaultNChildren() const;

	/** @brief Set the sorting scheme for this population */
	void setSortingScheme(sortingMode);
	/** @brief Retrieve the current sorting scheme for this population */
	sortingMode getSortingScheme() const;

	/** @brief Specify, what recombination mode should be used */
	void setRecombinationMethod(recoScheme);
	/** @brief Find out, what recombination mode is being used */
	recoScheme getRecombinationMethod() const;

	/** @brief Loads a checkpoint from disk */
	virtual void loadCheckpoint(const std::string&);

	/** @brief Instruct the algorithm whether it should log old parents for one iteration */
	void setLogOldParents(const bool&);
	/** @brief Retrieves the current value of the logOldParents_ flag */
	bool oldParentsLogged() const;

	/** @brief Retrieves the number of processable items for the current iteration */
	virtual std::size_t getNProcessableItems() const;

	/** @brief Adds the option to increase the population by a given amount per iteration */
	void setPopulationGrowth(const std::size_t&, const std::size_t&);
	/** @brief Allows to retrieve the growth rate of the population */
	std::size_t getGrowthRate() const;
	/** @brief Allows to retrieve the maximum population size when growth is enabled */
	std::size_t getMaxPopulationSize() const;

	//------------------------------------------------------------------------------------------
	// Settings specific to micro-training
	/** @brief Set the interval in which micro training should be performed */
	void setMicroTrainingInterval(const boost::uint32_t&);
	/** @brief Retrieve the interval in which micro training should be performed */
	boost::uint32_t getMicroTrainingInterval() const;

	//------------------------------------------------------------------------------------------
	// Settings for simulated annealing
	/** @brief Determines the strength of the temperature degradation */
	void setTDegradationStrength(const double&);
	/** @brief Retrieves the temperature degradation strength */
	double getTDegradationStrength() const;
	/** @brief Sets the start temperature */
	void setT0(const double&);
	/** @brief Retrieves the start temperature */
	double getT0() const;
	/** @brief Retrieves the current temperature */
	double getT() const;

	/**************************************************************************************************/
	/**
	 * Retrieves a specific parent individual and casts it to the desired type. Note that this
	 * function will only be accessible to the compiler if individual_type is a derivative of GIndividual,
	 * thanks to the magic of Boost's enable_if and Type Traits libraries.
	 *
	 * @param parent The id of the parent that should be returned
	 * @return A converted shared_ptr to the parent
	 */
	template <typename individual_type>
	inline boost::shared_ptr<individual_type> getParentIndividual(
			std::size_t parentId
		  , typename boost::enable_if<boost::is_base_of<GIndividual, individual_type> >::type* dummy = 0
	){
#ifdef DEBUG
		// Check that the parent id is in a valid range
		if(parentId >= this->getNParents()) {
			raiseException(
					"In GSerialEA::getParentIndividual<>() : Error" << std::endl
					<< "Requested parent id which does not exist: " << parentId << " / " << this->getNParents()
			);
		}

		// Retrieve a pointer to the parent individual
		boost::shared_ptr<GIndividual> parent_ptr = *(this->begin() + parentId);

		// Check that the pointer actually points somewhere
		if(!parent_ptr) {
			raiseException(
					"In GSerialEA::getParentIndividual<>() : Error" << std::endl
					<< "Tried to access uninitialized parent individual."
			);
		}

		boost::shared_ptr<individual_type> converted_parent_pointer = boost::dynamic_pointer_cast<individual_type>(parent_ptr);

		if(converted_parent_pointer) return converted_parent_pointer;
		else {
			raiseException(
					"In GSerialEA::getParentIndividual<>(): Conversion error"
			);
		}
#else
		return boost::static_pointer_cast<individual_type>(*(this->begin() + parentId));
#endif /* DEBUG */
	}

	/**************************************************************************************************/
	/**
	 * Retrieves a specific (logged) parent individual and casts it to the desired type. Note that this
	 * function will only be accessible to the compiler if individual_type is a derivative of GIndividual,
	 * thanks to the magic of Boost's enable_if and Type Traits libraries.
	 *
	 * @param parent The id of the logged parent that should be returned
	 * @return A converted shared_ptr to the parent
	 */
	template <typename individual_type>
	inline boost::shared_ptr<individual_type> getOldParentIndividual(
			std::size_t parentId
		  , typename boost::enable_if<boost::is_base_of<GIndividual, individual_type> >::type* dummy = 0
	){
#ifdef DEBUG
		// Check that we have indeed logged parents
		if(!logOldParents_) {
			raiseException(
					"In GSerialEA::getOldParentIndividual<>() : Error" << std::endl
					<< "Requested logged parent individual when no logging is done"
			);
		}

		// Check that the parent id is in a valid range
		if(parentId >= oldParents_.size()) {
			raiseException(
					"In GSerialEA::getOldParentIndividual<>() : Error" << std::endl
					<< "Requested parent id which does not exist: " << parentId << " / " << oldParents_.size()
			);
		}

		// Retrieve a pointer to the parent individual
		boost::shared_ptr<GIndividual> parent_ptr = *(oldParents_.begin() + parentId);

		// Check that the pointer actually points somewhere
		if(!parent_ptr) {
			raiseException(
					"In GSerialEA::getOldParentIndividual<>() : Error" << std::endl
					<< "Tried to access uninitialized parent individual."
			);
		}

		boost::shared_ptr<individual_type> converted_parent_pointer = boost::dynamic_pointer_cast<individual_type>(parent_ptr);

		if(converted_parent_pointer) return converted_parent_pointer;
		else {
			raiseException(
					"In GSerialEA::getOldParentIndividual<>() : Conversion error"
			);
		}
#else
		return boost::static_pointer_cast<individual_type>(*(oldParents_.begin() + parentId));
#endif /* DEBUG */
	}

	/**************************************************************************************************/

protected:
	/** @brief Loads the data of another population */
	virtual void load_(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone_() const;
	/** @brief Allows to set the personality type of the individuals */
	virtual void setIndividualPersonalities();

	/** @brief user-defined recombination scheme */
	void doRecombine();

	/** @brief Creates children from parents according to a predefined recombination scheme */
	virtual void recombine();
	/** @brief Adapts all children of this population */
	virtual void adaptChildren();
	/** @brief Selects the best children of the population */
	virtual void select();

	/** @brief Marks parents as parents and children as children */
	void markParents();
	/** @brief Marks children as children */
	void markChildren();
	/** @brief Lets individuals know about their position in the population */
	void markIndividualPositions();

	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual double cycleLogic();
	/** @brief Does some preparatory work before the optimization starts */
	virtual void init();
	/** @brief Does any necessary finalization work */
	virtual void finalize();

	/** @brief Resizes the population to the desired level and does some error checks */
	virtual void adjustPopulation();
	/** @brief Increases the population size if requested by the user */
	void performScheduledPopulationGrowth();

	/** @brief Retrieves the best individual found */
	virtual boost::shared_ptr<GIndividual> getBestIndividual();

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
		, const bool& showOrigin
	);

private:
	/**********************************************************************************/
    /**
     * A simple comparison operator that helps to sort individuals according to their
     * pareto status
     */
    class indParetoComp {
    public:
    	bool operator()(boost::shared_ptr<GIndividual> x, boost::shared_ptr<GIndividual> y) {
    		return x->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront() > y->getPersonalityTraits<GEAPersonalityTraits>()->isOnParetoFront();
    	}
    };

    /**********************************************************************************/
	/** @brief Enforces a one-time selection policy of MUCOMMANU_SINGLEEVAL */
	void setOneTimeMuCommaNu();
	/** @brief Updates the parent's structure */
	bool updateParentStructure();

	/** @brief Saves the state of the class to disc. Private, as we do not want to accidently trigger value calculation  */
	virtual void saveCheckpoint() const;

	/** @brief Implements the RANDOMRECOMBINE recombination scheme */
	void randomRecombine(boost::shared_ptr<GIndividual>&);
	/** @brief Implements the VALUERECOMBINE recombination scheme */
	void valueRecombine(boost::shared_ptr<GIndividual>&, const std::vector<double>&);

	/** @brief Selection, MUPLUSNU_SINGLEEVAL style */
	void sortMuplusnuMode();
	/** @brief Selection, MUCOMMANU_SINGLEEVAL style */
	void sortMucommanuMode();
	/** @brief Selection, MUNU1PRETAIN style */
	void sortMunu1pretainMode();

	/** Performs a simulated annealing style sorting and selection */
	void sortSAMode();
	/** @brief Calculates the Simulated Annealing probability for a child to replace a parent */
	double saProb(const double&, const double&);
	/** @brief Updates the temperature (used for simulated annealing) */
	void updateTemperature();

	/** @brief Selection according to the pareto tag in MUPLUSNU mode (i.e. taking into account the parents) */
	void sortMuPlusNuParetoMode();
	/** @brief Selection according to the pareto tag in MUCOMMANU mode (i.e. not taking into account the parents) */
	void sortMuCommaNuParetoMode();
	/** @brief Determines whether the first individual dominates the second */
	bool aDominatesB(boost::shared_ptr<GIndividual>, boost::shared_ptr<GIndividual>) const;

	std::size_t nParents_; ///< The number of parents
	boost::uint32_t microTrainingInterval_; ///< The number of iterations without improvements after which a micro training should be started
	recoScheme recombinationMethod_; ///< The chosen recombination method
	sortingMode smode_; ///< The chosen sorting scheme
	std::size_t defaultNChildren_; ///< Expected number of children
	bool oneTimeMuCommaNu_; ///< Specifies whether a one-time selection scheme of MUCOMMANU_SINGLEEVAL should be used
	std::size_t growthRate_; ///< Specifies the amount of individuals added per iteration
	std::size_t maxPopulationSize_; ///< Specifies the maximum amount of individuals in the population if growth is enabled

	double t0_; ///< The start temperature, used in simulated annealing
	double t_; ///< The current temperature, used in simulated annealing
	double alpha_; ///< A constant used in the cooling schedule in simulated annealing

	bool logOldParents_; ///< If set, a copy of the old parent individuals will be kept and the id of the parent individual will be recorded
	std::vector<boost::shared_ptr<GIndividual> > oldParents_; ///< Holds the last generation's parents, if logOldParents_ is set

#ifdef GENEVATESTING
public:
	/**************************************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Fills the collection with individuals */
	void fillWithObjects(const std::size_t& = 10);
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */

public:
	/**************************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////////
	/**************************************************************************************/
	/**
	 * This nested class defines the interface of optimization monitors, as used
	 * by default in the Geneva library for evolutionary algorithms.
	 */
	class GEAOptimizationMonitor
		: public GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT
	{
	    ///////////////////////////////////////////////////////////////////////
	    friend class boost::serialization::access;

	    template<typename Archive>
	    void serialize(Archive & ar, const unsigned int){
	      using boost::serialization::make_nvp;

	      ar & make_nvp("GOptimizationMonitorT_GIndividual", boost::serialization::base_object<GOptimizationAlgorithmT<GIndividual>::GOptimizationMonitorT>(*this))
	         & BOOST_SERIALIZATION_NVP(xDim_)
	         & BOOST_SERIALIZATION_NVP(yDim_)
	         & BOOST_SERIALIZATION_NVP(nMonitorInds_);
	    }
	    ///////////////////////////////////////////////////////////////////////

	public:
	    /** @brief The default constructor */
	    GEAOptimizationMonitor();
	    /** @brief The copy constructor */
	    GEAOptimizationMonitor(const GEAOptimizationMonitor&);
	    /** @brief The destructor */
	    virtual ~GEAOptimizationMonitor();

	    /** @brief A standard assignment operator */
	    const GEAOptimizationMonitor& operator=(const GEAOptimizationMonitor&);
	    /** @brief Checks for equality with another GParameter Base object */
	    virtual bool operator==(const GEAOptimizationMonitor&) const;
	    /** @brief Checks for inequality with another GEAOptimizationMonitor object */
	    virtual bool operator!=(const GEAOptimizationMonitor&) const;

	    /** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
	    virtual boost::optional<std::string> checkRelationshipWith(
	    		const GObject&
	    		, const Gem::Common::expectation&
	    		, const double&
	    		, const std::string&
	    		, const std::string&
	    		, const bool&
	    ) const;

	    /** @brief Set the dimension of the output canvas */
	    void setDims(const boost::uint16_t&, const boost::uint16_t&);
	    /** @brief Retrieve the x-dimension of the output canvas */
	    boost::uint16_t getXDim() const;
	    /** @brief Retrieve the y-dimension of the output canvas */
	    boost::uint16_t getYDim() const;

	    /** @brief Sets the number of individuals in the population that should be monitored */
	    void setNMonitorIndividuals(const std::size_t&);
	    /** @brief Retrieves the number of individuals that are being monitored */
	    std::size_t getNMonitorIndividuals() const;

	protected:
	    /** @brief A function that is called once before the optimization starts */
	    virtual std::string eaFirstInformation(GSerialEA * const);
	    /** @brief A function that is called during each optimization cycle */
	    virtual std::string eaCycleInformation(GSerialEA * const);
	    /** @brief A function that is called once at the end of the optimization cycle */
	    virtual std::string eaLastInformation(GSerialEA * const);

	    /** @brief A function that is called once before the optimization starts */
	    virtual std::string firstInformation(GOptimizationAlgorithmT<GIndividual> * const);
	    /** @brief A function that is called during each optimization cycle */
	    virtual std::string cycleInformation(GOptimizationAlgorithmT<GIndividual> * const);
	    /** @brief A function that is called once at the end of the optimization cycle */
	    virtual std::string lastInformation(GOptimizationAlgorithmT<GIndividual> * const);

	    /** @brief Loads the data of another object */
	    virtual void load_(const GObject*);
	    /** @brief Creates a deep clone of this object */
		virtual GObject* clone_() const;

	private:
		boost::uint16_t xDim_; ///< The dimension of the canvas in x-direction
		boost::uint16_t yDim_; ///< The dimension of the canvas in y-direction
		std::size_t nMonitorInds_; ///< The number if individuals that should be monitored

#ifdef GENEVATESTING
	public:
		/** @brief Applies modifications to this object. This is needed for testing purposes */
		virtual bool modify_GUnitTests();
		/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
		virtual void specificTestsNoFailureExpected_GUnitTests();
		/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
		virtual void specificTestsFailuresExpected_GUnitTests();

		/**********************************************************************************/
#endif /* GENEVATESTING */
	};

	/**************************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////////
	/**************************************************************************************/
};

/*********************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSerialEA)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSerialEA::GEAOptimizationMonitor)

#endif /* GEVOLUTIONARYALGORITHM_HPP_ */
