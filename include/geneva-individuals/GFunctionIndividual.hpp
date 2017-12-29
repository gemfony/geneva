/**
 * @file GFunctionIndividual.hpp
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

#ifndef GFUNCTIONINDIVIDUAL_HPP_
#define GFUNCTIONINDIVIDUAL_HPP_

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <tuple>
#include <type_traits>

// Boost header files go here
#include <boost/math/constants/constants.hpp>

// Geneva header files go here
#include "common/GParserBuilder.hpp"
#include "common/GCommonMathHelperFunctions.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GParameterSetMultiConstraint.hpp"
#include "geneva/GParameterSetFactory.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum class solverFunction : Gem::Common::ENUMBASETYPE {
	 PARABOLA = 0,
	 NOISYPARABOLA = 1,
	 ROSENBROCK = 2,
	 ACKLEY = 3,
	 RASTRIGIN = 4,
	 SCHWEFEL = 5,
	 SALOMON = 6,
	 NEGPARABOLA = 7
};

const solverFunction MAXDEMOFUNCTION = solverFunction::NEGPARABOLA;

// Make sure solverFunction can be streamed
/** @brief Puts a Gem::Geneva::solverFunction into a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::ostream &operator<<(std::ostream &, const Gem::Geneva::solverFunction &);

/** @brief Reads a Gem::Geneva::solverFunction from a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::istream &operator>>(std::istream &, Gem::Geneva::solverFunction &);

/**
 * This enum describes different parameter types that may be used to fill the object with data
 */
enum class parameterType : Gem::Common::ENUMBASETYPE {
	 USEGDOUBLECOLLECTION = 0,
	 USEGCONSTRAINEDOUBLECOLLECTION = 1,
	 USEGDOUBLEOBJECTCOLLECTION = 2,
	 USEGCONSTRAINEDDOUBLEOBJECTCOLLECTION = 3,
	 USEGCONSTRAINEDDOUBLEOBJECT = 4
};

// Make sure parameterType can be streamed
/** @brief Puts a Gem::Geneva::parameterType into a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::ostream &operator<<(std::ostream &, const Gem::Geneva::parameterType &);

/** @brief Reads a Gem::Geneva::parameterType from a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::istream &operator>>(std::istream &, Gem::Geneva::parameterType &);

/**
 * This enum describes several ways of initializing the data collections
 */
enum class initMode : Gem::Common::ENUMBASETYPE {
	 INITRANDOM = 0 // random values for all variables
	 , INITPERIMETER = 1 // Uses a parameter set on the perimeter of the allowed or common value range
};

// Make sure initMode can be streamed
/** @brief Puts a Gem::Geneva::initMode into a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::ostream &operator<<(std::ostream &, const Gem::Geneva::initMode &);

/** @brief Reads a Gem::Geneva::initMode from a stream. Needed also for boost::lexical_cast<> */
G_API_INDIVIDUALS std::istream &operator>>(std::istream &, Gem::Geneva::initMode &);

/******************************************************************************/
// A number of default settings for the factory
const double GFI_DEF_ADPROB = 1.0;
const double GFI_DEF_ADAPTADPROB = 0.1;
const double GFI_DEF_MINADPROB = 0.05;
const double GFI_DEF_MAXADPROB = 1.;
const std::uint32_t GFI_DEF_ADAPTIONTHRESHOLD = 1;
const bool GFI_DEF_USEBIGAUSSIAN = false;
const double GFI_DEF_SIGMA1 = 0.025;
const double GFI_DEF_SIGMASIGMA1 = 0.2;
const double GFI_DEF_MINSIGMA1 = 0.001;
const double GFI_DEF_MAXSIGMA1 = 1;
const double GFI_DEF_SIGMA2 = 0.025;
const double GFI_DEF_SIGMASIGMA2 = 0.2;
const double GFI_DEF_MINSIGMA2 = 0.001;
const double GFI_DEF_MAXSIGMA2 = 1;
const double GFI_DEF_DELTA = 0.05;
const double GFI_DEF_SIGMADELTA = 0.2;
const double GFI_DEF_MINDELTA = 0.001;
const double GFI_DEF_MAXDELTA = 1.;
const std::size_t GFI_DEF_PARDIM = 2;
const double GFI_DEF_MINVAR = -10.;
const double GFI_DEF_MAXVAR = 10.;
const bool GFI_DEF_USECONSTRAINEDDOUBLECOLLECTION = false;
const parameterType GFI_DEF_PARAMETERTYPE = parameterType::USEGCONSTRAINEDDOUBLEOBJECT;
const initMode GFI_DEF_INITMODE = initMode::INITPERIMETER;
const solverFunction GO_DEF_EVALFUNCTION = solverFunction::PARABOLA;
const double GFI_DEF_CROSSOVERPROB = 0.5;

