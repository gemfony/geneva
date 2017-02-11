/**
 * @file GParameterSetParChild.hpp
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
#include <tuple>

// Boost headers go here

#ifndef GPARAMETERSETPARCHILD_HPP_
#define GPARAMETERSETPARCHILD_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GBaseParChildT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This is a specialization of the GBaseParChildT<ind_type> class for GParameterSet
 * objects that adds the option to perform an amalgamation of objects, such as a
 * cross-over. Almost all of Geneva's EA-algorithms will use this class as their
 * base class (except those that deal with multi-populations).
 */
class GParameterSetParChild
	:public GBaseParChildT<GParameterSet>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBaseParChildT_GParameterSet", boost::serialization::base_object<GBaseParChildT<GParameterSet>>(*this))
		& BOOST_SERIALIZATION_NVP(amalgamationLikelihood_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GParameterSetParChild();
	/** @brief A standard copy constructor */
	G_API_GENEVA GParameterSetParChild(const GParameterSetParChild&);
	/** @brief The destructor */
	virtual G_API_GENEVA ~GParameterSetParChild();

	/** @brief The standard assignment operator */
	G_API_GENEVA const GParameterSetParChild& operator=(const GParameterSetParChild&);

	/** @brief Checks for equality with another GParameterSetParChild object */
	G_API_GENEVA bool operator==(const GParameterSetParChild&) const;
	/** @brief Checks for inequality with another GParameterSetParChild object */
	G_API_GENEVA bool operator!=(const GParameterSetParChild&) const;

	/** @brief Searches for compliance with expectations with respect to another object of the same type */
	virtual G_API_GENEVA void compare(
		const GObject& // the other object
		, const Gem::Common::expectation& // the expectation for this object, e.g. equality
		, const double& // the limit for allowed deviations of floating point types
	) const override;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual G_API_GENEVA void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) override;

	/** @brief Allows to set the likelihood for amalgamation of two units to be performed instead of "just" duplication. */
	G_API_GENEVA void setAmalgamationLikelihood(double);
	/** @brief Allows to retrieve the likelihood for amalgamation of two units to be performed instead of "just" duplication. */
	G_API_GENEVA double getAmalgamationLikelihood() const;

protected:
	/***************************************************************************/
	/** @brief Performs recombination, taking into account possible amalgamation actions */
	virtual G_API_GENEVA void doRecombine() override;
	/** @brief Marks the number of stalled optimization attempts in all individuals and gives them an opportunity to update their internal structures. */
	virtual G_API_GENEVA void actOnStalls() override;

	/** @brief Does some preparatory work before the optimization starts */
	virtual G_API_GENEVA void init() override;
	/** @brief Does any necessary finalization work */
	virtual G_API_GENEVA void finalize() override;

	/** @brief Loads the data of another population */
	virtual G_API_GENEVA void load_(const GObject *) override;

	double amalgamationLikelihood_; ///< Likelihood for children to be created by cross-over rather than "just" duplication (note that they may nevertheless be mutated)

private:
	/** @brief Uniformly distributed integer random numbers */
	std::uniform_int_distribution<std::size_t> m_uniform_int;

public:
	/***************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() override;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() override;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() override;
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterSetParChild)

#endif /* GPARAMETERSETPARCHILD_HPP_ */
