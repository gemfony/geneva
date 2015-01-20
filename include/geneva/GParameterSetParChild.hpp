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

// Standard headers go here

// Boost headers go here
#include <boost/tuple/tuple.hpp>

#ifndef GPARAMETERSETPARCHILD_HPP_
#define GPARAMETERSETPARCHILD_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

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
      & make_nvp("GBaseParChildT_GParameterSet", boost::serialization::base_object<GBaseParChildT<GParameterSet> >(*this))
      & BOOST_SERIALIZATION_NVP(amalgamationLikelihood_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GParameterSetParChild();
   /** @brief A standard copy constructor */
   GParameterSetParChild(const GParameterSetParChild&);
   /** @brief The destructor */
   virtual ~GParameterSetParChild();

   /** @brief A standard assignment operator */
   const GParameterSetParChild& operator=(const GParameterSetParChild&);

   /** @brief Checks for equality with another GParameterSetParChild object */
   bool operator==(const GParameterSetParChild&) const;
   /** @brief Checks for inequality with another GParameterSetParChild object */
   bool operator!=(const GParameterSetParChild&) const;

   /** @brief Checks whether this object fulfills a given expectation in relation to another object */
   virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
   ) const OVERRIDE;

   /** @brief Adds local configuration options to a GParserBuilder object */
   virtual void addConfigurationOptions (
      Gem::Common::GParserBuilder& gpb
   ) OVERRIDE;

   /** @brief Allows to set the likelihood for amalgamation of two units to be performed instead of "just" duplication. */
   void setAmalgamationLikelihood(double);
   /** @brief Allows to retrieve the likelihood for amalgamation of two units to be performed instead of "just" duplication. */
   double getAmalgamationLikelihood() const;

protected:
   /***************************************************************************/
   /** @brief Performs recombination, taking into account possible amalgamation actions */
   virtual void doRecombine();
   /** @brief Marks the number of stalled optimization attempts in all individuals and gives them an opportunity to update their internal structures. */
   virtual void actOnStalls() OVERRIDE;

   /** @brief Does some preparatory work before the optimization starts */
   virtual void init() OVERRIDE;
   /** @brief Does any necessary finalization work */
   virtual void finalize() OVERRIDE;

   /** @brief Loads the data of another population */
   virtual void load_(const GObject *) OVERRIDE;

   double amalgamationLikelihood_; ///< Likelihood for children to be created by cross-over rather than "just" duplication (note that they may nevertheless be mutated)

public:
   /***************************************************************************/
   /** @brief Applies modifications to this object. This is needed for testing purposes */
   virtual bool modify_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
   virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
   /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
   virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GParameterSetParChild)

#endif /* GPARAMETERSETPARCHILD_HPP_ */
