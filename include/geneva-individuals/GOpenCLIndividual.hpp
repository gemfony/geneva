/**
 * @file GOpenCLIndividual.hpp
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
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GOPENCLINDIVIDUAL_HPP_
#define GOPENCLINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include <geneva/GConstrainedDoubleObjectCollection.hpp>
#include <geneva/GDoubleGaussAdaptor.hpp>
#include <geneva/GParameterSet.hpp>

namespace Gem
{
namespace Geneva
{

/************************************************************************************************/
// Some defaults used for this individual
const std::string DEFAULTOPENCLTASK = "./openCLTask.cl"

/************************************************************************************************/
/**
 * This individual executes the evaluation step on the graphics card, using user-supplied
 * OpenCL code. This is a preliminary version, meant as a proof of concept for letting Geneva
 * use the graphics hardware. Later versions will abstract more details away, this class can
 * then become the base class for a user-supplied hierarchy of classes.
 */
class GOpenCLIndividual	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(openCLTask_);
	}

	///////////////////////////////////////////////////////////////////////


public:
	/** @brief The default constructor */
	GOpenCLIndividual();
	/** @brief Initialization with the name of the OpenCL file */
	GOpenCLIndividual(const std::string&);
	/** @brief The copy constructor */
	GOpenCLIndividual(const GOpenCLIndividual&);
	/** @brief The destructor */
	~GOpenCLIndividual();

	/** @brief A standard assignment operator */
	const GOpenCLIndividual& operator=(const GOpenCLIndividual&);

	/** @brief Checks for equality with another GOpenCLIndividual object */
	bool operator==(const GOpenCLIndividual& cp) const;
	/** @brief Checks for inequality with another GOpenCLIndividual object */
	bool operator!=(const GOpenCLIndividual& cp) const;

	/** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled. */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&,
			const Gem::Common::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&) const;

	/** @brief Allows to set a new OpenCL file */
	void setOpenCLTaskFile(const std::string&);
	/** @brief Allows to retrieve the name of the file currently used for OpenCL calculations */
	std::string getOpenCLTaskFile() const;

	/********************************************************************************************/
	/**
	 * Necessary initialization work for OpenCL and this individual in general
	 */
	static void init() {
		// nothing yet
	}

protected:
	/********************************************************************************************/
	/** @brief Loads the data of another GOpenCLIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation();

private:
	/********************************************************************************************/
	std::string openCLTask_; ///< Holds the name of the file with the evaluation program
};

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPENCLINDIVIDUAL_HPP_ */
