/**
 * @file GEvolutionaryAlgorithm.hpp
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

// Standard header files go here

// Boost header files go here

#ifndef GEVOLUTIONARYALGORITHM_HPP_
#define GEVOLUTIONARYALGORITHM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GSerialEA.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * This class acts as a wrapper arount the different modes of evolutionary algorithms
 * in Geneva (serial, multi-threaded or broker-based)
 */
class GEvolutionaryAlgorithm
	: public GMutableSetT<GIndividual>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GMutableSetT_GParameterSet", boost::serialization::base_object<GMutableSetT<GIndividual> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GEvolutionaryAlgorithm();
	/** @brief A copy constructor */
	GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm&);
	/** @brief The destructor */
	virtual ~GEvolutionaryAlgorithm();

	/** @brief Standard assignment operator */
	const GEvolutionaryAlgorithm& operator=(const GEvolutionaryAlgorithm&);

	/** @brief Checks for equality with another Go object */
	bool operator==(const GEvolutionaryAlgorithm&) const;
	/** @brief Checks for inequality with another Go object */
	bool operator!=(const GEvolutionaryAlgorithm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Allows to specify an optimization monitor to be used with evolutionary algorithms */
	void registerOptimizationMonitor(boost::shared_ptr<GSerialEA::GEAOptimizationMonitor>);

	/** @brief Allows to randomly initialize parameter members. Unused in this wrapper object */
	virtual void randomInit();
	/** @brief Triggers fitness calculation (i.e. optimization) for this object */
	virtual double fitnessCalculation();

	/**************************************************************************************/
	// The following is a trivial list of getters and setters
	void setPersonality(const personality&);
	personality getPersonality() const;

	void setParallelizationMode(const parMode&);
	parMode getParallelizationMode() const;

	void setConfigFileName(const std::string&);
	std::string getConfigFileName() const;

	void setMaxIterations(const boost::uint32_t&);
	boost::uint32_t getMaxIterations() const;

	void setMaxStallIteration(const boost::uint32_t&);
	boost::uint32_t getMaxStallIteration() const;

	void setMaxMinutes(const long&);
	long getMaxMinutes() const;

	void setReportIteration(const boost::uint32_t&);
	boost::uint32_t getReportIteration() const;

	void setOffset(const boost::uint32_t&);
	boost::uint32_t getOffset() const;

	void setEAPopulationSize(const std::size_t&);
	std::size_t getEAPopulationSize() const;

	void setEANParents(const std::size_t&);
	std::size_t getEANParents() const;

	void setEARecombinationScheme(const recoScheme&);
	recoScheme getEARecombinationScheme() const;

	void setEASortingScheme(const sortingMode&);
	sortingMode getEASortingScheme() const;

	void setEATrackParentRelations(const bool&);
	bool getEATrackParentRelations() const;

	void setEAGrowthRate(const std::size_t&);
	std::size_t getEAGrowthRate() const;

	void setEAMaxPopSize(const std::size_t&);
	std::size_t getEAMaxPopSize() const;


private:
};

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GEvolutionaryAlgorithm)

#endif /* GEVOLUTIONARYALGORITHM_HPP_ */
