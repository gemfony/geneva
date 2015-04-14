/**
 * @file GBaseSwarm.hpp
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

// Boost headers go here


#ifndef GBASESWARM_HPP_
#define GBASESWARM_HPP_


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GSwarmPersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {


/******************************************************************************/
/**
 * The GBaseSwarm class implements a swarm optimization algorithm, based on the infrastructure
 * provided by the GOptimizationAlgorithmT class. Its population is based on a constant number
 * of neighborhoods, whose amount of members is allowed to vary. This happens so that late
 * arrivals in case of networked execution can still be integrated into later iterations.
 *
 * TODO: Mark checkpoints so the serialization mode can be determined automatically (e.g. using file extension ??)
 */
class GBaseSwarm
	:public GOptimizationAlgorithmT<GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GOptimizationAlgorithmT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet> >(*this))
		& BOOST_SERIALIZATION_NVP(nNeighborhoods_)
		& BOOST_SERIALIZATION_NVP(defaultNNeighborhoodMembers_)
		& BOOST_SERIALIZATION_NVP(nNeighborhoodMembers_)
		& BOOST_SERIALIZATION_NVP(global_best_)
		& BOOST_SERIALIZATION_NVP(neighborhood_bests_)
		& BOOST_SERIALIZATION_NVP(c_personal_)
		& BOOST_SERIALIZATION_NVP(c_neighborhood_)
		& BOOST_SERIALIZATION_NVP(c_global_)
		& BOOST_SERIALIZATION_NVP(c_velocity_)
		& BOOST_SERIALIZATION_NVP(updateRule_)
		& BOOST_SERIALIZATION_NVP(randomFillUp_)
		& BOOST_SERIALIZATION_NVP(repulsionThreshold_)
		& BOOST_SERIALIZATION_NVP(dblLowerParameterBoundaries_)
		& BOOST_SERIALIZATION_NVP(dblUpperParameterBoundaries_)
		& BOOST_SERIALIZATION_NVP(dblVelVecMax_)
		& BOOST_SERIALIZATION_NVP(velocityRangePercentage_);
	}
	///////////////////////////////////////////////////////////////////////

