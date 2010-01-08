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

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/cast.hpp>
#include <boost/filesystem.hpp>

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


// GenEvA headers go here
#include "GIndividual.hpp"
#include "GOptimizationAlgorithm.hpp"
#include "GenevaExceptions.hpp"
#include "GEnums.hpp"

namespace Gem {
namespace GenEvA {

const std::size_t DEFAULTNNEIGHBORHOODS = 5;
const std::size_t DEFAULTNNEIGHBORHOODMEMBERS = 20;

/*********************************************************************************/
/**
 * The GSwarm class implements a swarm optimization algorithm, based on the infrastructure
 * provided by the GOptimizationAlgorithm class.
 */
class GSwarm
	:public GOptimizationAlgorithm
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("GOptimizationAlgorithm",	boost::serialization::base_object<GOptimizationAlgorithm>(*this));
		ar & make_nvp("nNeighborhoods_", nNeighborhoods_);
		ar & make_nvp("nNeighborhoodMembers_", nNeighborhoodMembers_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GSwarm();
	/** @brief A standard copy constructor */
	GSwarm(const GSwarm&);
	/** @brief The destructor */
	virtual ~GSwarm();

	/** @brief A standard assignment operator */
	const GSwarm& operator=(const GSwarm&);

	/** @brief Loads the data of another population */
	virtual void load(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone() const;

	/** @brief Checks for equality with another GSwarm object */
	bool operator==(const GSwarm&) const;
	/** @brief Checks for inequality with another GSwarm object */
	bool operator!=(const GSwarm&) const;
	/** @brief Checks for equality with another GSwarm object */
	virtual bool isEqualTo(const GObject&,  const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GSwarm object */
	virtual bool isSimilarTo(const GObject&, const double&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Emits information specific to this population */
	virtual void doInfo(const infoMode&);

	/** @brief Registers a function to be called when emitting information from doInfo */
	void registerInfoFunction(boost::function<void (const infoMode&, GSwarm * const)>);

	/** @brief Loads a checkpoint from disk */
	virtual void loadCheckpoint(const std::string&);

	/** @brief Sets the local multiplier used when calculating velocities to a fixed value in all individuals */
	void setCLocal(const double&);
	/** @brief Sets the local multiplier of each individual randomly within a given range */
	void setCLocal(const double&, const double&);
	/** @brief Sets the global multiplier used when calculating velocities to a fixed value in all individuals */
	void setCGlobal(const double&);
	/** @brief Sets the global multiplier of each individual randomly within a given range */
	void setCGlobal(const double&, const double&);
	/** @brief Sets the velocity multiplier to a fixed value for each individual */
	void setCVelocity(const double&);
	/** @brief Sets the velocity multiplier to a random value separately for each individual */
	void setCVelocity(const double&, const double&);

	/** @brief Retrieves the local multiplier */
	double getCLocal() const;
	/** @brief Retrieves the global multiplier */
	double getCGlobal() const;
	/** @brief Retrieves the velocity multiplier */
	double getCVelocity() const;

	/** @brief Sets the population size based on the number of neighborhoods and the number of individuals in them */
	void setPopulationSize(const std::size_t&, const std::size_t&);
	/** @brief Retrieves the number of neighborhoods */
	std::size_t getNNeighborhoods() const;
	/** @brief Retrieves the number of individuals in each neighborhood */
	std::size_t getNNeighborhoodMembers() const;

	/**************************************************************************************************/
	/**
	 * Retrieves the best individual of the population and casts it to the desired type.
	 *
	 * @return A converted shared_ptr to the best (i.e. first) individual of the population
	 */
	template <typename individual_type>
	inline boost::shared_ptr<individual_type> getBestIndividual(){
		// ...
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
	 * @param gbp A pointer to the population information should be emitted about
	 */
	static void simpleInfoFunction(const infoMode& im, GSwarm * const gbp) {
		std::ostringstream information;

		switch(im){
		case INFOINIT: // nothing
			break;

		case INFOPROCESSING:
			{
				bool isDirty = false;

				information << std::setprecision(10) << "In iteration "<< gbp->getIteration() << ": " << gbp->data.at(0)->getCurrentFitness(isDirty);

				if(isDirty) {
					information << " (dirty flag is set)";
				}
				information << std::endl;
			}
			break;

		case INFOEND: // nothing
			break;
		}

		// Let the audience know
		std::cout << information.str();
	}

	/**************************************************************************************************/

protected:
	/** @brief Allows to set the personality type of the individuals */
	virtual void setIndividualPersonalities();
	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual double cycleLogic();
	/** @brief Does some preparatory work before the optimization starts */
	virtual void init();

private:
	/** @brief Saves the state of the class to disc. Private, as we do not want to accidently trigger value calculation  */
	virtual void saveCheckpoint() const;

	boost::function<void (const infoMode&, GSwarm * const)> infoFunction_; ///< Used to emit information with doInfo()

	std::size_t nNeighborhoods_; ///< The number of neighborhoods in the population
	std::size_t nNeighborhoodMembers_; ///< The number of individuals belonging to each neighborhood

	boost::shared_ptr<GIndividual> global_best_; ///< The globally best individual
	std::vector<boost::shared_ptr<GIndividual> > local_bests_; ///< The collection of best individuals from each neighborhood
};

/*********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GSWARM_HPP_ */
