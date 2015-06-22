/**
 * @file GBaseGD.hpp
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
#include <memory>

// Boost headers go here

#ifndef GBASEGD_HPP_
#define GBASEGD_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GGDPersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GTestIndividual1.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/**
 * The default number of simultaneous starting points for the gradient descent
 */
const std::size_t DEFAULTGDSTARTINGPOINTS=1;
const double DEFAULTFINITESTEP=0.001;
const double DEFAULTSTEPSIZE=0.1;

/******************************************************************************/
/**
 * The GBaseGD class implements a steepest descent algorithm. It is possible
 * to search for optima starting from several positions simultaneously.
 */
class GBaseGD
	:public GOptimizationAlgorithmT<GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GOptimizationAlgorithmT_GParameterSet",
					  boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet>>(*this))
		& BOOST_SERIALIZATION_NVP(nStartingPoints_)
		& BOOST_SERIALIZATION_NVP(nFPParmsFirst_)
		& BOOST_SERIALIZATION_NVP(finiteStep_)
		& BOOST_SERIALIZATION_NVP(stepSize_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief An easy identifier for the class */
	static G_API_GENEVA const std::string nickname; // Initialized in the .cpp definition file

	/** @brief The default constructor */
	G_API_GENEVA GBaseGD();
	/** @brief Initialization with the number of starting points and the size of the finite step */
	G_API_GENEVA GBaseGD(const std::size_t&, const double&, const double&);
	/** @brief A standard copy constructor */
	G_API_GENEVA GBaseGD(const GBaseGD&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GBaseGD();

	/** @brief The standard assignment operator */
	G_API_GENEVA const GBaseGD& operator=(const GBaseGD&);

	/** @brief Checks for equality with another GBaseGD object */
	virtual G_API_GENEVA bool operator==(const GBaseGD&) const;
	/** @brief Checks for inequality with another GBaseGD object */
	virtual G_API_GENEVA bool operator!=(const GBaseGD&) const;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Loads a checkpoint */
	virtual G_API_GENEVA void loadCheckpoint(const boost::filesystem::path&) override;

	/** @brief Returns information about the type of optimization algorithm */
	virtual G_API_GENEVA std::string getOptimizationAlgorithm() const override;

	/** @brief Retrieves the number of starting points of the algorithm */
	G_API_GENEVA std::size_t getNStartingPoints() const;
	/** @brief Allows to set the number of starting points for the gradient descent */
	G_API_GENEVA void setNStartingPoints(std::size_t);

	/** @brief Set the size of the finite step of the adaption process */
	G_API_GENEVA void setFiniteStep(double);
	/** @brief Retrieve the size of the finite step of the adaption process */
	G_API_GENEVA double getFiniteStep() const;

	/** @brief Sets a multiplier for the adaption process */
	G_API_GENEVA void setStepSize(double);
	/** @brief Retrieves the current step size */
	G_API_GENEVA double getStepSize() const;

	/** @brief Retrieves the number of processable items for the current iteration */
	virtual G_API_GENEVA std::size_t getNProcessableItems() const override;

	/** @brief Returns the name of this optimization algorithm */
	virtual G_API_GENEVA std::string getAlgorithmName() const override;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) override;

	/** @brief Emits a name for this class / object */
	virtual G_API_GENEVA std::string name() const override;

protected:
	/***************************************************************************/
	/** @brief Loads the data of another population */
	virtual G_API_GENEVA void load_(const GObject *) override;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject *clone_() const override = 0;

	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual G_API_GENEVA boost::tuple<double, double> cycleLogic() override;
	/** @brief Does some preparatory work before the optimization starts */
	virtual G_API_GENEVA void init() override;
	/** @brief Does any necessary finalization work */
	virtual G_API_GENEVA void finalize() override;

	/** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
	virtual G_API_GENEVA std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const override;

	/** @brief Resizes the population to the desired level and does some error checks */
	virtual G_API_GENEVA void adjustPopulation() override;

	/** @brief Saves the state of the class to disc. */
	virtual G_API_GENEVA void saveCheckpoint() const override;

	/** @brief Triggers fitness calculation of a number of individuals */
	virtual G_API_GENEVA void runFitnessCalculation() override = 0;
	/** @brief Updates the individual parameters of children */
	virtual G_API_GENEVA void updateChildParameters();
	/** @brief Performs a step of the parent individuals */
	virtual G_API_GENEVA void updateParentIndividuals();

private:
	/***************************************************************************/
	std::size_t nStartingPoints_ = DEFAULTGDSTARTINGPOINTS; ///< The number of starting positions in the parameter space
	std::size_t nFPParmsFirst_ = 0; ///< The amount of floating point values in the first individual

	double finiteStep_ = DEFAULTFINITESTEP; ///< The size of the incremental adaption of the feature vector
	double stepSize_ = DEFAULTSTEPSIZE; ///< A multiplicative factor for the adaption
	long double stepRatio_ = (DEFAULTSTEPSIZE / DEFAULTFINITESTEP); ///< The ratio of stepSize_ and finiteStep_. NOTE: long double; Will be recalculated in init()

	std::vector<double> dblLowerParameterBoundaries_ = std::vector<double>(); ///< Holds lower boundaries of double parameters; Will be extracted in init()
	std::vector<double> dblUpperParameterBoundaries_ = std::vector<double>(); ///< Holds upper boundaries of double parameters; Will be extracted in init()
	std::vector<double> adjustedFiniteStep_ = std::vector<double>(); ///< A step-size normalized to each parameter range; Will be recalculated in init()

	/** @brief Lets individuals know about their position in the population */
	void markIndividualPositions();

public
	/***************************************************************************/:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() override;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;

public:
	/***************************************************************************/
	/////////////////////////////////////////////////////////////////////////////
	/***************************************************************************/
	/**
	 * This class defines the interface of optimization monitors, as used
	 * by default in the Geneva library for evolutionary algorithms.
	 */
	class GGDOptimizationMonitor
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
		G_API_GENEVA GGDOptimizationMonitor();
		/** @brief The copy constructor */
		G_API_GENEVA GGDOptimizationMonitor(const GGDOptimizationMonitor&);
		/** @brief The destructor */
		virtual G_API_GENEVA ~GGDOptimizationMonitor();

		/** @brief The standard assignment operator */
		const GGDOptimizationMonitor& operator=(const GGDOptimizationMonitor&);

		/** @brief Checks for equality with another GParameter Base object */
		virtual G_API_GENEVA bool operator==(const GGDOptimizationMonitor&) const;
		/** @brief Checks for inequality with another GGDOptimizationMonitor object */
		virtual G_API_GENEVA bool operator!=(const GGDOptimizationMonitor&) const;

		/** @brief Searches for compliance with expectations with respect to another object of the same type */
		virtual G_API_GENEVA void compare(
			const GObject& // the other object
			, const Gem::Common::expectation& // the expectation for this object, e.g. equality
			, const double& // the limit for allowed deviations of floating point types
		) const override;

		/** @brief Set the dimension of the output canvas */
		G_API_GENEVA void setDims(const boost::uint32_t&, const boost::uint32_t&);
		/** @brief Retrieve the dimensions as a tuple */
		G_API_GENEVA boost::tuple<boost::uint32_t, boost::uint32_t> getDims() const;
		/** @brief Retrieve the x-dimension of the output canvas */
		G_API_GENEVA boost::uint32_t getXDim() const;
		/** @brief Retrieve the y-dimension of the output canvas */
		G_API_GENEVA boost::uint32_t getYDim() const;

		/** @brief Allows to set the name of the result file */
		G_API_GENEVA void setResultFileName(const std::string&);
		/** @brief Allows to retrieve the name of the result file */
		G_API_GENEVA std::string getResultFileName() const;

	protected:
		/** @brief A function that is called once before the optimization starts */
		virtual G_API_GENEVA void firstInformation(GOptimizationAlgorithmT<GParameterSet> * const) override;
		/** @brief A function that is called during each optimization cycle */
		virtual G_API_GENEVA void cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const) override;
		/** @brief A function that is called once at the end of the optimization cycle */
		virtual G_API_GENEVA void lastInformation(GOptimizationAlgorithmT<GParameterSet> * const) override;

		/** @brief Loads the data of another object */
		virtual G_API_GENEVA void load_(const GObject*) override;
		/** @brief Creates a deep clone of this object */
		virtual G_API_GENEVA GObject* clone_() const override;

	private:
		boost::uint32_t xDim_; ///< The dimension of the canvas in x-direction
		boost::uint32_t yDim_; ///< The dimension of the canvas in y-direction

		std::string resultFile_; ///< The name of the file to which data is emitted

		std::shared_ptr<Gem::Common::GGraph2D> fitnessGraph_; ///< Holds the fitness data until plotted

	public:
		/** @brief Applies modifications to this object. This is needed for testing purposes */
		virtual G_API_GENEVA bool modify_GUnitTests() override;
		/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
		virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
		/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
		virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;

		/************************************************************************/
	};

	/***************************************************************************/
	/////////////////////////////////////////////////////////////////////////////
	/***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GBaseGD)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBaseGD::GGDOptimizationMonitor)

#endif /* GBASEGD_HPP_ */
