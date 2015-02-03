/**
 * @file GMultiCriterionParabolaIndividual.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard header files go here
#include <iostream>

// Includes check for correct Boost version(s)
#include <common/GGlobalDefines.hpp>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GPARABOLOIDINDIVIDUAL_HPP_
#define GPARABOLOIDINDIVIDUAL_HPP_

// Geneva header files go here
#include <geneva/GParameterSet.hpp>
#include <geneva/GConstrainedDoubleObject.hpp>
#include <common/GFactoryT.hpp>
#include <common/GParserBuilder.hpp>
#include <common/GHelperFunctions.hpp>

namespace Gem {
namespace Geneva {

// The number of parameters
const std::size_t NPAR_MC = 3;

/******************************************************************************/
/**
 * This individual implements several, possibly conflicting evaluation
 * criteria, each implemented as a parabola with its own minimum
 */
class GMultiCriterionParabolaIndividual :public GParameterSet
{
   friend class GMultiCriterionParabolaIndividualFactory;

	/***************************************************************************/
	/**
	 * This function triggers serialization of this class and its
	 * base classes.
	 */
	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	}

   /** @brief Make the class accessible to Boost.Serialization */
   friend class boost::serialization::access;
   /***************************************************************************/

public:
	/** @brief The standard constructor */
	GMultiCriterionParabolaIndividual(const std::size_t&);
	/** @brief A standard copy constructor */
	GMultiCriterionParabolaIndividual(const GMultiCriterionParabolaIndividual&);
	/** @brief The destructor */
	virtual ~GMultiCriterionParabolaIndividual();

	/** @brief A standard assignment operator */
	const GMultiCriterionParabolaIndividual& operator=(const GMultiCriterionParabolaIndividual&);

	/** @brief Assigns a number of minima to this object */
	void setMinima(const std::vector<double>&);

protected:
	/** @brief Loads the data of another GMultiCriterionParabolaIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here. */
	virtual double fitnessCalculation() OVERRIDE;

private:
   /** @brief The default constructor -- intentionally private*/
   GMultiCriterionParabolaIndividual();
   /** @brief Holds the minima needed for multi-criterion optimization */
   std::vector<double> minima_;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GMultiCriterionParabolaIndividual objects
 */
class GMultiCriterionParabolaIndividualFactory
	:public Gem::Common::GFactoryT<GParameterSet>
{
public:
	/** @brief The standard constructor for this class */
	GMultiCriterionParabolaIndividualFactory(const std::string&);
	/** @brief The destructor */
	virtual ~GMultiCriterionParabolaIndividualFactory();

protected:
   /** @brief Creates individuals of this type */
   virtual boost::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
   /** @brief Allows to describe local configuration options in derived classes */
   virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
   /** @brief Allows to act on the configuration options received from the configuration file */
   virtual void postProcess_(boost::shared_ptr<GParameterSet>&);

private:
   Gem::Common::GOneTimeRefParameterT<double> par_min_; ///< The lower boundary of the initialization range
   Gem::Common::GOneTimeRefParameterT<double> par_max_; ///< The upper boundary of the initialization range
   Gem::Common::GOneTimeRefParameterT<std::string> minima_string_; ///< The minima encoded as a string

   std::vector<double> minima_; ///< The desired minima of the parabolas
   std::size_t nPar_; ///< The number of parameters to be added to the individual
   bool firstParsed_; ///< Set to false when the configuration files were parsed for the first time
};

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content
 */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::GMultiCriterionParabolaIndividual&);
std::ostream& operator<<(std::ostream&, boost::shared_ptr<Gem::Geneva::GMultiCriterionParabolaIndividual>);

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GMultiCriterionParabolaIndividual)

#endif /* GPARABOLOIDINDIVIDUAL_HPP_ */
