/**
 * @file GSwarmPersonalityTraits.hpp
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

#ifndef GSWARMPERSONALITYTRAITS_HPP_
#define GSWARMPERSONALITYTRAITS_HPP_

// Geneva headers go here
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to swarm optimization.
 */
class GSwarmPersonalityTraits :public GPersonalityTraits
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	G_API_GENEVA void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar
	  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits)
	  & BOOST_SERIALIZATION_NVP(neighborhood_)
	  & BOOST_SERIALIZATION_NVP(noPositionUpdate_)
	  & BOOST_SERIALIZATION_NVP(personal_best_)
	  & BOOST_SERIALIZATION_NVP(personal_best_quality_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API_GENEVA GSwarmPersonalityTraits();
	/** @brief The copy contructor */
	G_API_GENEVA GSwarmPersonalityTraits(const GSwarmPersonalityTraits&);
	/** @brief The standard destructor */
	virtual G_API_GENEVA ~GSwarmPersonalityTraits();

	/** @brief A standard assignment operator */
	G_API_GENEVA const GSwarmPersonalityTraits& operator=(const GSwarmPersonalityTraits&);

	/** @brief Checks for equality with another GSwarmPersonalityTraits object */
	G_API_GENEVA bool operator==(const GSwarmPersonalityTraits&) const;
	/** @brief Checks for inequality with another GSwarmPersonalityTraits object */
	G_API_GENEVA bool operator!=(const GSwarmPersonalityTraits&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API_GENEVA boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	/** @brief Specifies in which neighborhood the individual is at present */
	G_API_GENEVA void setNeighborhood(const std::size_t&) ;
	/** @brief Retrieves the id of the neighborhood the individual is in at present */
	G_API_GENEVA std::size_t getNeighborhood(void) const;

	/** @brief Sets the noPositionUpdate_ flag */
	G_API_GENEVA void setNoPositionUpdate();
	/** @brief Retrieves the current value of the noPositionUpdate_ flag */
	G_API_GENEVA bool noPositionUpdate() const;
	/** @brief Retrieves and resets the current value of the noPositionUpdate_ flag */
	G_API_GENEVA bool checkNoPositionUpdateAndReset();

	/** @brief Allows to add a new personal best to the individual */
	G_API_GENEVA void registerPersonalBest(boost::shared_ptr<GParameterSet>);
	/** @brief Allows to retrieve the personal best individual */
	G_API_GENEVA boost::shared_ptr<GParameterSet> getPersonalBest() const;
	/** @brief Resets the personal best individual */
	G_API_GENEVA void resetPersonalBest();
	/** @brief Retrieve quality of personally best individual */
	G_API_GENEVA boost::tuple<double, double> getPersonalBestQuality() const;

   /** @brief Emits a name for this class / object */
   virtual G_API_GENEVA std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GSwarmPersonalityTraits object */
	virtual G_API_GENEVA void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object */
	virtual G_API_GENEVA GObject* clone_() const OVERRIDE;

private:
	/** @brief Stores the current position in the population */
	std::size_t neighborhood_;

	/** @brief Determines whether the individual has been randomly initialized */
	bool noPositionUpdate_;

	/** @brief Holds the personally best GParameterSet */
	boost::shared_ptr<GParameterSet> personal_best_;
	/** @brief The quality of the personally best individual */
	boost::tuple<double, double> personal_best_quality_;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API_GENEVA bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API_GENEVA void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSwarmPersonalityTraits)

#endif /* GSWARMPERSONALITYTRAITS_HPP_ */

