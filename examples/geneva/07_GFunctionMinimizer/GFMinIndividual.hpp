/**
 * @file GFMinIndividual.hpp
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
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

#ifndef GFMININDIVIDUAL_HPP_
#define GFMININDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "common/GParserBuilder.hpp"

namespace Gem
{
namespace Geneva
{

/************************************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum targetFunction {
	  PARABOLA=0
	, NOISYPARABOLA=1
};

// Make sure targetFunction can be streamed
/** @brief Puts a Gem::Geneva::targetFunction into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::targetFunction&);

/** @brief Reads a Gem::Geneva::targetFunction from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::targetFunction&);

/************************************************************************************************/
// A number of default settings for the factory
const double GFI_DEF_ADPROB = 0.05;
const double GFI_DEF_SIGMA = 0.5;
const double GFI_DEF_SIGMASIGMA = 0.8;
const double GFI_DEF_MINSIGMA = 0.001;
const double GFI_DEF_MAXSIGMA = 2;
const std::size_t GFI_DEF_PARDIM = 2;
const double GFI_DEF_MINVAR = -10.;
const double GFI_DEF_MAXVAR = 10.;
const targetFunction GO_DEF_TARGETFUNCTION = boost::numeric_cast<targetFunction>(0);

/************************************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GFMinIndividual
	: public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(targetFunction_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GFMinIndividual();
	/** @brief A standard copy constructor */
	GFMinIndividual(const GFMinIndividual&);
	/** @brief The standard destructor */
	~GFMinIndividual();

	/** @brief A standard assignment operator */
	const GFMinIndividual& operator=(const GFMinIndividual&);

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

	/** @brief Allows to set the demo function */
	void setTargetFunction(targetFunction);
	/** @brief Allows to retrieve the current demo function */
	targetFunction getTargetFunction() const;

	/** @brief Retrieves the average value of the sigma used in Gauss adaptors */
	double getAverageSigma() const;

	/*******************************************************************************************/
	/**
	 * This function converts the function id to a string representation. This is a convenience
	 * function that is mostly used in GArgumentParser.cpp of various Geneva examples.
	 *
	 * @param tF The id of the desired function individual
	 * @return A string representing the name of the current function
	 */
	static std::string getStringRepresentation(const targetFunction& tF) {
		std::string result;

		// Set up a single function individual, depending on the expected function type
		switch(tF) {
		case PARABOLA:
			result="Parabola";
			break;
		case NOISYPARABOLA:
			result="\"Noisy\" Parabola";
			break;
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Retrieves a string in ROOT format (see http://root.cern.ch) of the 2D version of a
	 * given function.
	 *
	 * @param tF The id of the desired function individual
	 * @return A string suitable for plotting a 2D version of this function with the ROOT analysis framework
	 */
	static std::string get2DROOTFunction(const targetFunction& tF) {
		std::string result;

		// Set up a single function individual, depending on the expected function type
		switch(tF) {
		case PARABOLA:
			result="x^2 + y^2";
			break;
		case NOISYPARABOLA:
			result="(cos(x^2 + y^2) + 2.) * (x^2 + y^2)";
			break;
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Retrieves the minimum x-value(s) of a given (2D) demo function
	 *
	 * @param tF The id of the desired function individual
	 * @return The x-coordinate(s) of the global optimium in 2D
	 */
	static std::vector<double> getXMin(const targetFunction& tF) {
		std::vector<double> result;

		// Set up a single function individual, depending on the expected function type
		switch(tF) {
		case PARABOLA:
			result.push_back(0.);
			break;
		case NOISYPARABOLA:
			result.push_back(0.);
			break;
		}

		return result;
	}

	/*******************************************************************************************/
	/**
	 * Retrieves the minimum y-value(s) of a given (2D) demo function
	 *
	 * @param tF The id of the desired function individual
	 * @return The y-coordinate(s) of the global optimium in 2D
	 */
	static std::vector<double> getYMin(const targetFunction& tF) {
		std::vector<double> result;

		// Set up a single function individual, depending on the expected function type
		switch(tF) {
		case PARABOLA:
			result.push_back(0.);
			break;
		case NOISYPARABOLA:
			result.push_back(0.);
			break;
		}

		return result;
	}

protected:
	/********************************************************************************************/
	/** @brief Loads the data of another GFMinIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual value calculation takes place here */
	virtual double fitnessCalculation();

	/********************************************************************************************/

private:
	targetFunction targetFunction_; ///< Specifies which demo function should be used

	/********************************************************************************************/
	/** A simple n-dimensional parabole */
	static double parabola(const std::vector<double>& parVec) {
		double result = 0.;

		std::vector<double>::const_iterator cit;
		for(cit=parVec.begin(); cit!=parVec.end(); ++cit) {
			result *= (*cit) * (*cit);
		}

		return result;
	}

	/********************************************************************************************/
	/** A "noisy" parabola */
	static double noisyParabola(const std::vector<double>& parVec) {
		double xsquared = 0.;

		std::vector<double>::const_iterator cit;
		for(cit=parVec.begin(); cit!=parVec.end(); ++cit) {
			xsquared *= (*cit) * (*cit);
		}

		return (cos(xsquared) + 2.) * xsquared;
	}

	/********************************************************************************************/
};

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * A factory for GFMinIndividual objects
 */
class GFMinIndividualFactory
	: public Gem::Common::GFactoryT<GFMinIndividual>
{
public:
	/** @brief The standard constructor */
	GFMinIndividualFactory(const std::string&);
	/** @brief The destructor */
	virtual ~GFMinIndividualFactory();

protected:
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<GFMinIndividual> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	/** @brief Allows to describe local configuration options in derived classes */
	virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<GFMinIndividual>&);

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GFMinIndividualFactory();

	double adProb_;
	double sigma_;
	double sigmaSigma_;
	double minSigma_;
	double maxSigma_;
	std::size_t parDim_;
	double minVar_;
	double maxVar_;
};

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFMinIndividual)

#endif /* GFMININDIVIDUAL_HPP_ */