public:
   /** @brief An easy identifier for the class */
   static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

	/** @brief The default constructor */
   G_API_GENEVA GBaseSwarm();
	/** @brief Initialization with neighborhood sizes and amount of individuals in each neighborhood */
   G_API_GENEVA GBaseSwarm(const std::size_t&, const std::size_t&);
	/** @brief A standard copy constructor */
   G_API_GENEVA GBaseSwarm(const GBaseSwarm&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GBaseSwarm();

   /** @brief The standard assignment operator */
   G_API_GENEVA const GBaseSwarm& operator=(const GBaseSwarm&);

   /** @brief Checks for equality with another GBaseSwarm object */
   virtual G_API_GENEVA bool operator==(const GBaseSwarm&) const;
   /** @brief Checks for inequality with another GBaseSwarm object */
   virtual G_API_GENEVA bool operator!=(const GBaseSwarm&) const;

   /** @brief Searches for compliance with expectations with respect to another object of the same type */
   virtual G_API_GENEVA void compare(
      const GObject& // the other object
      , const Gem::Common::expectation& // the expectation for this object, e.g. equality
      , const double& // the limit for allowed deviations of floating point types
   ) const OVERRIDE;

	/** @brief Sets the number of neighborhoods and the number of members in them */
	G_API_GENEVA void setSwarmSizes(std::size_t, std::size_t);

	/** @brief Returns information about the type of optimization algorithm */
	virtual G_API_GENEVA std::string getOptimizationAlgorithm() const OVERRIDE;

	/** @brief Loads a checkpoint from disk */
	virtual G_API_GENEVA void loadCheckpoint(const boost::filesystem::path&) OVERRIDE;

	/** @brief Allows to set a static multiplier for personal distances */
	G_API_GENEVA void setCPersonal(double);
	/** @brief Allows to retrieve the static multiplier for personal distances */
	G_API_GENEVA double getCPersonal() const;

	/** @brief Allows to set a static multiplier for neighborhood distances */
	G_API_GENEVA void setCNeighborhood(double);
	/** @brief Allows to retrieve the static multiplier for neighborhood distances */
	G_API_GENEVA double getCNeighborhood() const;

	/** @brief Allows to set a static multiplier for global distances */
	G_API_GENEVA void setCGlobal(double);
	/** @brief Allows to retrieve the static multiplier for global distances */
	G_API_GENEVA double getCGlobal() const;

	/** @brief Allows to set a static multiplier for velocities */
	G_API_GENEVA void setCVelocity(double);
	/** @brief Allows to retrieve the static multiplier for velocities */
	G_API_GENEVA double getCVelocity() const;

	/** @brief Allows to set the velocity range percentage */
	G_API_GENEVA void setVelocityRangePercentage(double);
	/** @brief Allows to retrieve the velocity range percentage */
	G_API_GENEVA double getVelocityRangePercentage() const;

	/** @brief Retrieves the number of neighborhoods */
	G_API_GENEVA std::size_t getNNeighborhoods() const;
	/** @brief Retrieves the default number of individuals in each neighborhood */
	G_API_GENEVA std::size_t getDefaultNNeighborhoodMembers() const;
	/** @brief Retrieves the current number of individuals in a given neighborhood */
	G_API_GENEVA std::size_t getCurrentNNeighborhoodMembers(const std::size_t&) const;

	/** @brief Allows to specify the update rule to be used by the swarm */
	G_API_GENEVA void setUpdateRule(updateRule);
	/** @brief Allows to retrieve the update rule currently used by the swarm */
	G_API_GENEVA updateRule getUpdateRule() const;

	/** @brief Allows to specify the number of stalls as of which the algorithm switches to repulsive mode */
	G_API_GENEVA void setRepulsionThreshold(boost::uint32_t);
	/** @brief Allows to retrieve the number of stalls as of which the algorithm switches to repulsive mode */
	G_API_GENEVA boost::uint32_t getRepulsionThreshold() const;

	/** @brief All individuals automatically added to a neighborhood will have equal value */
	G_API_GENEVA void setNeighborhoodsEqualFillUp();
	/** @brief All individuals automatically added to a neighborhood will have a random value */
	G_API_GENEVA void setNeighborhoodsRandomFillUp(bool = true);
	/** @brief Allows to check whether neighborhoods are filled up with random individuals */
	G_API_GENEVA bool neighborhoodsFilledUpRandomly() const;

	/** @brief Retrieves the number of processable items for the current iteration */
	virtual G_API_GENEVA std::size_t getNProcessableItems() const OVERRIDE;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) OVERRIDE;

	/***************************************************************************/
	/**
	 * Retrieves the best individual of a neighborhood and casts it to the desired type. Note that this
	 * function will only be accessible to the compiler if parameterset_type is a derivative of GParameterSet,
	 * thanks to the magic of Boost's enable_if and Type Traits libraries.
	 *
	 * @param neighborhood The neighborhood, whose best individual should be returned
	 * @return A converted shared_ptr to the best individual of a given neighborhood
	 */
	template <typename parameterset_type>
	boost::shared_ptr<parameterset_type> getBestNeighborhoodIndividual(
      std::size_t neighborhood
     , typename boost::enable_if<boost::is_base_of<GParameterSet, parameterset_type> >::type* dummy = 0
	){
#ifdef DEBUG
		// Check that the neighborhood is in a valid range
		if(neighborhood >= nNeighborhoods_) {
		   glogger
		   << "In GBaseSwarm::getBestNeighborhoodIndividual<>() : Error" << std::endl
         << "Requested neighborhood which does not exist: " << neighborhood << " / " << nNeighborhoods_ << std::endl
         << GEXCEPTION;

		   // Make the compiler happy
		   return boost::shared_ptr<parameterset_type>();
		}
#endif /* DEBUG */

      // Does error checks on the conversion internally
      return Gem::Common::convertSmartPointer<GParameterSet, parameterset_type>(neighborhood_bests_[neighborhood]);
	}

   /** @brief Emits a name for this class / object */
   virtual G_API_GENEVA std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another population */
	virtual G_API_GENEVA void load_(const GObject *) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject *clone_() const = 0;

	/** @brief Does some preparatory work before the optimization starts */
	virtual G_API_GENEVA void init() OVERRIDE;
	/** @brief Does any necessary finalization work */
	virtual G_API_GENEVA void finalize() OVERRIDE;

   /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
   virtual G_API_GENEVA boost::shared_ptr<GPersonalityTraits> getPersonalityTraits() const OVERRIDE;

	/** @brief The actual business logic to be performed during each iteration; Returns the best achieved fitness */
	virtual G_API_GENEVA boost::tuple<double, double> cycleLogic() OVERRIDE;
   /** @brief Fixes an incomplete population */
   virtual G_API_GENEVA void adjustNeighborhoods() BASE;

	/** @brief Saves the state of the class to disc. */
	virtual G_API_GENEVA void saveCheckpoint() const OVERRIDE;

	/** @brief Updates the best individuals found */
	virtual G_API_GENEVA boost::tuple<double, double> findBests();
	/** @brief Resizes the population to the desired level and does some error checks */
	virtual G_API_GENEVA void adjustPopulation() OVERRIDE;

	/** @brief Helper function that returns the id of the first individual of a neighborhood */
	G_API_GENEVA std::size_t getFirstNIPos(const std::size_t&) const;
	/** @brief Helper function that returns the id of the first individual of a neighborhood, using a vector of neighborhood sizes */
	G_API_GENEVA std::size_t getFirstNIPosVec(const std::size_t&, const std::vector<std::size_t>&) const;
	/** @brief Helper function that returns the id of the last individual of a neighborhood */
	G_API_GENEVA std::size_t getLastNIPos(const std::size_t&) const;

	/** @brief Triggers an update of an individual's positions */
	G_API_GENEVA void updateIndividualPositions(
     const std::size_t&
     , boost::shared_ptr<GParameterSet>
     , boost::shared_ptr<GParameterSet>
     , boost::shared_ptr<GParameterSet>
     , boost::shared_ptr<GParameterSet>
     , boost::tuple<double, double, double, double>
	);

	/** @brief Triggers an update of all individual's positions */
	virtual G_API_GENEVA void updatePositions();

	/** @brief Updates the fitness of all individuals */
	virtual G_API_GENEVA void runFitnessCalculation() = 0;
   /** @brief Adjusts the velocity vector so that its values don't exceed the allowed value range */
	G_API_GENEVA void pruneVelocity(std::vector<double>&);

	std::size_t nNeighborhoods_; ///< The number of neighborhoods in the population
	std::size_t defaultNNeighborhoodMembers_; ///< The desired number of individuals belonging to each neighborhood
	std::vector<std::size_t> nNeighborhoodMembers_; ///< The current number of individuals belonging to each neighborhood

	boost::shared_ptr<GParameterSet> global_best_; ///< The globally best individual
	std::vector<boost::shared_ptr<GParameterSet> > neighborhood_bests_; ///< The collection of best individuals from each neighborhood
	std::vector<boost::shared_ptr<GParameterSet> > velocities_; ///< Holds velocities, as calculated in the previous iteration

	double c_personal_; ///< A factor for multiplication of personal best distances
	double c_neighborhood_; ///< A factor for multiplication of neighborhood best distances
	double c_global_; ///< A factor for multiplication of global best distances
	double c_velocity_; ///< A factor for multiplication of velocities

	updateRule updateRule_; ///< Specifies how the parameters are updated
	bool randomFillUp_; ///< Specifies whether neighborhoods are filled up with random values

	boost::uint32_t repulsionThreshold_; ///< The number of stalls until the swarm algorithm switches to repulsion instead of attraction

	std::vector<double> dblLowerParameterBoundaries_; ///< Holds lower boundaries of double parameters
	std::vector<double> dblUpperParameterBoundaries_; ///< Holds upper boundaries of double parameters
	std::vector<double> dblVelVecMax_; ///< Holds the maximum allowed values of double-type velocities

	double velocityRangePercentage_; ///< Indicates the percentage of a value range used for the initialization of the velocity

	/** Updates the personal best of an individual */
	G_API_GENEVA void updatePersonalBest(boost::shared_ptr<GParameterSet>);
	/** Updates the personal best of an individual, if a better solution was found */
	G_API_GENEVA void updatePersonalBestIfBetter(boost::shared_ptr<GParameterSet>);

	/** @brief Returns the name of this optimization algorithm */
	virtual G_API_GENEVA std::string getAlgorithmName() const OVERRIDE;

