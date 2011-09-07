/**
 * @file GBrokerGD.hpp
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

#ifndef GBROKERGD_HPP_
#define GBROKERGD_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GHelperFunctions.hpp"
#include "common/GThreadWrapper.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GBaseGD.hpp"
#include "geneva/GIndividual.hpp"

#ifdef GENEVATESTING
#include "geneva-individuals/GTestIndividual1.hpp"
#endif /* GENEVATESTING */

namespace Gem {
namespace Geneva {

/*********************************************************************************/
/**
 * A networked version of the GBaseGD class
 */
class GBrokerGD
	: public GBaseGD
	, public Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GBaseGD)
		   & make_nvp("GBrokerConnectorT_GIndividual", boost::serialization::base_object<Gem::Courtier::GBrokerConnectorT<GIndividual> >(*this))
		   & BOOST_SERIALIZATION_NVP(maxResubmissions_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBrokerGD();
	/** @brief Initialization with the number of starting points and the size of the finite step */
	GBrokerGD(const std::size_t&, const float&, const float&);
	/** @brief A standard copy constructor */
	GBrokerGD(const GBrokerGD&);
	/** @brief The destructor */
	virtual ~GBrokerGD();

	/** @brief A standard assignment operator */
	const GBrokerGD& operator=(const GBrokerGD&);

	/** @brief Checks for equality with another GBrokerGD object */
	bool operator==(const GBrokerGD&) const;
	/** @brief Checks for inequality with another GBrokerGD object */
	bool operator!=(const GBrokerGD&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Allows to set the maximum allowed number of resubmissions */
	void setMaxResubmissions(std::size_t maxResubmissions);
	/** @brief Returns the maximum allowed number of resubmissions */
	std::size_t getMaxResubmissions() const;

	/** @brief Checks whether a given algorithm type likes to communicate via the broker */
	virtual bool usesBroker() const;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
		, const bool& showOrigin
	);

protected:
	/** @brief Loads the data of another population */
	virtual void load_(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone_() const;

	/** @brief Performs necessary initialization work */
	virtual void init();
	/** @brief Does any necessary finalization work */
	virtual void finalize();

	/** @brief Triggers fitness calculation of a number of individuals */
	virtual double doFitnessCalculation(const std::size_t&);

private:
    /*********************************************************************************/
    /**
     * A simple comparison operator that helps to sort individuals according to their
     * position in the population Smaller position numbers will end up in front.
     */
    struct indPositionComp {
    	bool operator()(boost::shared_ptr<GParameterSet> x, boost::shared_ptr<GParameterSet> y) {
    		return x->getPersonalityTraits<GGDPersonalityTraits>()->getPopulationPosition()
    			   < y->getPersonalityTraits<GGDPersonalityTraits>()->getPopulationPosition();
    	}
    };

    /*********************************************************************************/

    std::vector<bool> sm_value_; ///< Internal storage for server mode flags
    std::size_t maxResubmissions_; ///< The maximum number of allowed re-submissions in an iteration

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

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/** @brief We need to provide a specialization of the factory function that creates objects of this type. */
template <> boost::shared_ptr<Gem::Geneva::GBrokerGD> TFactory_GUnitTests<Gem::Geneva::GBrokerGD>();

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBrokerGD)

#endif /* GBROKERGD_HPP_ */
