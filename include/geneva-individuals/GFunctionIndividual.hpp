/**
 * @file GFunctionIndividual.hpp
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

#ifndef GFUNCTIONINDIVIDUAL_HPP_
#define GFUNCTIONINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem
{
namespace Geneva
{

/******************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum solverFunction {
	  PARABOLA=0
	, NOISYPARABOLA=1
	, ROSENBROCK=2
	, ACKLEY=3
	, RASTRIGIN=4
	, SCHWEFEL=5
	, SALOMON=6
};

const solverFunction MAXDEMOFUNCTION=SALOMON;

// Make sure solverFunction can be streamed
/** @brief Puts a Gem::Geneva::solverFunction into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::solverFunction&);

/** @brief Reads a Gem::Geneva::solverFunction from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::solverFunction&);

/**
 * This enum describes different parameter types that may be used to fill the object with data
 */
enum parameterType {
	USEGDOUBLECOLLECTION = 0
	, USEGCONSTRAINEDOUBLECOLLECTION = 1
	, USEGDOUBLEOBJECTCOLLECTION = 2
	, USEGCONSTRAINEDDOUBLEOBJECTCOLLECTION = 3
};

// Make sure parameterType can be streamed
/** @brief Puts a Gem::Geneva::parameterType into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::parameterType&);

/** @brief Reads a Gem::Geneva::parameterType from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::parameterType&);

/**
 * This enum describes several ways of initializing the data collections
 */
enum initMode {
	INITRANDOM = 0 // random values for all variables
	, INITPERIMETER = 1 // Uses a parameter set on the perimeter of the allowed or common value range
};

// Make sure initMode can be streamed
/** @brief Puts a Gem::Geneva::initMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::initMode&);

/** @brief Reads a Gem::Geneva::initMode from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::initMode&);

/******************************************************************************/
// A number of default settings for the factory
const double GFI_DEF_ADPROB = 1.0;
const boost::uint32_t GFI_DEF_ADAPTIONTHRESHOLD = 1;
const bool GFI_DEF_USEBIGAUSSIAN = false;
const double GFI_DEF_SIGMA1 = 0.5;
const double GFI_DEF_SIGMASIGMA1 = 0.8;
const double GFI_DEF_MINSIGMA1 = 0.001;
const double GFI_DEF_MAXSIGMA1 = 2;
const double GFI_DEF_SIGMA2 = 0.5;
const double GFI_DEF_SIGMASIGMA2 = 0.8;
const double GFI_DEF_MINSIGMA2 = 0.001;
const double GFI_DEF_MAXSIGMA2 = 2;
const double GFI_DEF_DELTA = 0.5;
const double GFI_DEF_SIGMADELTA = 0.8;
const double GFI_DEF_MINDELTA = 0.001;
const double GFI_DEF_MAXDELTA = 2.;
const std::size_t GFI_DEF_PARDIM = 2;
const double GFI_DEF_MINVAR = -10.;
const double GFI_DEF_MAXVAR = 10.;
const bool GFI_DEF_USECONSTRAINEDDOUBLECOLLECTION = false;
const parameterType GFI_DEF_PARAMETERTYPE = USEGDOUBLECOLLECTION;
const initMode GFI_DEF_INITMODE = INITRANDOM;
const solverFunction GO_DEF_EVALFUNCTION = boost::numeric_cast<solverFunction>(0);