private:
	/***************************************************************************/
	/** @brief Helper function that checks the content of two nNeighborhoodMembers_ arrays */
	bool nNeighborhoodMembersEqual(const std::vector<std::size_t>&, const std::vector<std::size_t>&) const;

	/** @brief Small helper function that helps to fill up a neighborhood, if there is just one entry in it */
	void fillUpNeighborhood1();

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;

public:
	/***************************************************************************/
	/////////////////////////////////////////////////////////////////////////////
	/***************************************************************************/
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

	      ar
	      & make_nvp("GOptimizationMonitorT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(*this))
	      & BOOST_SERIALIZATION_NVP(xDim_)
	      & BOOST_SERIALIZATION_NVP(yDim_)
	      & BOOST_SERIALIZATION_NVP(resultFile_);
	    }
	    ///////////////////////////////////////////////////////////////////////

	public:
	    /** @brief The default constructor */
	    G_API_GENEVA GSwarmOptimizationMonitor();
	    /** @brief The copy constructor */
	    G_API_GENEVA GSwarmOptimizationMonitor(const GSwarmOptimizationMonitor&);
	    /** @brief The destructor */
	    virtual G_API_GENEVA ~GSwarmOptimizationMonitor();

	    G_API_GENEVA const GBaseSwarm::GSwarmOptimizationMonitor& operator=(
	          const GBaseSwarm::GSwarmOptimizationMonitor&
	    );

	    /** @brief Checks for equality with another GParameter Base object */
	    virtual G_API_GENEVA bool operator==(const GSwarmOptimizationMonitor&) const;
	    /** @brief Checks for inequality with another GSwarmOptimizationMonitor object */
	    virtual G_API_GENEVA bool operator!=(const GSwarmOptimizationMonitor&) const;

	    /** @brief Searches for compliance with expectations with respect to another object of the same type */
	    virtual G_API_GENEVA void compare(
	       const GObject& // the other object
	       , const Gem::Common::expectation& // the expectation for this object, e.g. equality
	       , const double& // the limit for allowed deviations of floating point types
	    ) const OVERRIDE;

	    /** @brief Set the dimension of the output canvas */
	    G_API_GENEVA void setDims(const boost::uint16_t&, const boost::uint16_t&);
	    /** @brief Retrieve the x-dimension of the output canvas */
	    G_API_GENEVA boost::uint16_t getXDim() const;
	    /** @brief Retrieve the y-dimension of the output canvas */
	    G_API_GENEVA boost::uint16_t getYDim() const;

	    /** @brief Allows to set the name of the result file */
	    G_API_GENEVA void setResultFileName(const std::string&);
	    /** @brief Allows to retrieve the name of the result file */
	    G_API_GENEVA std::string getResultFileName() const;

	protected:
	    /** @brief A function that is called once before the optimization starts */
	    virtual G_API_GENEVA void firstInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;
	    /** @brief A function that is called during each optimization cycle */
	    virtual G_API_GENEVA void cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;
	    /** @brief A function that is called once at the end of the optimization cycle */
	    virtual G_API_GENEVA void lastInformation(GOptimizationAlgorithmT<GParameterSet> * const) OVERRIDE;

	    /** @brief Loads the data of another object */
	    virtual G_API_GENEVA void load_(const GObject*) OVERRIDE;
	    /** @brief Creates a deep clone of this object */
		virtual G_API_GENEVA GObject* clone_() const OVERRIDE;

	private:
		boost::uint16_t xDim_; ///< The dimension of the canvas in x-direction
		boost::uint16_t yDim_; ///< The dimension of the canvas in y-direction

		std::string resultFile_; ///< The name of the file to which data is emitted

      boost::shared_ptr<Gem::Common::GGraph2D> fitnessGraph_; ///< Holds the fitness data until plotted

	public:
		/** @brief Applies modifications to this object. This is needed for testing purposes */
		virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
		/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
		virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
		/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
		virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;

		/************************************************************************/
	};

   /***************************************************************************/
   /////////////////////////////////////////////////////////////////////////////
   /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBaseSwarm)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBaseSwarm::GSwarmOptimizationMonitor)

#endif /* GBASESWARM_HPP_ */
