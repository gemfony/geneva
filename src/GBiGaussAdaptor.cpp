/**
 * @file GBiGaussAdaptor.cpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

#include "GBiGaussAdaptor.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBiGaussAdaptor)

namespace Gem {
    namespace GenEvA {

/*************************************************************************/
/**
 * The default constructor. We do not want it to be publicly accessible.
 * Hence it is defined private. It still needs to be there due to the requirement
 * of this class being serializable (which implies default construction by
 * the friend class boost::serialization::access .
 */
	GBiGaussAdaptor::GBiGaussAdaptor(void)
	    :GAdaptorT<double> ("GBiGaussAdaptor")
	{
	    setSigma(DEFAULTDISTANCE);
	    setSigmaSigma(DEFAULTDISTANCESIGMA, DEFAULTMINDISTANCE);
	}

/*************************************************************************/
/**
 * The standard constructor. It passes the adaptor's name to the parent class
 * and initialises the internal variables.
 *
 * @param name The name assigned to this adaptor
 */
	GBiGaussAdaptor::GBiGaussAdaptor(std::string name)
	    :GAdaptorT<double> (name)
	{
	    setSigma(DEFAULTDISTANCE);
	    setSigmaSigma(DEFAULTDISTANCESIGMA, DEFAULTMINDISTANCE);
	}

/*************************************************************************/
/**
 * In addition to passing the name of the adaptor to the parent class,
 * it is also possible to specify a value for the sigma_ parameter in
 * this constructor.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param name The name assigned to this adaptor
 */
	GBiGaussAdaptor::GBiGaussAdaptor(double sigma, std::string name)
	    :GAdaptorT<double> (name)
	{
	    // These functions do error checks on sigma, so we do not assign
	    // the value directly to the private variable.
	    setSigma(sigma);
	    setSigmaSigma(DEFAULTDISTANCESIGMA, DEFAULTMINDISTANCE);
	}

/*************************************************************************/
/**
 * This constructor lets a user set all parameters in one go.
 *
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 * @param name The name assigned to this adaptor
 */
	GBiGaussAdaptor::GBiGaussAdaptor(double sigma, double sigmaSigma, double minSigma, std::string name)
	    :GAdaptorT<double> (name)
	{
	    setSigma(sigma);
	    setSigmaSigma(sigmaSigma, minSigma);
	}

/*************************************************************************/
/**
 * A standard copy constructor.
 *
 * @param cp Another GBiGaussAdaptor object
 */
	GBiGaussAdaptor::GBiGaussAdaptor(const GBiGaussAdaptor& cp)
	    :GAdaptorT<double> (cp)
	{
	    setSigma(cp.sigma_);
	    setSigmaSigma(cp.sigmaSigma_, cp.minSigma_);
	}

/*************************************************************************/
/**
 * The standard destructor. Empty, as we have no local, dynamically
 * allocated data.
 */
	GBiGaussAdaptor::~GBiGaussAdaptor()
	{ /* nothing */ }

/*******************************************************************************************/
/**
 * A standard assignment operator for GBiGaussAdaptor objects,
 *
 * @param cp A copy of another GBiGaussAdaptor object
 */
	const GBiGaussAdaptor& GBiGaussAdaptor::operator=(const GBiGaussAdaptor& cp)
	{
	    GBiGaussAdaptor::load(&cp);
	    return *this;
	}

/*************************************************************************/
/**
 * This is where the actual mutation of the supplied value takes place.
 * The sigma_ retrieved with GBiGaussAdaptor::getSigma() might get
 * mutated itself, if the sigmaSigma_ parameter is not 0. The function is
 * declared inline as it will be called very frequently. The gr obkject is
 * a protected member of GObject, one of our parents.
 *
 * @param value The value that is going to be mutated
 */
	inline void GBiGaussAdaptor::customMutations(double &value)
	{
	    // adapt the value in situ. Note that this changes
	    // the argument of this function
	    value += gr.gaussRandom(0.,sigma_);
	}

/*************************************************************************/
/**
 * This adaptor allows the evolutionary adaption of sigma_. This allows the
 * algorithm to adapt to changing geometries of the quality surface. The
 * function is declared inline, as it will be called very often.
 */
	inline void GBiGaussAdaptor::initNewRun(void)
	{
	    // do we want to adapt sigma_ at all ?
	    if(sigmaSigma_) { // != 0 ?
		sigma_ *= exp(gr.gaussRandom(0.,sigmaSigma_));
		// make sure sigma_ doesn't get too small
		if(fabs(sigma_) < minSigma_) sigma_ = minSigma_;
	    }
	}

/*************************************************************************/
/**
 * This function sets the value of the sigma_ parameter. If the value
 * does not make sense, it will be adapted to a useful value. A log
 * message is emitted in this case.
 *
 * @param sigma The initial value for the sigma_ parameter
 */
	void GBiGaussAdaptor::setSigma(double sigma)
	{
	    // A value of sigma smaller or equal 0 is not useful. Adapt and log.
	    if(sigma <= 0.)
	    {
		std::ostringstream warning;
		warning << "In GBiGaussAdaptor::setSigma(double): WARNING" << std::endl
			<< "Bad value for sigma given: " << sigma << std::endl
			<< "The value will be adapted to the default value " << DEFAULTDISTANCE << std::endl;

		LOGGER.log(warning.str(), Gem::GLogFramework::WARNING);

		sigma_ = DEFAULTDISTANCE;
		return;
	    }

	    sigma_ = sigma;
	}

