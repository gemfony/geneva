/**
 * @file GEvolutionaryAlgorithm.hpp
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

#ifndef GEVOLUTIONARYALGORITHM_HPP_
#define GEVOLUTIONARYALGORITHM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GIndividual.hpp"
#include "GOptimizationAlgorithmT.hpp"
#include "GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/**
 * The default sorting mode
 */
const sortingMode DEFAULTSMODE=MUPLUSNU;

/**
 * The default number of generations without improvement after which
 * a micro-training should be started. A value of 0 means that no
 * micro-training will take place.
 */
const boost::uint32_t DEFAULTMICROTRAININGINTERVAL=0;

/*********************************************************************************/
/**
 * The GEvolutionaryAlgorithm class adds the notion of parents and children to
 * the GOptimizationAlgorithm class. The evolutionary adaptation is realized
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
class GEvolutionaryAlgorithm
	:public GOptimizationAlgorithmT<Gem::Geneva::GIndividual>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("GOptimizationAlgorithmT_GIndividual", boost::serialization::base_object<GOptimizationAlgorithmT<Gem::Geneva::GIndividual> >(*this))
		   & BOOST_SERIALIZATION_NVP(nParents_)
		   & BOOST_SERIALIZATION_NVP(microTrainingInterval_)
		   & BOOST_SERIALIZATION_NVP(recombinationMethod_)
		   & BOOST_SERIALIZATION_NVP(smode_)
		   & BOOST_SERIALIZATION_NVP(defaultNChildren_)
		   & BOOST_SERIALIZATION_NVP(oneTimeMuCommaNu_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GEvolutionaryAlgorithm();
	/** @brief A standard copy constructor */
	GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm&);
	/** @brief The destructor */
	virtual ~GEvolutionaryAlgorithm();

	/** @brief A standard assignment operator */
	const GEvolutionaryAlgorithm& operator=(const GEvolutionaryAlgorithm&);

	/** @brief Checks for equality with another GEvolutionaryAlgorithm object */
	bool operator==(const GEvolutionaryAlgorithm&) const;
	/** @brief Checks for inequality with another GEvolutionaryAlgorithm object */
	bool operator!=(const GEvolutionaryAlgorithm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Common::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Emits information specific to this population */
	virtual void doInfo(const infoMode&);

	/** @brief Registers a function to be called when emitting information from doInfo */
	void registerInfoFunction(boost::function<void (const infoMode&, GEvolutionaryAlgorithm * const)>);

	/** @brief Sets the default population size and number of parents */
	void setDefaultPopulationSize(const std::size_t&, const std::size_t&);

	/** @brief Retrieve the number of parents in this population */
	std::size_t getNParents() const;
	/** @brief Retrieve the number of children in this population */
	std::size_t getNChildren() const;
	/** @brief Retrieves the defaultNChildren_ parameter */
	std::size_t getDefaultNChildren() const;

	/** @brief Set the sorting scheme for this population */
	void setSortingScheme(const sortingMode&);
	/** @brief Retrieve the current sorting scheme for this population */
	sortingMode getSortingScheme() const;

	/** @brief Specify, what recombination mode should be used */
	void setRecombinationMethod(const recoScheme&);
	/** @brief Find out, what recombination mode is being used */
	recoScheme getRecombinationMethod() const;

	/** @brief Loads a checkpoint from disk */
	virtual void loadCheckpoint(const std::string&);

	//------------------------------------------------------------------------------------------
	// Settings specific to micro-training
	/** @brief Set the interval in which micro training should be performed */
	void setMicroTrainingInterval(const boost::uint32_t&);
	/** @brief Retrieve the interval in which micro training should be performed */
	boost::uint32_t getMicroTrainingInterval() const;

	/**************************************************************************************************/
	/**
	 * Retrieves the best individual of the population and casts it to the desired type.
	 * Note that this function will only be accessible to the compiler if individual_type is a derivative
	 * of GIndividual, thanks to the magic of Boost's enable_if and Type Traits libraries. Hence
	 * we do not need to check convertibility using dynamic_cast<>.
	 *
	 * @return A converted shared_ptr to the best (i.e. first) individual of the population
	 */
	template <typename individual_type>
	inline boost::shared_ptr<individual_type> getBestIndividual(
			typename boost::enable_if<boost::is_base_of<GIndividual, individual_type> >::type* dummy = 0
	){
#ifdef DEBUG
		if(data.empty()) {
			std::ostringstream error;
			error << "In GEvolutionaryAlgorithm::getBestIndividual<individual_type>() : Error!" << std::endl
				  << "Tried to access individual at position 0 even though population is empty." << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		boost::shared_ptr<individual_type> p = boost::dynamic_pointer_cast<individual_type>(data[0]);

		if(p) return p;
		else {
			std::ostringstream error;
			error << "In GEvolutionaryAlgorithm::getBestIndividual<individual_type>() : Conversion error!" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}
#else
		return boost::static_pointer_cast<individual_type>(data[0]);
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
	 * @param gbp A pointer to the population information should be emitted about
	 */
	static void simpleInfoFunction(const infoMode& im, GEvolutionaryAlgorithm * const gbp) {
		std::ostringstream information;

		switch(im){
		case INFOINIT: // nothing
			break;

		case INFOPROCESSING:
			{
				bool isDirty = false;

				information << std::setprecision(10) << "In iteration "<< gbp->getIteration() << ": " << gbp->getBestFitness() << std::endl;
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

private:
	/** @brief Enforces a one-time selection policy of MUCOMMANU */
	void setOneTimeMuCommaNu();
	/** @brief Updates the parent's structure */
	bool updateParentStructure();

	/** @brief Saves the state of the class to disc. Private, as we do not want to accidently trigger value calculation  */
	virtual void saveCheckpoint() const;

	/** @brief Implements the RANDOMRECOMBINE recombination scheme */
	void randomRecombine(boost::shared_ptr<GIndividual>&);
	/** @brief Implements the VALUERECOMBINE recombination scheme */
	void valueRecombine(boost::shared_ptr<GIndividual>&, const std::vector<double>&);

	/** @brief Selection, MUPLUSNU style */
	void sortMuplusnuMode();
	/** @brief Selection, MUCOMMANU style */
	void sortMucommanuMode();
	/** @brief Selection, MUNU1PRETAIN style */
	void sortMunu1pretainMode();

	std::size_t nParents_; ///< The number of parents
	boost::uint32_t microTrainingInterval_; ///< The number of generations without improvements after which a micro training should be started
	recoScheme recombinationMethod_; ///< The chosen recombination method
	sortingMode smode_; ///< The chosen sorting scheme
	std::size_t defaultNChildren_; ///< Expected number of children
	bool oneTimeMuCommaNu_; ///< Specifies whether a one-time selection scheme of MUCOMMANU should be used

	boost::function<void (const infoMode&, GEvolutionaryAlgorithm * const)> infoFunction_; ///< Used to emit information with doInfo()

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

/*********************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GEVOLUTIONARYALGORITHM_HPP_ */
