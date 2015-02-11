/**
 * @file GLineFitIndividual.hpp
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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm> // for std::sort

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>

#ifndef GLINEFITINDIVIDUAL_HPP_
#define GLINEFITINDIVIDUAL_HPP_

// Geneva header files go here
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GEAPersonalityTraits.hpp"
#include "geneva/GGDPersonalityTraits.hpp"
#include "geneva/GSwarmPersonalityTraits.hpp"
#include "common/GExceptions.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GFactoryT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This individual takes a vector of 2D double-tuples and calculates the
 * root-square deviation from the line defined by its two parameters
 */
class G_API GLineFitIndividual
   :public Gem::Geneva::GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		& BOOST_SERIALIZATION_NVP(dataPoints_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GLineFitIndividual(const std::vector<boost::tuple<double, double> >&);
	/** @brief The copy constructor */
	GLineFitIndividual(const GLineFitIndividual&);
	/** @brief The standard destructor */
	virtual ~GLineFitIndividual();

	/** @brief A standard assignment operator */
	const GLineFitIndividual& operator=(const GLineFitIndividual&);

	/** @brief Checks for equality with another GLineFitIndividual object */
	bool operator==(const GLineFitIndividual& cp) const;
	/** @brief Checks for inequality with another GLineFitIndividual object */
	bool operator!=(const GLineFitIndividual& cp) const;

	/** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&,
			const Gem::Common::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&
	) const;

	/** @brief Retrieves the tuple (a,b) of the line represented by this object */
	boost::tuple<double, double> getLine() const;

protected:
	/** @brief Loads the data of another GLineFitIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here. */
	virtual double fitnessCalculation() OVERRIDE;


private:
	/** @brief The default constructor -- private, as it is only needed for (de-)serialization purposes */
	GLineFitIndividual();

	std::vector<boost::tuple<double, double> > dataPoints_; ///< Holds the data points used for the fit procedure

#ifdef GEM_TESTING
public:
	/** @brief Applies modifications to this object. */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. */
	virtual void specificTestsFailuresExpected_GUnitTests();

#endif /* GEM_TESTING */
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GLineFitIndividual objects
 */
class GLineFitIndividualFactory
   : public Gem::Common::GFactoryT<GParameterSet>
{
public:
   /** @brief The standard constructor */
   GLineFitIndividualFactory(
      const std::vector<boost::tuple<double, double> >&
      , const std::string&
   );
   /** @brief The destructor */
   virtual ~GLineFitIndividualFactory();

protected:
   /** @brief Creates individuals of this type */
   virtual boost::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
   /** @brief Allows to describe local configuration options in derived classes */
   virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
   /** @brief Allows to act on the configuration options received from the configuration file */
   virtual void postProcess_(boost::shared_ptr<GParameterSet>&);

private:
   /** @brief The default constructor. Intentionally private and undefined */
   GLineFitIndividualFactory();

   std::vector<boost::tuple<double, double> > dataPoints_; ///< Holds data points for the fit
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GLineFitIndividual)

#endif /* GLINEFITINDIVIDUAL_HPP_ */