/*************************************************************************/
/**
 * This function sets the values of the sigmaSigma_ parameter and the
 * minimal value allowed for sigma_. Note that there will only be adaptation
 * of sigma_, if the user specifies a value for sigmaSigma_ other than 0.
 *
 * If either sigmaSigma or minSigma do not have useful values, they will be adapted
 * and a log message will be emitted.
 *
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 */
	void GBiGaussAdaptor::setSigmaSigma(double sigmaSigma, double minSigma)
	{
	    double tmpSigmaSigma = sigmaSigma, tmpMinSigma = minSigma;

	    // A value of sigmaSigma smaller than 0. is not useful.
	    // Note that a sigmaSigma of 0 indicates that no adaption of the
	    // stepwidth is intended.
	    if(sigmaSigma < 0.)
	    {
		std::ostringstream warning;
		warning << "In GBiGaussAdaptor::setSigmaSigma(double, double): WARNING" << std::endl
			<< "Bad value for sigmaSigma given: " << sigmaSigma << std::endl
			<< "The value will be adapted to the default value " << DEFAULTDISTANCESIGMA << std::endl;

		LOGGER.log(warning.str(), Gem::GLogFramework::WARNING);

		tmpSigmaSigma = DEFAULTDISTANCESIGMA;
	    }

	    // A minimum allowed value for sigma_ <= 0 is not useful. Note that
	    // this way also 0 is forbidden as value, as no progress would be
	    // possible anymore in the optimisation.
	    if(minSigma <= 0.)
	    {
		std::ostringstream warning;
		warning << "In GBiGaussAdaptor::setSigmaSigma(double, double): WARNING" << std::endl
			<< "Bad value for minSigma given: " << minSigma << std::endl
			<< "The value will be adapted to the default value " << DEFAULTMINDISTANCE << std::endl;

		LOGGER.log(warning.str(), Gem::GLogFramework::WARNING);

		tmpMinSigma = DEFAULTMINDISTANCE;
	    }

	    sigmaSigma_ = tmpSigmaSigma;
	    setMinSigma(tmpMinSigma);
	}

/*************************************************************************/
/**
 * Retrieves the current value of sigma_.
 *
 * @return The current value of sigma_
 */
	double GBiGaussAdaptor::getSigma(void) const
	{
	    return sigma_;
	}

/*************************************************************************/
/**
 * Retrieves the current value of sigmaSigma_ .
 *
 * @return The value of sigmaSigma_
 */
	double GBiGaussAdaptor::getSigmaSigma(void) const
	{
	    return sigmaSigma_;
	}

/*************************************************************************/
/**
 * Allows to set a value for the minimally allowed sigma_. If minSigma does not
 * have a useful value, it will be reset to the default value and a log
 * message will be emitted.
 *
 * @param The minimally allowed value for sigma_
 */
	void GBiGaussAdaptor::setMinSigma(double minSigma)
	{
	    // A value of minSigma <= 0. is not useful
	    if(minSigma <= 0.)
	    {
		std::ostringstream warning;
		warning << "In GBiGaussAdaptor::setMinSigma(double): WARNING" << std::endl
			<< "Bad value for minSigma given: " << minSigma << std::endl
			<< "The value will be adapted to the default value " << DEFAULTMINDISTANCE << std::endl;

		LOGGER.log(warning.str(), Gem::GLogFramework::WARNING);

		minSigma_ = DEFAULTMINDISTANCE;
		return;
	    }

	    minSigma_=minSigma;
	}

/*************************************************************************/
/**
 * Retrieves the minimally allowed value of sigma_
 *
 * @return The minimally allowed value for sigma_
 */
	double GBiGaussAdaptor::getMinSigma(void) const
	{
	    return minSigma_;
	}

/*************************************************************************/
/**
 * Convenience function that lets users set all relevant parameters of this class
 * at once.
 * @param sigma The initial value for the sigma_ parameter
 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
 * @param minSigma The minimal value allowed for sigma_
 */
	void GBiGaussAdaptor::setAll(double sigma, double sigmaSigma, double minSigma)
	{
	    GBiGaussAdaptor::setSigma(sigma);
	    GBiGaussAdaptor::setSigmaSigma(sigmaSigma,minSigma);
	}

/*************************************************************************/
/**
 * This function creates a deep copy of this object
 *
 * @return A deep copy of this object
 */
	GObject *GBiGaussAdaptor::clone(void)
	{
	    return new GBiGaussAdaptor(*this);
	}

/*************************************************************************/
/**
 * This function loads the data of another GBiGaussAdaptor, camouflaged
 * as a GObject.
 *
 * @param A copy of another GBiGaussAdaptor, camouflaged as a GObject
 */
	void GBiGaussAdaptor::load(const GObject *cp)
	{
	    // Convert GObject pointer to local format
	    const GBiGaussAdaptor *gdga = this->conversion_cast(cp, this);

	    // Load the data of our parent class ...
	    GAdaptorT<double>::load(cp);

	    // ... and then our own data
	    sigma_ = gdga->sigma_;
	    sigmaSigma_ = gdga->sigmaSigma_;
	    minSigma_ = gdga->minSigma_;
	}

/*************************************************************************/

    } /* namespace GenEvA */
} /* namespace Gem */
