/**
 * @file GBasePopulation.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard headers go here
#include <string>
#include <sstream>
#include <cmath>

// Boost headers go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

/**
 * Check that we have support for threads
 */
#ifndef BOOST_HAS_THREADS
#error "Error: Support for multi-threading does not seem to be available."
#endif

#ifndef GBASEPOPULATION_HPP_
#define GBASEPOPULATION_HPP_

// GenEvA headers go here

#include "GIndividual.hpp"
#include "GIndividualSet.hpp"
#include "GRandom.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"
#include "GEnums.hpp"

namespace Gem {
namespace GenEvA {

/**
 * The two const variables MUPLUSNU and MUCOMMANU determine, whether new parents
 * should be selected from the entire population or from the children only
 */
const bool MUPLUSNU = true;
const bool MUCOMMANU = false;

/**
 * The two const variables MAXIMIZE and MINIMIZE determine, whether the library
 * should work in maximization or minimization mode.
 */
const bool MAXIMIZE = true;
const bool MINIMIZE = false;

/**
 * The number of generations after which information should be
 * emitted about the inner state of the population.
 */
const boost::int32_t DEFAULTREPORTGEN = 20;

/**
 * The default maximum number of generations
 */
const boost::int32_t DEFAULTMAXGEN = 100;

/**
 * The default maximization mode
 */
const bool DEFAULTMAXMODE = false;

/**
 * A 0 time period . timedHalt will not trigger if this duration is set
 */
const std::string EMPTYDURATION = "00:00:00.000"; // 0 - no duration

/**
 * The default maximum duration of the calculation.
 */
const std::string DEFAULTDURATION = EMPTYDURATION;

/*********************************************************************************/
/**
 * The GBasePopulation class adds the notion of parents and children to
 * the GIndividualSet class. The evolutionary adaptation is realized
 * through the cycle of mutation, evaluation and sorting, as defined in this
 * class.
 *
 * Populations are collections of individuals, which themselves are classes
 * exhibiting the GIndividual class'es API, most notably the GIndividual::fitness() and
 * GIndividual::mutate() functions. Individuals thus can themselves be populations,
 * which can again contain populations, and so on.
 *
 * In order to add parents to this class, use the default constructor, then add
 * at least one GIndividual to the class and call setPopulationSize(). The population
 * will then "filled up" with missing individuals as required, before the optimization
 * starts. Note that this class will enforce a minimum, default number of children,
 * as implied by the population size and the number of parents set in the beginning.
 */
class GBasePopulation
	:public GIndividualSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;

		ar & make_nvp("GIndividualSet",
				boost::serialization::base_object<GIndividualSet>(*this));
		ar & make_nvp("nParents_", nParents_);
		ar & make_nvp("popSize_", popSize_);
		ar & make_nvp("generation_", generation_);
		ar & make_nvp("maxGeneration_", maxGeneration_);
		ar & make_nvp("reportGeneration_", reportGeneration_);
		ar & make_nvp("recombinationMethod_", recombinationMethod_);
		ar & make_nvp("muplusnu_", muplusnu_);
		ar & make_nvp("maximize_", maximize_);
		ar & make_nvp("maxDuration_", maxDuration_);
		ar & make_nvp("defaultNChildren_", defaultNChildren_);
		// Note that id and firstId_ are not serialized as we need the id
		// to be recalculated for de-serialized objects. Likewise, startTime_
		// doesn't need to be serialized.
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBasePopulation();
	/** @brief A standard copy constructor */
	GBasePopulation(const GBasePopulation&);
	/** @brief The destructor */
	virtual ~GBasePopulation();

	/** @brief A standard assignment operator */
	const GBasePopulation& operator=(const GBasePopulation&);

