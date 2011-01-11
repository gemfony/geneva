/**
 * @file GParaboloidIndividual2D.hpp
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


// Standard header files go here
#include <iostream>

// Includes check for correct Boost version(s)
#include <common/GGlobalDefines.hpp>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GPARABOLOIDINDIVIDUAL2D_HPP_
#define GPARABOLOIDINDIVIDUAL2D_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include <geneva/GParameterSet.hpp>

namespace Gem
{
namespace Geneva
{
/******************************************************************/
/**
 * This individual searches for the minimum of a 2-dimensional parabola.
 * It is part of an introductory example, used in the Geneva manual.
 */
class GParaboloidIndividual2D :public GParameterSet
{
public:
	/** @brief The default constructor */
	GParaboloidIndividual2D();
	/** @brief A standard copy constructor */
	GParaboloidIndividual2D(const GParaboloidIndividual2D&);
	/** @brief The standard destructor */
	virtual ~GParaboloidIndividual2D();

	/** @brief A standard assignment operator */
	const GParaboloidIndividual2D& operator=(const GParaboloidIndividual2D&);

protected:
	/** @brief Loads the data of another GParaboloidIndividual2D */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here. */
	virtual double fitnessCalculation();

private:
	/** @brief Make the class accessible to Boost.Serialization */
	friend class boost::serialization::access;

	/**************************************************************/
	/**
	 * This function triggers serialization of this class and its
	 * base classes.
	 */
	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	}
	/**************************************************************/

	const double PAR_MIN_;
	const double PAR_MAX_;
};

/******************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParaboloidIndividual2D)

#endif /* GPARABOLOIDINDIVIDUAL2D_HPP_ */
