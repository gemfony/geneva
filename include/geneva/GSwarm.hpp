/**
 * @file GSwarm.hpp
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


#ifndef GSWARM_HPP_
#define GSWARM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GIndividual.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"

#ifdef GENEVATESTING
#include "geneva-individuals/GTestIndividual1.hpp"
#endif /* GENEVATESTING */

namespace Gem {
namespace Geneva {

const std::size_t DEFAULTNNEIGHBORHOODS = 5;
const std::size_t DEFAULTNNEIGHBORHOODMEMBERS = 20;

/*********************************************************************************/
/**
 * The GSwarm class implements a swarm optimization algorithm, based on the infrastructure
 * provided by the GOptimizationAlgorithmT class. Its population is based on a constant number
 * of neighborhoods, whose amount of members is allowed to vary. This happens so that late
 * arrivals in case of networked execution can still be integrated into later iterations.
 *
 * TODO: Mark checkpoints so the serialization mode can be determined automatically (e.g. using file extension ??)
 */
class GSwarm
	:public GOptimizationAlgorithmT<GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("GOptimizationAlgorithmT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet> >(*this))
		   & BOOST_SERIALIZATION_NVP(nNeighborhoods_)
		   & BOOST_SERIALIZATION_NVP(defaultNNeighborhoodMembers_)
		   & BOOST_SERIALIZATION_NVP(nNeighborhoodMembers_)
		   & BOOST_SERIALIZATION_NVP(global_best_)
		   & BOOST_SERIALIZATION_NVP(local_bests_)
		   & BOOST_SERIALIZATION_NVP(c_personal_)
		   & BOOST_SERIALIZATION_NVP(c_local_)
		   & BOOST_SERIALIZATION_NVP(c_global_)
		   & BOOST_SERIALIZATION_NVP(c_delta_)
		   & BOOST_SERIALIZATION_NVP(ur_)
		   & BOOST_SERIALIZATION_NVP(randomFillUp_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GSwarm(const std::size_t&, const std::size_t&);
	/** @brief A standard copy constructor */
	GSwarm(const GSwarm&);
	/** @brief The destructor */
	virtual ~GSwarm();

	/** @brief A standard assignment operator */
	const GSwarm& operator=(const GSwarm&);

	/** @brief Checks for equality with another GSwarm object */
	bool operator==(const GSwarm&) const;
	/** @brief Checks for inequality with another GSwarm object */
	bool operator!=(const GSwarm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Loads a checkpoint from disk */
	virtual void loadCheckpoint(const std::string&);

	/** @brief Allows to set a static multiplier for personal distances */
	void setCPersonal(const double&);
	/** @brief Allows to retrieve the static multiplier for personal distances */
	double getCPersonal() const;

	/** @brief Allows to set a static multiplier for local distances */
	void setCLocal(const double&);
	/** @brief Allows to retrieve the static multiplier for local distances */
	double getCLocal() const;

	/** @brief Allows to set a static multiplier for global distances */
	void setCGlobal(const double&);
	/** @brief Allows to retrieve the static multiplier for local distances */
	double getCGlobal() const;

	/** @brief Allows to set a static multiplier for deltas */
	void setCDelta(const double&);
	/** @brief Allows to retrieve the static multiplier for deltas */
	double getCDelta() const;

	/** @brief Retrieves the number of neighborhoods */
	std::size_t getNNeighborhoods() const;
	/** @brief Retrieves the default number of individuals in each neighborhood */
	std::size_t getDefaultNNeighborhoodMembers() const;
	/** @brief Retrieves the current number of individuals in a given neighborhood */
	std::size_t getCurrentNNeighborhoodMembers(const std::size_t&) const;

	/** @brief Allows to specify the update rule to be used by the swarm */
	void setUpdateRule(const updateRule&);
	/** @brief Allows to retrieve the update rule currently used by the swarm */
	updateRule getUpdateRule() const;

	/** @brief All individuals automatically added to a neighborhood will have equal value */
	void setNeighborhoodsEqualFillUp();
	/** @brief All individuals automatically added to a neighborhood will have a random value */
	void setNeighborhoodsRandomFillUp();
	/** @brief Allows to check whether neighborhoods are filled up with random individuals */
	bool neighborhoodsFilledUpRandomly() const;

	/** @brief Retrieves the number of processable items for the current iteration */
	virtual std::size_t getNProcessableItems() const;

	/**************************************************************************************************/
	/**
	 * Retrieves the best individual of the population and casts it to the desired type. Note that this
	 * function will only be accessible to the compiler if parameterset_type is a derivative of GParameterSet,
	 * thanks to the magic of Boost's enable_if and Type Traits libraries. The returned individual is a clone,
	 * so you can act on it freely.
	 *
	 * @return A converted shared_ptr to a copy of the best individual of the population
	 */
	template <typename parameterset_type>
	inline boost::shared_ptr<parameterset_type> getBestIndividual(
			typename boost::enable_if<boost::is_base_of<GParameterSet, parameterset_type> >::type* dummy = 0
	){
#ifdef DEBUG
		// Check that global_best_ actually points somewhere
		if(!global_best_) {
			raiseException(
					"In GSwarm::getBestIndividual<>() : Error" << std::endl
					<< "Tried to access uninitialized globally best individual."
			);
		}
#endif /* DEBUG */

		return global_best_->clone<parameterset_type>();
	}

	/**************************************************************************************************/
	/**
	 * Retrieves the best individual of a neighborhood and casts it to the desired type. Note that this
	 * function will only be accessible to the compiler if parameterset_type is a derivative of GParameterSet,
	 * thanks to the magic of Boost's enable_if and Type Traits libraries.
	 *
	 * @param neighborhood The neighborhood, whose best individual should be returned
	 * @return A converted shared_ptr to the best individual of a given neighborhood
	 */
	template <typename parameterset_type>
	inline boost::shared_ptr<parameterset_type> getBestNeighborhoodIndividual(
			std::size_t neighborhood
		  , typename boost::enable_if<boost::is_base_of<GParameterSet, parameterset_type> >::type* dummy = 0
	){
#ifdef DEBUG
		// Check that the neighborhood is in a valid range
		if(neighborhood >= nNeighborhoods_) {
			raiseException(
					"In GSwarm::getBestNeighborhoodIndividual<>() : Error" << std::endl
					<< "Requested neighborhood which does not exist: " << neighborhood << " / " << nNeighborhoods_
			);
		}

		// Check that pointer actually points somewhere
		if(!local_bests_[neighborhood]) {
			raiseException(
					"In GSwarm::getBestNeighborhoodIndividual<>() : Error" << std::endl
					<< "Tried to access uninitialized locally best individual."
			);
		}

		boost::shared_ptr<parameterset_type> p = boost::dynamic_pointer_cast<parameterset_type>(local_bests_[neighborhood]);

		if(p) return p;
		else {
			raiseException(
					"In GSwarm::getBestNeighborhoodIndividual<>() : Conversion error"
			);
		}
#else
		return boost::static_pointer_cast<parameterset_type>(local_bests_[neighborhood]);
#endif /* DEBUG */
	}

protected:
	/** @brief Loads the data of another population */
	virtual void load_(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone_() const;
	/** @brief Allows to set the personality type of the individuals */
	virtual void setIndividualPersonalities();
	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual double cycleLogic();
	/** @brief Does some preparatory work before the optimization starts */
	virtual void init();
	/** @brief Does any necessary finalization work */
	virtual void finalize();

	/** @brief Resizes the population to the desired level and does some error checks */
	virtual void adjustPopulation();

	/** @brief Saves the state of the class to disc. */
	virtual void saveCheckpoint() const;

	/** @brief Updates the positions and/or fitness of all individuals */
	virtual void swarmLogic();
	/** @brief Updates the best individuals found */
	virtual double findBests();
	/** @brief Adjusts each neighborhood so it has the correct size */
	virtual void adjustNeighborhoods();

	/** @brief Helper function that returns the id of the first individual of a neighborhood */
	std::size_t getFirstNIPos(const std::size_t&) const;
	/** @brief Helper function that returns the id of the first individual of a neighborhood, using a vector of neighborhood sizes */
	std::size_t getFirstNIPosVec(const std::size_t&, const std::vector<std::size_t>&) const;
	/** @brief Helper function that returns the id of the last individual of a neighborhood */
	std::size_t getLastNIPos(const std::size_t&) const;

	/** @brief Triggers an update of the individuals' positions */
	void updatePositions(
		  const std::size_t&
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::tuple<double, double, double, double>
	);

	/** @brief Triggers the fitness calculation */
	virtual void updateFitness(
			const boost::uint32_t&
			, const std::size_t&
			, boost::shared_ptr<GParameterSet>
	);

	/** @brief Updates the individual's position and performs the fitness calculation */
	void updateSwarm (
		  const boost::uint32_t&
		  , const std::size_t&
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
	      , boost::tuple<double, double, double, double>
	);

    /** @brief Checks whether each neighborhood has at least the default size */
    bool neighborhoodsHaveNominalValues() const;

	std::size_t nNeighborhoods_; ///< The number of neighborhoods in the population
	std::size_t defaultNNeighborhoodMembers_; ///< The desired number of individuals belonging to each neighborhood
	std::vector<std::size_t> nNeighborhoodMembers_; ///< The current number of individuals belonging to each neighborhood

	boost::shared_ptr<GParameterSet> global_best_; ///< The globally best individual
	std::vector<boost::shared_ptr<GParameterSet> > local_bests_; ///< The collection of best individuals from each neighborhood
	std::vector<boost::shared_ptr<GParameterSet> > velocities_; ///< Holds velocities, as calculated in the previous iteration

	double c_personal_; ///< A factor for multiplication of personal bests
	double c_local_; ///< A factor for multiplication of local bests
	double c_global_; ///< A factor for multiplication of global bests
	double c_delta_; ///< A factor for multiplication of deltas

	updateRule ur_; ///< Specifies how the parameters are updated
	bool randomFillUp_; ///< Specifies whether neighborhoods are filled up with random values

	/** @brief The default constructor. Intentionally protected, as it is only needed for de-serialization purposes. */
	GSwarm();

	/** Updates the personal best of an individual */
	void updatePersonalBest(boost::shared_ptr<GParameterSet>, boost::shared_ptr<GParameterSet>);
	/** Updates the personal best of an individual, if a better solution was found */
	void updatePersonalBestIfBetter(boost::shared_ptr<GParameterSet>, boost::shared_ptr<GParameterSet>);

private:
	/**************************************************************************************************/
	/** @brief Helper function that checks the content of two nNeighborhoodMembers_ arrays */
	bool nNeighborhoodMembersEqual(const std::vector<std::size_t>&, const std::vector<std::size_t>&) const;

	/** @brief Small helper function that helps to fill up a neighborhood, if there is just one entry in it */
	void fillUpNeighborhood1();

#ifdef GENEVATESTING
public:
	/**************************************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
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
	 * by default in the Geneva library for swarm algorithms.
	 */
	class GSwarmOptimizationMonitor
		: public GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT
	{
	    ///////////////////////////////////////////////////////////////////////
	    friend class boost::serialization::access;

	    template<typename Archive>
	    void serialize(Archive & ar, const unsigned int){
	      using boost::serialization::make_nvp;

	      ar & make_nvp("GOptimizationMonitorT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(*this))
	         & BOOST_SERIALIZATION_NVP(xDim_)
	         & BOOST_SERIALIZATION_NVP(yDim_);
	    }
	    ///////////////////////////////////////////////////////////////////////

	public:
	    /** @brief The default constructor */
	    GSwarmOptimizationMonitor();
	    /** @brief The copy constructor */
	    GSwarmOptimizationMonitor(const GSwarmOptimizationMonitor&);
	    /** @brief The destructor */
	    virtual ~GSwarmOptimizationMonitor();

	    /** @brief A standard assignment operator */
	    const GSwarmOptimizationMonitor& operator=(const GSwarmOptimizationMonitor&);
	    /** @brief Checks for equality with another GParameter Base object */
	    virtual bool operator==(const GSwarmOptimizationMonitor&) const;
	    /** @brief Checks for inequality with another GSwarmOptimizationMonitor object */
	    virtual bool operator!=(const GSwarmOptimizationMonitor&) const;

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

	protected:
	    /** @brief A function that is called once before the optimization starts */
	    virtual std::string firstInformation(GOptimizationAlgorithmT<GParameterSet> * const);
	    /** @brief A function that is called during each optimization cycle */
	    virtual std::string cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const);
	    /** @brief A function that is called once at the end of the optimization cycle */
	    virtual std::string lastInformation(GOptimizationAlgorithmT<GParameterSet> * const);

	    /** @brief A function that is called once before the optimization starts */
	    virtual std::string swarmFirstInformation(GSwarm * const);
	    /** @brief A function that is called during each optimization cycle */
	    virtual std::string swarmCycleInformation(GSwarm * const);
	    /** @brief A function that is called once at the end of the optimization cycle */
	    virtual std::string swarmLastInformation(GSwarm * const);

	    /** @brief Loads the data of another object */
	    virtual void load_(const GObject*);
	    /** @brief Creates a deep clone of this object */
		virtual GObject* clone_() const;

	private:
		boost::uint16_t xDim_; ///< The dimension of the canvas in x-direction
		boost::uint16_t yDim_; ///< The dimension of the canvas in y-direction

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

/******************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/** @brief We need to provide a specialization of the factory function that creates objects of this type. */
template <> boost::shared_ptr<Gem::Geneva::GSwarm> TFactory_GUnitTests<Gem::Geneva::GSwarm>();

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSwarm)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSwarm::GSwarmOptimizationMonitor)

#endif /* GSWARM_HPP_ */