/******************************************************************************/
// Forward declaraion
class GFunctionIndividualFactory;

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
	 void serialize(Archive &ar, const unsigned int) {
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		 & BOOST_SERIALIZATION_NVP(demoFunction_);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 using FACTORYTYPE = GFunctionIndividualFactory;

	 /** @brief The default constructor */
	 G_API_INDIVIDUALS GFunctionIndividual() = default;
	 /** @brief Initialization with the desired demo function */
	 explicit G_API_INDIVIDUALS GFunctionIndividual(const solverFunction &);
	 /** @brief A standard copy constructor */
	 G_API_INDIVIDUALS GFunctionIndividual(const GFunctionIndividual& cp) = default;

	 /** @brief The standard destructor */
	 G_API_INDIVIDUALS ~GFunctionIndividual() override = default;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 G_API_INDIVIDUALS void compare(
		 const GObject & // the other object
		 , const Gem::Common::expectation & // the expectation for this object, e.g. equality
		 , const double & // the limit for allowed deviations of floating point types
	 ) const final;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 G_API_INDIVIDUALS void addConfigurationOptions(Gem::Common::GParserBuilder &) override;

	 /** @brief Allows to set the demo function */
	 G_API_INDIVIDUALS void setDemoFunction(solverFunction);
	 /** @brief Allows to retrieve the current demo function */
	 G_API_INDIVIDUALS solverFunction getDemoFunction() const;

	 /** @brief Allows to cross check the parameter size */
	 G_API_INDIVIDUALS std::size_t getParameterSize() const;

	 /***************************************************************************/
	 /**
	  * This function converts the function id to a string representation. This is a convenience
	  * function that is mostly used in GArgumentParser.cpp of various Geneva examples.
	  *
	  * @param df The id of the desired function individual
	  * @return A string representing the name of the current function
	  */
	 static G_API_INDIVIDUALS std::string getStringRepresentation(const solverFunction &df) {
		 std::string result;

		 // Set up a single function individual, depending on the expected function type
		 switch (df) {
			 case solverFunction::PARABOLA:
				 result = "Parabola";
				 break;
			 case solverFunction::NOISYPARABOLA:
				 result = "Berlich noisy parabola";
				 break;
			 case solverFunction::ROSENBROCK:
				 result = "Rosenbrock";
				 break;
			 case solverFunction::ACKLEY:
				 result = "Ackley";
				 break;
			 case solverFunction::RASTRIGIN:
				 result = "Rastrigin";
				 break;
			 case solverFunction::SCHWEFEL:
				 result = "Schwefel";
				 break;
			 case solverFunction::SALOMON:
				 result = "Salomon";
				 break;
			 case solverFunction::NEGPARABOLA:
				 result = "Negative parabola";
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
	 static G_API_INDIVIDUALS std::string get2DROOTFunction(const solverFunction &df) {
		 std::string result;

		 // Set up a single function individual, depending on the expected function type
		 switch (df) {
			 case solverFunction::PARABOLA:
				 result = "x^2 + y^2";
				 break;
			 case solverFunction::NOISYPARABOLA:
				 result = "(cos(x^2 + y^2) + 2.) * (x^2 + y^2)";
				 break;
			 case solverFunction::ROSENBROCK:
				 result = "100.*(x^2 - y)^2 + (1 - x)^2";
				 break;
			 case solverFunction::ACKLEY:
				 result = "exp(-0.2)*sqrt(x^2 + y^2) + 3.*(cos(2.*x) + sin(2.*y))";
				 break;
			 case solverFunction::RASTRIGIN:
				 result = "20.+(x^2 - 10.*cos(2*pi*x)) + (y^2 - 10.*cos(2*pi*y))";
				 break;
			 case solverFunction::SCHWEFEL:
				 result = "-0.5*(x*sin(sqrt(abs(x))) + y*sin(sqrt(abs(y))))";
				 break;
			 case solverFunction::SALOMON:
				 result = "-cos(2.*pi*sqrt(x^2 + y^2)) + 0.1*sqrt(x^2 + y^2) + 1.";
				 break;
			 case solverFunction::NEGPARABOLA:
				 result = "-(x^2 + y^2)";
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
	 static G_API_INDIVIDUALS std::vector<double> getXMin(const solverFunction &df) {
		 std::vector<double> result;

		 // Set up a single function individual, depending on the expected function type
		 switch (df) {
			 case solverFunction::PARABOLA:
				 result.push_back(0.);
				 break;
			 case solverFunction::NOISYPARABOLA:
				 result.push_back(0.);
				 break;
			 case solverFunction::ROSENBROCK:
				 result.push_back(1.);
				 break;
			 case solverFunction::ACKLEY:
				 // two global optima
				 result.push_back(-1.5096201);
				 result.push_back(1.5096201);
				 break;
			 case solverFunction::RASTRIGIN:
				 result.push_back(0.);
				 break;
			 case solverFunction::SCHWEFEL:
				 result.push_back(420.968746);
				 break;
			 case solverFunction::SALOMON:
				 result.push_back(0.);
				 break;
			 case solverFunction::NEGPARABOLA:
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
	 static G_API_INDIVIDUALS std::vector<double> getYMin(const solverFunction &df) {
		 std::vector<double> result;

		 // Set up a single function individual, depending on the expected function type
		 switch (df) {
			 case solverFunction::PARABOLA:
				 result.push_back(0.);
				 break;
			 case solverFunction::NOISYPARABOLA:
				 result.push_back(0.);
				 break;
			 case solverFunction::ROSENBROCK:
				 result.push_back(1.);
				 break;
			 case solverFunction::ACKLEY:
				 result.push_back(-0.7548651);
				 break;
			 case solverFunction::RASTRIGIN:
				 result.push_back(0.);
				 break;
			 case solverFunction::SCHWEFEL:
				 result.push_back(420.968746);
				 break;
			 case solverFunction::SALOMON:
				 result.push_back(0.);
				 break;
			 case solverFunction::NEGPARABOLA:
				 result.push_back(0.);
				 break;
		 }

		 return result;
	 }

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another GFunctionIndividual */
	 G_API_INDIVIDUALS void load_(const GObject *) final;

	 /** @brief The actual value calculation takes place here */
	 G_API_INDIVIDUALS double fitnessCalculation() final;

	 /***************************************************************************/

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 G_API_INDIVIDUALS GObject *clone_() const final;

	 solverFunction demoFunction_ = solverFunction::PARABOLA; ///< Specifies which demo function should be used
};

/******************************************************************************/
/**
 * Provide an easy way to print the individual's content
 */
G_API_INDIVIDUALS std::ostream &operator<<(std::ostream &, const Gem::Geneva::GFunctionIndividual &);

G_API_INDIVIDUALS std::ostream &operator<<(std::ostream &, std::shared_ptr <Gem::Geneva::GFunctionIndividual>);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFunctionIndividual objects
 */
class GFunctionIndividualFactory
	: public GParameterSetFactory
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<class Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSetFactory)
		 & BOOST_SERIALIZATION_NVP(adProb_)
		 & BOOST_SERIALIZATION_NVP(adaptAdProb_)
		 & BOOST_SERIALIZATION_NVP(minAdProb_)
		 & BOOST_SERIALIZATION_NVP(maxAdProb_)
		 & BOOST_SERIALIZATION_NVP(adaptionThreshold_)
		 & BOOST_SERIALIZATION_NVP(useBiGaussian_)
		 & BOOST_SERIALIZATION_NVP(sigma1_)
		 & BOOST_SERIALIZATION_NVP(sigmaSigma1_)
		 & BOOST_SERIALIZATION_NVP(minSigma1_)
		 & BOOST_SERIALIZATION_NVP(maxSigma1_)
		 & BOOST_SERIALIZATION_NVP(sigma2_)
		 & BOOST_SERIALIZATION_NVP(sigmaSigma2_)
		 & BOOST_SERIALIZATION_NVP(minSigma2_)
		 & BOOST_SERIALIZATION_NVP(maxSigma2_)
		 & BOOST_SERIALIZATION_NVP(delta_)
		 & BOOST_SERIALIZATION_NVP(sigmaDelta_)
		 & BOOST_SERIALIZATION_NVP(minDelta_)
		 & BOOST_SERIALIZATION_NVP(maxDelta_)
		 & BOOST_SERIALIZATION_NVP(parDim_)
		 & BOOST_SERIALIZATION_NVP(minVar_)
		 & BOOST_SERIALIZATION_NVP(maxVar_)
		 & BOOST_SERIALIZATION_NVP(pT_)
		 & BOOST_SERIALIZATION_NVP(iM_);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief The standard constructor */
	 explicit G_API_INDIVIDUALS GFunctionIndividualFactory(const std::string &);
	 /** @brief The copy constructor */
	 G_API_INDIVIDUALS GFunctionIndividualFactory(const GFunctionIndividualFactory &cp) = default;

	 /** @brief The destructor */
	 G_API_INDIVIDUALS ~GFunctionIndividualFactory() override = default;

	 /**************************************************************************/
	 // Getters and setters

	 /** @brief Allows to retrieve the adaptionThreshold_ variable */
	 G_API_INDIVIDUALS std::uint32_t getAdaptionThreshold() const;
	 /** @brief Set the value of the adaptionThreshold_ variable */
	 G_API_INDIVIDUALS void setAdaptionThreshold(std::uint32_t adaptionThreshold);

	 /** @brief Allows to retrieve the adProb_ variable */
	 G_API_INDIVIDUALS double getAdProb() const;
	 /** @brief Set the value of the adProb_ variable */
	 G_API_INDIVIDUALS void setAdProb(double adProb);

	 /** @brief Allows to retrieve the iM_ variable */
	 G_API_INDIVIDUALS initMode getIM() const;
	 /** @brief Set the value of the iM_ variable */
	 G_API_INDIVIDUALS void setIM(initMode im);

	 /** @brief Allows to retrieve the parDim_ variable */
	 G_API_INDIVIDUALS std::size_t getParDim() const;
	 /** @brief (Re-)Set the dimension of the function */
	 G_API_INDIVIDUALS void setParDim(std::size_t);

	 /** @brief Allows to retrieve the pT_ variable */
	 G_API_INDIVIDUALS parameterType getPT() const;
	 /** @brief Set the value of the pT_ variable */
	 G_API_INDIVIDUALS void setPT(parameterType pt);

	 /** @brief Allows to retrieve the useBiGaussian_ variable */
	 G_API_INDIVIDUALS bool getUseBiGaussian() const;
	 /** @brief Set the value of the useBiGaussian_ variable */
	 G_API_INDIVIDUALS void setUseBiGaussian(bool useBiGaussian);

	 /** @brief Allows to retrieve the minVar_ variable */
	 G_API_INDIVIDUALS double getMinVar() const;
	 /** @brief Allows to retrieve the maxVar_ variable */
	 G_API_INDIVIDUALS double getMaxVar() const;
	 /** @brief Extract the minimum and maximum boundaries of the variables */
	 G_API_INDIVIDUALS std::tuple<double, double> getVarBoundaries() const;
	 /** @brief Set the minimum and maximum boundaries of the variables */
	 G_API_INDIVIDUALS void setVarBoundaries(std::tuple<double, double>);

	 /** @brief Allows to retrieve the delta_ variable */
	 G_API_INDIVIDUALS double getDelta() const;
	 /** @brief Set the value of the delta_ variable */
	 G_API_INDIVIDUALS void setDelta(double delta);
	 /** @brief Allows to retrieve the minDelta_ variable */
	 G_API_INDIVIDUALS double getMinDelta() const;
	 /** @brief Allows to retrieve the maxDelta_ variable */
	 G_API_INDIVIDUALS double getMaxDelta() const;
	 /** @brief Allows to retrieve the allowed value range of delta */
	 G_API_INDIVIDUALS std::tuple<double, double> getDeltaRange() const;
	 /** @brief Allows to set the allowed value range of delta */
	 G_API_INDIVIDUALS void setDeltaRange(std::tuple<double, double>);

	 /** @brief Allows to retrieve the minSigma1_ variable */
	 G_API_INDIVIDUALS double getMinSigma1() const;
	 /** @brief Allows to retrieve the maxSigma1_ variable */
	 G_API_INDIVIDUALS double getMaxSigma1() const;
	 /** @brief Allows to retrieve the allowed value range of sigma1_ */
	 G_API_INDIVIDUALS std::tuple<double, double> getSigma1Range() const;
	 /** @brief Allows to set the allowed value range of sigma1_ */
	 G_API_INDIVIDUALS void setSigma1Range(std::tuple<double, double>);

	 /** @brief Allows to retrieve the minSigma2_ variable */
	 G_API_INDIVIDUALS double getMinSigma2() const;
	 /** @brief Allows to retrieve the maxSigma2_ variable */
	 G_API_INDIVIDUALS double getMaxSigma2() const;
	 /** @brief Allows to retrieve the allowed value range of sigma2_ */
	 G_API_INDIVIDUALS std::tuple<double, double> getSigma2Range() const;
	 /** @brief Allows to set the allowed value range of sigma2_ */
	 G_API_INDIVIDUALS void setSigma2Range(std::tuple<double, double>);

	 /** @brief Allows to retrieve the sigma1_ variable */
	 G_API_INDIVIDUALS double getSigma1() const;
	 /** @brief Set the value of the sigma1_ variable */
	 G_API_INDIVIDUALS void setSigma1(double sigma1);

	 /** @brief Allows to retrieve the sigma2_ variable */
	 G_API_INDIVIDUALS double getSigma2() const;
	 /** @brief Set the value of the sigma2_ variable */
	 G_API_INDIVIDUALS void setSigma2(double sigma2);

	 /** @brief Allows to retrieve the sigmaDelta_ variable */
	 G_API_INDIVIDUALS double getSigmaDelta() const;
	 /** @brief Set the value of the sigmaDelta_ variable */
	 G_API_INDIVIDUALS void setSigmaDelta(double sigmaDelta);

	 /** @brief Allows to retrieve the sigmaSigma1_ variable */
	 G_API_INDIVIDUALS double getSigmaSigma1() const;
	 /** @brief Set the value of the sigmaSigma1_ variable */
	 G_API_INDIVIDUALS void setSigmaSigma1(double sigmaSigma1);

	 /** @brief Allows to retrieve the sigmaSigma2_ variable */
	 G_API_INDIVIDUALS double getSigmaSigma2() const;
	 /** @brief Set the value of the sigmaSigma2_ variable */
	 G_API_INDIVIDUALS void setSigmaSigma2(double sigmaSigma2);

	 /** @brief Allows to retrieve the rate of evolutionary adaption of adProb_ */
	 G_API_INDIVIDUALS double getAdaptAdProb() const;
	 /** @brief Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature) */
	 G_API_INDIVIDUALS void setAdaptAdProb(double adaptAdProb);

	 /** @brief Allows to retrieve the allowed range for adProb_ variation */
	 G_API_INDIVIDUALS std::tuple<double, double> getAdProbRange() const;
	 /** @brief Allows to set the allowed range for adaption probability variation */
	 G_API_INDIVIDUALS void setAdProbRange(double minAdProb, double maxAdProb);

	 // End of public getters and setters
	 /***************************************************************************/

	 /** @brief Loads the data of another GFunctionIndividualFactory object */
	 G_API_INDIVIDUALS void load(std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>>) override;
	 /** @brief Creates a deep clone of this object */
	 G_API_INDIVIDUALS std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> clone() const override;

protected:
	 /** @brief Creates individuals of this type */
	 G_API_INDIVIDUALS std::shared_ptr <GParameterSet> getObject_(Gem::Common::GParserBuilder &, const std::size_t &) override;
	 /** @brief Allows to describe local configuration options in derived classes */
	 G_API_INDIVIDUALS void describeLocalOptions_(Gem::Common::GParserBuilder &) override;
	 /** @brief Allows to act on the configuration options received from the configuration file */
	 G_API_INDIVIDUALS void postProcess_(std::shared_ptr<GParameterSet> &) override;

private:
	 /** @brief Set the value of the minVar_ variable */
	 void setMinVar(double minVar);

	 /** @brief Set the value of the maxVar_ variable */
	 void setMaxVar(double maxVar);

	 /** @brief Set the value of the minDelta_ variable */
	 void setMinDelta(double minDelta);

	 /** @brief Set the value of the maxDelta_ variable */
	 void setMaxDelta(double maxDelta);

	 /** @brief Set the value of the minSigma1_ variable */
	 void setMinSigma1(double minSigma1);

	 /** @brief Set the value of the maxSigma1_ variable */
	 void setMaxSigma1(double maxSigma1);

	 /** @brief Set the value of the minSigma2_ variable */
	 void setMinSigma2(double minSigma2);

	 /** @brief Set the value of the maxSigma2_ variable */
	 void setMaxSigma2(double maxSigma2);

	 /** @brief The default constructor; Only needed for (de-)serialization purposes. */
	 GFunctionIndividualFactory();

	 Gem::Common::GOneTimeRefParameterT<double> adProb_{GFI_DEF_ADPROB};
	 Gem::Common::GOneTimeRefParameterT<double> adaptAdProb_{GFI_DEF_ADAPTADPROB};
	 Gem::Common::GOneTimeRefParameterT<double> minAdProb_{GFI_DEF_MINADPROB};
	 Gem::Common::GOneTimeRefParameterT<double> maxAdProb_{GFI_DEF_MAXADPROB};
	 Gem::Common::GOneTimeRefParameterT<std::uint32_t> adaptionThreshold_{GFI_DEF_ADAPTIONTHRESHOLD};
	 Gem::Common::GOneTimeRefParameterT<bool> useBiGaussian_{GFI_DEF_USEBIGAUSSIAN};
	 Gem::Common::GOneTimeRefParameterT<double> sigma1_{GFI_DEF_SIGMA1};
	 Gem::Common::GOneTimeRefParameterT<double> sigmaSigma1_{GFI_DEF_SIGMASIGMA1};
	 Gem::Common::GOneTimeRefParameterT<double> minSigma1_{GFI_DEF_MINSIGMA1};
	 Gem::Common::GOneTimeRefParameterT<double> maxSigma1_{GFI_DEF_MAXSIGMA1};
	 Gem::Common::GOneTimeRefParameterT<double> sigma2_{GFI_DEF_SIGMA2};
	 Gem::Common::GOneTimeRefParameterT<double> sigmaSigma2_{GFI_DEF_SIGMASIGMA2};
	 Gem::Common::GOneTimeRefParameterT<double> minSigma2_{GFI_DEF_MINSIGMA2};
	 Gem::Common::GOneTimeRefParameterT<double> maxSigma2_{GFI_DEF_MAXSIGMA2};
	 Gem::Common::GOneTimeRefParameterT<double> delta_{GFI_DEF_DELTA};
	 Gem::Common::GOneTimeRefParameterT<double> sigmaDelta_{GFI_DEF_SIGMADELTA};
	 Gem::Common::GOneTimeRefParameterT<double> minDelta_{GFI_DEF_MINDELTA};
	 Gem::Common::GOneTimeRefParameterT<double> maxDelta_{GFI_DEF_MAXDELTA};
	 Gem::Common::GOneTimeRefParameterT<std::size_t> parDim_{GFI_DEF_PARDIM};
	 Gem::Common::GOneTimeRefParameterT<double> minVar_{GFI_DEF_MINVAR};
	 Gem::Common::GOneTimeRefParameterT<double> maxVar_{GFI_DEF_MAXVAR};
	 Gem::Common::GOneTimeRefParameterT<parameterType> pT_{GFI_DEF_PARAMETERTYPE};
	 Gem::Common::GOneTimeRefParameterT<initMode> iM_{GFI_DEF_INITMODE};
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A simple constraint checker searching for valid solutions that fulfill
 * a given constraint. Here, the sum of all double variables needs to be smaller
 * than a given constant.
 */
class GDoubleSumConstraint : public GParameterSetConstraint {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSetConstraint)
		 & BOOST_SERIALIZATION_NVP(C_);
	 }
	 ///////////////////////////////////////////////////////////////////////
public:

	 /** @brief The default constructor */
	 G_API_INDIVIDUALS GDoubleSumConstraint() = default;
	 /** @brief Initialization with the constant */
	 explicit G_API_INDIVIDUALS GDoubleSumConstraint(const double &);
	 /** @brief The copy constructor */
	 G_API_INDIVIDUALS GDoubleSumConstraint(const GDoubleSumConstraint &cp) = default;

	 /** @brief The destructor */
	 G_API_INDIVIDUALS ~GDoubleSumConstraint() override = default;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 G_API_INDIVIDUALS void compare(
		 const GObject & // the other object
		 , const Gem::Common::expectation & // the expectation for this object, e.g. equality
		 , const double & // the limit for allowed deviations of floating point types
	 ) const final;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 G_API_INDIVIDUALS void addConfigurationOptions(Gem::Common::GParserBuilder&) override;

protected:
	 G_API_INDIVIDUALS double check_(const GParameterSet *) const override;

	 /** @brief Loads the data of another GParameterSetMultiConstraint */
	 G_API_INDIVIDUALS void load_(const GObject *) override;

private:
	 /** @brief Creates a deep clone of this object */
	 G_API_INDIVIDUALS GObject *clone_() const override;

	 double C_ = 1.; ///< The constant that should not be exceeded by the sum of parameters
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constraint checker trying to enforce a condition x+y+z=C (note the equal
 * sign!) for double variables
 */
class GDoubleSumGapConstraint : public GParameterSetConstraint {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSetConstraint)
		 & BOOST_SERIALIZATION_NVP(C_)
		 & BOOST_SERIALIZATION_NVP(gap_);
	 }
	 ///////////////////////////////////////////////////////////////////////
public:

	 /** @brief The default constructor */
	 G_API_INDIVIDUALS GDoubleSumGapConstraint() = default;
	 /** @brief Initialization with the constant */
	 G_API_INDIVIDUALS GDoubleSumGapConstraint(const double &, const double &);
	 /** @brief The copy constructor */
	 G_API_INDIVIDUALS GDoubleSumGapConstraint(const GDoubleSumGapConstraint& cp) = default;

	 /** @brief The destructor */
	 G_API_INDIVIDUALS ~GDoubleSumGapConstraint() override = default;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 G_API_INDIVIDUALS void compare(
		 const GObject & // the other object
		 , const Gem::Common::expectation & // the expectation for this object, e.g. equality
		 , const double & // the limit for allowed deviations of floating point types
	 ) const final;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 G_API_INDIVIDUALS void addConfigurationOptions(Gem::Common::GParserBuilder &) override;

protected:
	 G_API_INDIVIDUALS double check_(const GParameterSet *) const override;

	 /** @brief Loads the data of another GParameterSetMultiConstraint */
	 G_API_INDIVIDUALS void load_(const GObject *) override;

private:
	 /** @brief Creates a deep clone of this object */
	 G_API_INDIVIDUALS GObject *clone_() const override;

	 double C_ = 1.; ///< The constant that should not be exceeded by the sum of parameters
	 double gap_ = 0.5; ///< A tolerance around C_ that is still considered to be valid
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A simple constraint checker searching for valid solutions that fulfill
 * a given constraint. Here, valid solutions lie in a sphere around 0
 */
class GSphereConstraint : public GParameterSetConstraint {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;
		 ar
		 &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSetConstraint);
	 }
	 ///////////////////////////////////////////////////////////////////////
public:

	 /** @brief The default constructor */
	 G_API_INDIVIDUALS GSphereConstraint() = default;
	 /** @brief Initialization with the diameter */
	 explicit G_API_INDIVIDUALS GSphereConstraint(const double &cp);
	 /** @brief The copy constructor */
	 G_API_INDIVIDUALS GSphereConstraint(const GSphereConstraint &) = default;

	 /** @brief The destructor */
	 G_API_INDIVIDUALS ~GSphereConstraint() override = default;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 G_API_INDIVIDUALS void compare(
		 const GObject & // the other object
		 , const Gem::Common::expectation & // the expectation for this object, e.g. equality
		 , const double & // the limit for allowed deviations of floating point types
	 ) const final;

	 /** @brief Adds local configuration options to a GParserBuilder object */
	 G_API_INDIVIDUALS void addConfigurationOptions(Gem::Common::GParserBuilder &) override;

protected:
	 G_API_INDIVIDUALS double check_(const GParameterSet *) const override;

	 /** @brief Loads the data of another GParameterSetMultiConstraint */
	 G_API_INDIVIDUALS void load_(const GObject *) override;

private:
	 /** @brief Creates a deep clone of this object */
	 G_API_INDIVIDUALS GObject *clone_() const override;

	 /** @brief The diameter of the sphere */
	 double diameter_ = 1.;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFunctionIndividual)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFunctionIndividualFactory)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDoubleSumConstraint)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDoubleSumGapConstraint)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GSphereConstraint)

#endif /* GFUNCTIONINDIVIDUAL_HPP_ */
