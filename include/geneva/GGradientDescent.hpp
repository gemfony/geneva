/**
 * @file GGradientDescent.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

#ifndef GGRADIENTDESCENT_HPP_
#define GGRADIENTDESCENT_HPP_

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
#include "GTestIndividual1.hpp"
#endif /* GENEVATESTING */

namespace Gem {
namespace Geneva {

/**
 * The default number of simultaneous starting points for the gradient descent
 */
const std::size_t DEFAULTGDSTARTINGPOINTS=1;
const double DEFAULTFINITESTEP=0.01;
const double DEFAULTSTEPSIZE=0.1;

/*********************************************************************************/
/**
 * The GGradientDescent class implements a steepest descent algorithm. It is possible
 * to search for optima starting from several positions simultaneously.
 */
class GGradientDescent
	:public GOptimizationAlgorithmT<GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("GOptimizationAlgorithmT_GParameterSet", boost::serialization::base_object<GOptimizationAlgorithmT<GParameterSet> >(*this))
		   & BOOST_SERIALIZATION_NVP(nStartingPoints_)
		   & BOOST_SERIALIZATION_NVP(nFPParmsFirst_)
		   & BOOST_SERIALIZATION_NVP(finiteStep_)
		   & BOOST_SERIALIZATION_NVP(stepSize_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GGradientDescent();
	/** @brief Initialization with the number of starting points and the size of the finite step */
	GGradientDescent(const std::size_t&, const float&, const float&);
	/** @brief A standard copy constructor */
	GGradientDescent(const GGradientDescent&);
	/** @brief The destructor */
	virtual ~GGradientDescent();

	/** @brief A standard assignment operator */
	const GGradientDescent& operator=(const GGradientDescent&);

	/** @brief Checks for equality with another GGradientDescent object */
	bool operator==(const GGradientDescent&) const;
	/** @brief Checks for inequality with another GGradientDescent object */
	bool operator!=(const GGradientDescent&) const;

	/** @brief Sets the individual's personality types to GradientDescent */
	void setIndividualPersonalities();

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Emits information specific to this population */
	virtual void doInfo(const infoMode&);

	/** @brief Registers a function to be called when emitting information from doInfo */
	void registerInfoFunction(boost::function<void (const infoMode&, GGradientDescent * const)>);

	/** @brief Loads a checkpoint from disk */
	virtual void loadCheckpoint(const std::string&);

	/** @brief Retrieves the number of starting points of the algorithm */
	std::size_t getNStartingPoints() const;
	/** @brief Allows to set the number of starting points for the gradient descent */
	void setNStartingPoints(const std::size_t&);

	/** @brief Set the size of the finite step of the adaption process */
	void setFiniteStep(const float&);
	/** @brief Retrieve the size of the finite step of the adaption process */
	float getFiniteStep() const;

	/** @brief Sets a multiplier for the adaption process */
	void setStepSize(const float&);
	/** @brief Retrieves the current step size */
	float getStepSize() const;

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
	 * @param gd A pointer to the population information should be emitted about
	 */
	static void simpleInfoFunction(const infoMode& im, GGradientDescent * const gd) {
		std::ostringstream information;

		switch(im){
		case INFOINIT: // nothing
			break;

		case INFOPROCESSING:
			{
				information << std::setprecision(10) << "In iteration "<< gd->getIteration() << ": " << gd->getBestFitness() << std::endl;
			}
			break;

		case INFOEND:
			{
				information << std::setprecision(10) << "Best fitness found: " << gd->getBestFitness() << std::endl;
			}
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

	/** @brief Triggers fitness calculation of a number of individuals */
	virtual double doFitnessCalculation(const std::size_t&);
	/** @brief Updates the individual parameters of children */
	virtual void updateChildParameters();
	/** @brief Performs a step of the parent individuals */
	virtual void updateParentIndividuals();

	boost::function<void (const infoMode&, GGradientDescent * const)> infoFunction_; ///< Used to emit information with doInfo()

private:
	/**************************************************************************************************/
	std::size_t nStartingPoints_; ///< The number of starting positions in the parameter space
	std::size_t nFPParmsFirst_; ///< The amount of floating point values in the first individual
	float finiteStep_; ///< The size of the incremental adaption of the feature vector
	float stepSize_; ///< A multiplicative factor for the adaption

	/** @brief Lets individuals know about their position in the population */
	void markIndividualPositions();

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

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/** @brief We need to provide a specialization of the factory function that creates objects of this type. */
template <> boost::shared_ptr<Gem::Geneva::GGradientDescent> TFactory_GUnitTests<Gem::Geneva::GGradientDescent>();

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */

#endif /* GGRADIENTDESCENT_HPP_ */