	/** @brief Loads the data of another population */
	virtual void load(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone();

	/** @brief The core function of the entire GenEvA library.
	 * Triggers the optimization of a population. */
	virtual void optimize();

	/** @brief Emits information specific to this population */
	virtual void doInfo(const infoMode&);

	/** @brief Registers a function to be called when emitting information from doInfo */
	void registerInfoFunction(boost::function<void (const infoMode&, GBasePopulation * const)>);

	/** @brief Sets population size and number of parents */
	void setPopulationSize(const std::size_t&, const std::size_t&);

	/** @brief Retrieve the number of parents in this population */
	std::size_t getNParents() const;
	/** @brief Retrieve the number of children in this population */
	std::size_t getNChildren() const;
	/** @brief Retrieve the current population size */
	std::size_t getPopulationSize() const;
	/** @brief Retrieves the defaultNChildren_ parameter */
	std::size_t getDefaultNChildren() const;
	/** @brief Retrieves the default population size */
	std::size_t getDefaultPopulationSize() const;

	/** @brief Set the sorting scheme for this population (MUPLUSNU or MUCOMMANU) */
	void setSortingScheme(const bool&);
	/** @brief Retrieve the current sorting scheme for this population */
	bool getSortingScheme() const;

	/** @brief Set the number of generations after which sorting should be stopped */
	void setMaxGeneration(const boost::uint32_t&);
	/** @brief Retrieve the number of generations after which sorting should be stopped */
	boost::uint32_t getMaxGeneration() const;
	/** @brief Get information about the current generation */
	boost::uint32_t getGeneration() const;

	/** @brief Sets the maximum allowed processing time */
	void setMaxTime(boost::posix_time::time_duration);
	/** @brief Retrieves the maximum allowed processing time */
	boost::posix_time::time_duration getMaxTime();

	/** @brief Specify whether we want to work in maximization or minimization mode */
	void setMaximize(const bool& val);
	/** @brief Find out whether we work in maximization or minimization mode */
	bool getMaximize() const;

	/** @brief Specify, what recombination mode should be used */
	void setRecombinationMethod(const recoScheme&);
	/** @brief Find out, what recombination mode is being used */
	recoScheme getRecombinationMethod() const;

	/** @brief Sets the number of generations after which the population should
	 * report about its inner state. */
	void setReportGeneration(const boost::uint32_t&);
	/** @brief Returns the number of generations after which the population should
	 * report about its inner state. */
	boost::uint32_t getReportGeneration() const;

	/** @brief Retrieve the id of this class */
	std::string getId();

	/** @brief Retrieves the best individual of this population */
	boost::shared_ptr<GIndividual> getBestIndividual();

protected:
	/** @brief user-defined halt-criterium for the optimization */
	virtual bool customHalt();
	/** @brief user-defined recombination scheme */
	virtual void customRecombine();

	/** @brief Creates children from parents according to a predefined recombination scheme */
	virtual void recombine();
	/** @brief Mutates all children of this population */
	virtual void mutateChildren();
	/** @brief Selects the best children of the population */
	virtual void select();

	/** @brief The mutation scheme for this population */
	virtual void customMutations();
	/** @brief The evaluation scheme for this population */
	virtual double fitnessCalculation();

	/** @brief Marks parents as parents and children as children */
	void markParents();

	/** @brief Lets individuals know about the current generation */
	void markGeneration();

private:
	/** @brief Emits true once a given time has passed */
	bool timedHalt();
	/** @brief Adjusts the actual population size to the desired value */
	void adjustPopulation();

	/** @brief Determines when to stop the optimization */
	bool halt();

	/** @brief Implements the RANDOMRECOMBINE recombination scheme */
	void randomRecombine(boost::shared_ptr<GIndividual>&);
	/** @brief Implements the VALUERECOMBINE recombination scheme */
	void valueRecombine(boost::shared_ptr<GIndividual>&);

	/***********************************************************************************/
	/**
	 * Emits information about the population it has been given. This is the default
	 * information function provided for all populations. Information is emitted in the
	 * format of the ROOT analysis toolkit (see http://root.cern.ch). Note that we are
	 * using a static member function in order to avoid storing a local "this" pointer in
	 * this function when registering it in the boost::function object. This might otherwise
	 * be problematic when copying the boost::function object.
	 *
	 * @param im Indicates the information mode
	 * @param gbp A pointer to the population information should be emitted about
	 */
	static void defaultInfoFunction(const infoMode& im, GBasePopulation * const gbp) {
		std::ostringstream information;

		switch(im){
		case INFOINIT:
			information << "{" << std::endl
						<< "  TH1F *h" << gbp << " = new TH1F(\"h"
						<< gbp << "\",\"h" << gbp << "\"," << gbp->getMaxGeneration()+1
						<< ",0," << gbp->getMaxGeneration() << ");" << std::endl << std::endl;
			break;

		case INFOPROCESSING:
			{
				bool isDirty = false;

				information << std::setprecision(10) << "  h" << gbp
							<< "->Fill(" << gbp->getGeneration() << ", "
							<< gbp->data.at(0)->getCurrentFitness(isDirty) << ");";

				if(isDirty) {
					information << "// dirty!";
				}
				information << std::endl;
			}
			break;

		case INFOEND:
			information << std::endl
						<< "  h"<< gbp << "->Draw();" << std::endl
						<< "}" << std::endl;
			break;
		}

		LOGGER->log(information.str(), Gem::GLogFramework::PROGRESS);
	}
	/***********************************************************************************/

	std::size_t nParents_; ///< The number of parents
	std::size_t popSize_; ///< The size of the population. Only used in adjustPopulation()
	boost::uint32_t generation_; ///< The current generation
	boost::uint32_t maxGeneration_; ///< The maximum number of generations
	boost::uint32_t reportGeneration_; ///< Number of generations after which a report should be issued
	recoScheme recombinationMethod_; ///< The chosen recombination method
	bool muplusnu_; ///< The chosen sorting scheme
	bool maximize_; ///< The optimization mode (minimization/false vs. maximization/true)
	std::string id_; ///< A unique id, used in networking contexts
	bool firstId_; ///< Is this the first call to getId() ?
	boost::posix_time::time_duration maxDuration_; ///< Maximum time frame for the optimization
	boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization
	std::size_t defaultNChildren_; ///< Expected number of children

	boost::function<void (const infoMode&, GBasePopulation * const)> infoFunction_; ///< Used to emit information with doInfo()
};

/*********************************************************************************/

}
} /* namespace Gem::GenEvA */

#endif /*GBASEPOPULATION_HPP_*/