/******************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GFunctionIndividual
	: public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(demoFunction_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GFunctionIndividual();
	/** @brief Initialization with the desired demo function */
	GFunctionIndividual(const solverFunction&);
	/** @brief A standard copy constructor */
	GFunctionIndividual(const GFunctionIndividual&);
	/** @brief The standard destructor */
	~GFunctionIndividual();

	/** @brief A standard assignment operator */
	const GFunctionIndividual& operator=(const GFunctionIndividual&);

	/** @brief Checks for equality with another GFunctionIndividual object */
	bool operator==(const GFunctionIndividual&) const;
	/** @brief Checks for inequality with another GFunctionIndividual object */
	bool operator!=(const GFunctionIndividual& cp) const;

	/** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled. */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&,
			const Gem::Common::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&) const;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

	/** @brief Allows to set the demo function */
	void setDemoFunction(solverFunction);
	/** @brief Allows to retrieve the current demo function */
	solverFunction getDemoFunction() const;

	/** @brief Allows to cross check the parameter size */
	std::size_t getParameterSize() const;

	/***************************************************************************/
	/**
	 * This function converts the function id to a string representation. This is a convenience
	 * function that is mostly used in GArgumentParser.cpp of various Geneva examples.
	 *
	 * @param df The id of the desired function individual
	 * @return A string representing the name of the current function
	 */
	static std::string getStringRepresentation(const solverFunction& df) {
		std::string result;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			result="Parabola";
			break;
		case NOISYPARABOLA:
			result="Berlich noisy parabola";
			break;
		case ROSENBROCK:
			result="Rosenbrock";
			break;
		case ACKLEY:
			result="Ackley";
			break;
		case RASTRIGIN:
			result="Rastrigin";
			break;
		case SCHWEFEL:
			result="Schwefel";
			break;
		case SALOMON:
			result="Salomon";
			break;
		}

		return result;
	}

	/***************************************************************************/
	/**
	 * Retrieves a string in ROOT format (see http://root.cern.ch) of the 2D version of a
	 * given function.
	 *
	 * @param df The id of the desired function individual
	 * @return A string suitable for plotting a 2D version of this function with the ROOT analysis framework
	 */
	static std::string get2DROOTFunction(const solverFunction& df) {
		std::string result;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			result="x^2 + y^2";
			break;
		case NOISYPARABOLA:
			result="(cos(x^2 + y^2) + 2.) * (x^2 + y^2)";
			break;
		case ROSENBROCK:
			result="100.*(x^2 - y)^2 + (1 - x)^2";
			break;
		case ACKLEY:
			result="exp(-0.2)*sqrt(x^2 + y^2) + 3.*(cos(2.*x) + sin(2.*y))";
			break;
		case RASTRIGIN:
			result="20.+(x^2 - 10.*cos(2*pi*x)) + (y^2 - 10.*cos(2*pi*y))";
			break;
		case SCHWEFEL:
			result="-0.5*(x*sin(sqrt(abs(x))) + y*sin(sqrt(abs(y))))";
			break;
		case SALOMON:
			result="-cos(2.*pi*sqrt(x^2 + y^2)) + 0.1*sqrt(x^2 + y^2) + 1.";
			break;
		}

		return result;
	}

	/***************************************************************************/
	/**
	 * Retrieves the minimum x-value(s) of a given (2D) demo function
	 *
	 * @param df The id of the desired function individual
	 * @return The x-coordinate(s) of the global optimium in 2D
	 */
	static std::vector<double> getXMin(const solverFunction& df) {
		std::vector<double> result;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			result.push_back(0.);
			break;
		case NOISYPARABOLA:
			result.push_back(0.);
			break;
		case ROSENBROCK:
			result.push_back(1.);
			break;
		case ACKLEY:
			// two global optima
			result.push_back(-1.5096201);
			result.push_back( 1.5096201);
			break;
		case RASTRIGIN:
			result.push_back(0.);
			break;
		case SCHWEFEL:
			result.push_back(420.968746);
			break;
		case SALOMON:
			result.push_back(0.);
			break;
		}

		return result;
	}

	/***************************************************************************/
	/**
	 * Retrieves the minimum y-value(s) of a given (2D) demo function
	 *
	 * @param df The id of the desired function individual
	 * @return The y-coordinate(s) of the global optimium in 2D
	 */
	static std::vector<double> getYMin(const solverFunction& df) {
		std::vector<double> result;

		// Set up a single function individual, depending on the expected function type
		switch(df) {
		case PARABOLA:
			result.push_back(0.);
			break;
		case NOISYPARABOLA:
			result.push_back(0.);
			break;
		case ROSENBROCK:
			result.push_back(1.);
			break;
		case ACKLEY:
			result.push_back(-0.7548651);
			break;
		case RASTRIGIN:
			result.push_back(0.);
			break;
		case SCHWEFEL:
			result.push_back(420.968746);
			break;
		case SALOMON:
			result.push_back(0.);
			break;
		}

		return result;
	}

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GFunctionIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual value calculation takes place here */
	virtual double fitnessCalculation();

	/***************************************************************************/

private:
	solverFunction demoFunction_; ///< Specifies which demo function should be used
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFunctionIndividual objects
 */
class GFunctionIndividualFactory
	: public Gem::Common::GFactoryT<GFunctionIndividual>
{
public:
	/** @brief The standard constructor */
	GFunctionIndividualFactory(const std::string&);
	/** @brief The destructor */
	virtual ~GFunctionIndividualFactory();

	/** @brief (Re-)Set the dimension of the function */
	void setParDim(std::size_t);
	/** @brief Extract the minimum and maximum boundaries of the variables */
	boost::tuple<double,double> getVarBoundaries() const;

protected:
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<GFunctionIndividual> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	/** @brief Allows to describe local configuration options in derived classes */
	virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<GFunctionIndividual>&);

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GFunctionIndividualFactory();

	double adProb_;
	boost::uint32_t adaptionThreshold_;
	bool useBiGaussian_;
	double sigma1_;
	double sigmaSigma1_;
	double minSigma1_;
	double maxSigma1_;
	double sigma2_;
	double sigmaSigma2_;
	double minSigma2_;
	double maxSigma2_;
	double delta_;
	double sigmaDelta_;
	double minDelta_;
	double maxDelta_;
	std::size_t parDim_;
	std::size_t parDimLocal_;
	double minVar_;
	double maxVar_;
	parameterType pT_;
	initMode iM_;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFunctionIndividual)

#endif /* GFUNCTIONINDIVIDUAL_HPP_ */
