/**
 * @file GSwarm.hpp
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


// Standard headers go here
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cfloat>
#include <vector>
#include <algorithm>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

/**
 * Check that we have support for threads
 */
#ifndef BOOST_HAS_THREADS
#error "Error: Support for multi-threading does not seem to be available."
#endif

#ifndef GSWARM_HPP_
#define GSWARM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GIndividual.hpp"
#include "GParameterSet.hpp"
#include "GOptimizationAlgorithmT.hpp"
#include "GOptimizationEnums.hpp"

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
	void load(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		std::size_t currentNNeighborhoods = nNeighborhoods_;

		ar & make_nvp("GOptimizationAlgorithmT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet> >(*this))
		   & BOOST_SERIALIZATION_NVP(nNeighborhoods_);

		if(currentNNeighborhoods != nNeighborhoods_) {
			delete [] nNeighborhoodMembers_;
			nNeighborhoodMembers_ = new std::size_t[nNeighborhoods_];
		}

		std::vector<std::size_t> nNeighborhoodMembersVec;
		ar & BOOST_SERIALIZATION_NVP(nNeighborhoodMembersVec);
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			nNeighborhoodMembers_[i] = nNeighborhoodMembersVec[i];
		}

		ar & BOOST_SERIALIZATION_NVP(global_best_);

		std::vector<boost::shared_ptr<GParameterSet> > local_bests_vec;
		ar & BOOST_SERIALIZATION_NVP(local_bests_vec);

		if(currentNNeighborhoods != nNeighborhoods_) {
			delete [] local_bests_;
			local_bests_ = new boost::shared_ptr<GParameterSet>[nNeighborhoods_];
		}

		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			local_bests_[i] = local_bests_vec[i];
		}

		ar & BOOST_SERIALIZATION_NVP(c_local_)
		   & BOOST_SERIALIZATION_NVP(c_global_)
		   & BOOST_SERIALIZATION_NVP(c_delta_)
		   & BOOST_SERIALIZATION_NVP(ur_);
	}

	template<typename Archive>
	void save(Archive & ar, const unsigned int) const {
		using boost::serialization::make_nvp;

		ar & make_nvp("GOptimizationAlgorithmT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet> >(*this))
		   & BOOST_SERIALIZATION_NVP(nNeighborhoods_);

		std::vector<std::size_t> nNeighborhoodMembersVec;
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			nNeighborhoodMembersVec.push_back(nNeighborhoodMembers_[i]);
		}

		ar & BOOST_SERIALIZATION_NVP(nNeighborhoodMembersVec)
		   & BOOST_SERIALIZATION_NVP(global_best_);

		std::vector<boost::shared_ptr<GParameterSet> > local_bests_vec;
		for(std::size_t i=0; i<nNeighborhoods_; i++) {
			local_bests_vec.push_back(local_bests_[i]);
		}

		ar & BOOST_SERIALIZATION_NVP(local_bests_vec)
		   & BOOST_SERIALIZATION_NVP(c_local_)
		   & BOOST_SERIALIZATION_NVP(c_global_)
		   & BOOST_SERIALIZATION_NVP(c_delta_)
		   & BOOST_SERIALIZATION_NVP(ur_);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

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
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Emits information specific to this population */
	virtual void doInfo(const infoMode&);

	/** @brief Registers a function to be called when emitting information from doInfo */
	void registerInfoFunction(boost::function<void (const infoMode&, GSwarm * const)>);

	/** @brief Loads a checkpoint from disk */
	virtual void loadCheckpoint(const std::string&);

	/** @brief Allows to set a static multiplier for local distances */
	void setCLocal(const float&);
	/** @brief Allows to retrieve the static multiplier for local distances or the lower boundary of a random range */
	float getCLocal() const;

	/** @brief Allows to set a static multiplier for global distances */
	void setCGlobal(const float&);
	/** @brief Allows to retrieve the static multiplier for local distances or the lower boundary of a random range */
	float getCGlobal() const;

	/** @brief Allows to set a static multiplier for deltas */
	void setCDelta(const float&);
	/** @brief Allows to retrieve the static multiplier for deltas or the lower boundary of a random range */
	float getCDelta() const;

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

	/**************************************************************************************************/
	/**
	 * Retrieves the best individual of the population and casts it to the desired type. Note that this
	 * function will only be accessible to the compiler if parameterset_type is a derivative of GParameterSet,
	 * thanks to the magic of Boost's enable_if and Type Traits libraries.
	 *
	 * @return A converted shared_ptr to the best (i.e. first) individual of the population
	 */
	template <typename parameterset_type>
	inline boost::shared_ptr<parameterset_type> getBestIndividual(
			typename boost::enable_if<boost::is_base_of<GParameterSet, parameterset_type> >::type* dummy = 0
	){
#ifdef DEBUG
		// Check that global_best_ actually points somewhere
		if(!global_best_) {
			std::ostringstream error;
			error << "In GSwarm::getBestIndividual<>() : Error" << std::endl
				  << "Tried to access uninitialized globally best individual." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		boost::shared_ptr<parameterset_type> p = boost::dynamic_pointer_cast<parameterset_type>(global_best_);

		if(p) return p;
		else {
			std::ostringstream error;
			error << "In GSwarm::getBestIndividual<>() : Conversion error" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#else
		return boost::static_pointer_cast<parameterset_type>(global_best_);
#endif /* DEBUG */
	}

	/**************************************************************************************************/
	/**
	 * Retrieves the best individual of a neighborhood and casts it to the desired type. Note that this
	 * function will only be accessible to the compiler if parameterset_type is a derivative of GParameterSet,
	 * thanks to the magic of Boost's enable_if and Type Traits libraries.
	 *
	 * @param neighborhood The neighborhood, whose best individual should be returned
	 * @return A converted shared_ptr to the best (i.e. first) individual of the population
	 */
	template <typename parameterset_type>
	inline boost::shared_ptr<parameterset_type> getBestNeighborhoodIndividual(
			std::size_t neighborhood
		  , typename boost::enable_if<boost::is_base_of<GIndividual, parameterset_type> >::type* dummy = 0
	){
#ifdef DEBUG
		// Check that the neighborhood is in a valid range
		if(neighborhood >= nNeighborhoods_) {
			std::ostringstream error;
			error << "In GSwarm::getBestNeighborhoodIndividual<>() : Error" << std::endl
				  << "Requested neighborhood which does not exist: " << neighborhood << " / " << nNeighborhoods_ << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Check that global_best_ actually points somewhere
		if(!local_bests_[neighborhood]) {
			std::ostringstream error;
			error << "In GSwarm::getBestNeighborhoodIndividual<>() : Error" << std::endl
				  << "Tried to access uninitialized locally best individual." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		boost::shared_ptr<parameterset_type> p = boost::dynamic_pointer_cast<parameterset_type>(local_bests_[neighborhood]);

		if(p) return p;
		else {
			std::ostringstream error;
			error << "In GSwarm::getBestNeighborhoodIndividual<>() : Conversion error" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#else
		return boost::static_pointer_cast<parameterset_type>(local_bests_[neighborhood]);
#endif /* DEBUG */
	}

	/**************************************************************************************************/
	/**
	 * Emits information about the population it has been given, using a simple format. Note that we are
	 * using a static member function in order to avoid storing a local "this" pointer in this function
	 * when registering it in boost::function.
	 *
	 * Far more sophisticated setups than this information function are possible, and in general
	 * it is recommended to register function objects instead of this function.
	 *
	 * @param im Indicates the information mode
	 * @param gs A pointer to the population information should be emitted about
	 */
	static void simpleInfoFunction(const infoMode& im, GSwarm * const gs) {
		std::ostringstream information;

		switch(im){
		case INFOINIT: // nothing
			break;

		case INFOPROCESSING:
			{
				bool isDirty = false;

				information << std::setprecision(10) << "In iteration "<< gs->getIteration() << ": " << gs->getBestFitness() << std::endl;
			}
			break;

		case INFOEND: // nothing
			break;
		}

		// Let the audience know
		std::cout << information.str();
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
	std::size_t getFirstNIPosVec(const std::size_t&, std::size_t*) const;
	/** @brief Helper function that returns the id of the last individual of a neighborhood */
	std::size_t getLastNIPos(const std::size_t&) const;

	/** @brief Triggers an update of the individuals' positions */
	void updatePositions(
		    std::size_t
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , double
		  , double
		  , double
	);
	/** @brief Triggers the fitness calculation */
	virtual void updateFitness(std::size_t, boost::shared_ptr<GParameterSet>);
	/** @brief Updates the individual's position and performs the fitness calculation */
	void updatePositionsAndFitness (
		    std::size_t
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , boost::shared_ptr<GParameterSet>
		  , double
		  , double
		  , double
	);

    /** @brief Checks whether each neighborhood has at least the default size */
    bool neighborhoodsHaveNominalValues() const;

	boost::function<void (const infoMode&, GSwarm * const)> infoFunction_; ///< Used to emit information with doInfo()

	std::size_t nNeighborhoods_; ///< The number of neighborhoods in the population
	std::size_t defaultNNeighborhoodMembers_; ///< The desired number of individuals belonging to each neighborhood
	std::size_t *nNeighborhoodMembers_; ///< The current number of individuals belonging to each neighborhood

	boost::shared_ptr<GParameterSet> global_best_; ///< The globally best individual
	boost::shared_ptr<GParameterSet> *local_bests_; ///< The collection of best individuals from each neighborhood

	std::vector<boost::shared_ptr<GParameterSet> > velocities_; ///< Holds velocities, as calculated in the previous iteration

	/** @brief A factor for multiplication of local bests */
	float c_local_;
	/** @brief A factor for multiplication of global bests */
	float c_global_;
	/** @brief A factor for multiplication of deltas */
	float c_delta_;

	/** @brief Specifies how the parameters are updated */
	updateRule ur_;

	/** @brief The default constructor. Intentionally empty, as it is only needed for de-serialization purposes. */
	GSwarm(){}

private:
	/**************************************************************************************************/
	/** @brief Helper function that checks the content of two nNeighborhoodMembers_ arrays */
	bool nNeighborhoodMembersEqual(const std::size_t *, const std::size_t *) const;

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
};

/******************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GSWARM_HPP_ */
