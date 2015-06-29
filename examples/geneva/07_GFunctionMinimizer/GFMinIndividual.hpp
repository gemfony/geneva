/**
 * @file GFMinIndividual.hpp
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
#include <sstream>
#include <vector>
#include <tuple>

// Boost header files go here

#ifndef GFMININDIVIDUAL_HPP_
#define GFMININDIVIDUAL_HPP_

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "common/GParserBuilder.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum targetFunction {
	GFM_PARABOLA=0
	, GFM_NOISYPARABOLA=1
};

// Make sure targetFunction can be streamed
/** @brief Puts a Gem::Geneva::targetFunction into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::targetFunction&);

/** @brief Reads a Gem::Geneva::targetFunction from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Geneva::targetFunction&);

/******************************************************************************/
// A number of default settings for the factory
const double GFI_DEF_ADPROB = 1.0;
const double GFI_DEF_SIGMA = 0.025;
const double GFI_DEF_SIGMASIGMA = 0.2;
const double GFI_DEF_MINSIGMA = 0.001;
const double GFI_DEF_MAXSIGMA = 1;
const std::size_t GFI_DEF_PARDIM = 2;
const double GFI_DEF_MINVAR = -10.;
const double GFI_DEF_MAXVAR = 10.;
const targetFunction GO_DEF_TARGETFUNCTION = boost::numeric_cast<targetFunction>(0);

/******************************************************************************/
/**
 * This individual searches for a minimum of a number of predefined functions, each capable
 * of processing their input in multiple dimensions.
 */
class GFMinIndividual
	: public GParameterSet
{
	/////////////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		& BOOST_SERIALIZATION_NVP(targetFunction_);
	}

	/////////////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GFMinIndividual();
	/** @brief A standard copy constructor */
	GFMinIndividual(const GFMinIndividual&);
	/** @brief The standard destructor */
	virtual ~GFMinIndividual();

	/** @brief A standard assignment operator */
	const GFMinIndividual& operator=(const GFMinIndividual&);

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&) final;

	/** @brief Allows to set the demo function */
	void setTargetFunction(targetFunction);
	/** @brief Allows to retrieve the current demo function */
	targetFunction getTargetFunction() const;

	/** @brief Retrieves the average value of the sigma used in Gauss adaptors */
	double getAverageSigma() const;

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GFMinIndividual */
	virtual void load_(const GObject*) final;
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const final;

	/** @brief The actual value calculation takes place here */
	virtual double fitnessCalculation() final;

	/***************************************************************************/

private:
	targetFunction targetFunction_; ///< Specifies which demo function should be used

	/***************************************************************************/
	/** @brief A simple n-dimensional parabola */
	double parabola(const std::vector<double>& parVec) const;
	/** @brief A "noisy" parabola */
	double noisyParabola(const std::vector<double>& parVec) const;

	/***************************************************************************/
};

/******************************************************************************/
/**
 * Provides an easy way to print the individual's content
 */
std::ostream& operator<<(std::ostream&, const Gem::Geneva::GFMinIndividual&);
std::ostream& operator<<(std::ostream&, std::shared_ptr<Gem::Geneva::GFMinIndividual>);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFMinIndividual objects
 */
class GFMinIndividualFactory
	: public Gem::Common::GFactoryT<GParameterSet>
{
public:
	/** @brief The standard constructor */
	GFMinIndividualFactory(const std::string&);
	/** @brief The destructor */
	virtual ~GFMinIndividualFactory();

protected:
	/** @brief Creates individuals of this type */
	virtual std::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	/** @brief Allows to describe local configuration options in derived classes */
	virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(std::shared_ptr<GParameterSet>&);

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GFMinIndividualFactory() = delete;

	double adProb_;
	double sigma_;
	double sigmaSigma_;
	double minSigma_;
	double maxSigma_;
	std::size_t parDim_;
	double minVar_;
	double maxVar_;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFMinIndividual)

#endif /* GFMININDIVIDUAL_HPP_ */
