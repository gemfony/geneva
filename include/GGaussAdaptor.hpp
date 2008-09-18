/**ty
 * @file GGaussAdaptor.hpp
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

// Standard headers go here
#include <cmath>
#include <string>
#include <sstream>
#include <utility> // For std::pair

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GINTADAPTOR_HPP_
#define GINTADAPTOR_HPP_

// GenEvA headers go here

#include "GAdaptorT.hpp"
#include "GObject.hpp"
#include "GRandom.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

const double DEFAULTSIGMA = 0.1; ///< Default start value for sigma_
const double DEFAULTSIGMASIGMA = 0.001; ///< Default width of the gaussian used for sigma adaption
const double DEFAULTMINSIGMA = 0.0000001; ///< Default minimum allowed value for sigma_
const double DEFAULTMAXSIGMA = 5; ///< Default maximum allowed value for sigma_

const std::string GGAUSSADAPTORSTANDARDNAME = "GGaussAdaptor";

/************************************************************************************************/
/**
 * GGaussAdaptor represents an adaptor used for the mutation of numeric
 * types, by the addition of gaussian-distributed random numbers. Different numeric
 * types may be used, including Boost's integer representations.
 * The type used needs to be specified as a template parameter.
 */
template<typename num_type>
class GGaussAdaptor:
	public GAdaptorT<num_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT_num", boost::serialization::base_object<GAdaptorT<num_type> >(*this));
		ar & make_nvp("sigma_", sigma_);
		ar & make_nvp("sigmaSigma_", sigmaSigma_);
		ar & make_nvp("minSigma_", minSigma_);
		ar & make_nvp("maxSigma_", maxSigma_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * The standard constructor. It passes the adaptor's standard name to the
	 * parent class and initializes the internal variables.
	 */
	GGaussAdaptor()
		:GAdaptorT<num_type> (GGAUSSADAPTORSTANDARDNAME),
		 sigma_(DEFAULTSIGMA),
		 sigmaSigma_(DEFAULTSIGMASIGMA),
		 minSigma_(DEFAULTMINSIGMA),
		 maxSigma_(DEFAULTMAXSIGMA)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * In addition to passing the name of the adaptor to the parent class,
	 * it is also possible to specify a value for the sigma_ parameter in
	 * this constructor.
	 *
	 * @param sigma The initial value for the sigma_ parameter
	 */
	explicit GGaussAdaptor(const double& sigma)
		:GAdaptorT<num_type> (GDGASTANDARDNAME),
		 sigmaSigma_(DEFAULTSIGMASIGMA),
		 minSigma_(DEFAULTMINSIGMA),
		 maxSigma_(DEFAULTMAXSIGMA)
	{
		// This functions does an error check on sigma, so we do not assign
		// the value directly to the private variable.
		setSigma(sigma);
	}

	/********************************************************************************************/
	/**
	 * This constructor lets a user set all parameters in one go.
	 *
	 * @param sigma The initial value for the sigma_ parameter
	 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	 * @param minSigma The minimal value allowed for sigma_
	 * @param name The name assigned to this adaptor
	 */
	GGaussAdaptor(const double& sigma, const double& sigmaSigma,
				const double& minSigma, const double& maxSigma)
		:GAdaptorT<num_type> (GDGASTANDARDNAME)
	{
		// These functions do error checks on their values
		setSigmaAdaptionRate(sigmaSigma);
		setSigmaRange(minSigma, maxSigma);
		setSigma(sigma);
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp Another GGaussAdaptor object
	 */
	GGaussAdaptor(const GGaussAdaptor<num_type>& cp)
		:GAdaptorT<num_type> (cp)
	{
		setSigmaAdaptionRate(cp.sigmaSigma_);
		setSigmaRange(cp.minSigma_, cp.maxSigma_);
		setSigma(cp.sigma_);
	}

	/********************************************************************************************/
	/**
	 * The standard destructor. Empty, as we have no local, dynamically
	 * allocated data.
	 */
	~GGaussAdaptor()
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A standard assignment operator for GGaussAdaptor objects,
	 *
	 * @param cp A copy of another GGaussAdaptor object
	 */
	const GGaussAdaptor<num_type>& operator=(const GGaussAdaptor<num_type>& cp)
	{
		GGaussAdaptor<num_type>::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * This function loads the data of another GGaussAdaptor, camouflaged as a GObject.
	 *
	 * @param A copy of another GGaussAdaptor, camouflaged as a GObject
	 */
	void load(const GObject *cp)
	{
		// Convert GObject pointer to local format
		const GGaussAdaptor<num_type> *gdga = this->conversion_cast(cp, this);

		// Load the data of our parent class ...
		GAdaptorT<num_type>::load(cp);

		// ... and then our own data
		sigma_ = gdga->sigma_;
		sigmaSigma_ = gdga->sigmaSigma_;
		minSigma_ = gdga->minSigma_;
		maxSigma_ = gdga->maxSigma_;
	}


	/********************************************************************************************/
	/**
	 * This function creates a deep copy of this object
	 *
	 * @return A deep copy of this object
	 */
	GObject *clone()
	{
		return new GGaussAdaptor<num_type>(*this);
	}

	/********************************************************************************************/
	/**
	 * This function sets the value of the sigma_ parameter.
	 *
	 * @param sigma The new value of the sigma_ parameter
	 */
	void setSigma(const double& sigma)
	{
		// Sigma must be in the allowed value range
		if(sigma < minSigma_ || sigma > maxSigma_)
		{
			std::ostringstream error;
			error << "In GGaussAdaptor::setSigma(const double&): Error!" << std::endl
			  	  << "sigma is not in the allowed range: " << std::endl
			  	  << sigma << " " << minSigma_ << " " << maxSigma_ << std::endl;

			throw geneva_error_condition(error.str());
		}

		sigma_ = sigma;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of sigma_.
	 *
	 * @return The current value of sigma_
	 */
	double GDoubleGaussAdaptor::getSigma() const throw() {
		return sigma_;
	}

	/********************************************************************************************/
	/**
	 * Sets the allowed value range of sigma_
	 *
	 * @param minSigma The minimum allowed value of sigma_
	 * @param minSigma The maximum allowed value of sigma_
	 */
	void setSigmaRange(const double& minSigma, const double& maxSigma){
		// Do some error checks
		if(minSigma<=0. || minSigma >= maxSigma){ // maxSigma will automatically be > 0. now
			std::ostringstream error;
			error << "In GGaussAdaptor::setSigmaRange(const double&, const double&): Error!" << std::endl
				  << "Invalid values for minSigma and maxSigma given:" << minSigma << " " << maxSigma << std::endl;

			throw geneva_error_condition(error.str());
		}

		minSigma_ = minSigma;
		maxSigma_ = maxSigma;

		// Adapt sigma, if necessary
		if(sigma_ < minSigma_) sigma_ = minSigma_;
		else if(sigma_>maxSigma_) sigma_ = maxSigma_;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the allowed value range for sigma.
	 *
	 * @return The allowed value range for sigma
	 */
	std::pair<double,double> GDoubleGaussAdaptor::getSigmaRange() const throw() {
		return std::make_pair(minSigma_, maxSigma_);
	}

	/********************************************************************************************/
	/**
	 * This function sets the values of the sigmaSigma_ parameter and the
	 * minimal value allowed for sigma_.
	 *
	 * @param sigmaSigma The new value of the sigmaSigma_ parameter
	 */
	void setSigmaAdaptionRate(const double& sigmaSigma)
	{
		// A value of sigmaSigma <= 0. is not useful.
		if(sigmaSigma <= 0.)
		{
			std::ostringstream error;
			error << "In GGaussAdaptor::setSigmaSigma(double, double): Error!" << std::endl
				  << "Bad value for sigmaSigma given: " << sigmaSigma << std::endl;

			throw geneva_error_condition(error.str());
		}

		sigmaSigma_ = sigmaSigma;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the value of sigmaSigma_ .
	 *
	 * @return The value of the sigmaSigma_ parameter
	 */
	double getSigmaAdaptionRate() const throw() {
		return sigmaSigma_;
	}

	/********************************************************************************************/
	/**
	 * Convenience function that lets users set all relevant parameters of this class
	 * at once.
	 *
	 * @param sigma The initial value for the sigma_ parameter
	 * @param sigmaSigma The initial value for the sigmaSigma_ parameter
	 * @param minSigma The minimal value allowed for sigma_
	 * @param minSigma The maximum value allowed for sigma_
	 */
	void setAll(const double& sigma, const double& sigmaSigma, const double& minSigma, const double& maxSigma)
	{
		setSigmaAdaptionRate(sigmaSigma);
		setSigmaRange(minSigma, maxSigma);
		setSigma(sigma);
	}

	/********************************************************************************************/
	/**
	 * Returns the standard name of a GGaussAdaptor
	 *
	 * @return The name assigned to adaptors of this type
	 */
	static std::string adaptorName() throw() {
		return GGAUSSADAPTORSTANDARDNAME;
	}

protected:
	/********************************************************************************************/
	/**
	 * This adaptor allows the evolutionary adaption of sigma_. This allows the
	 * algorithm to adapt to changing geometries of the quality surface. The
	 * function is declared inline, as it might be called very often.
	 */
	inline void adaptMutation()
	{
		sigma_ *= exp(gr.gaussRandom(0.,sigmaSigma_));

		// make sure sigma_ doesn't get out of range
		if(fabs(sigma_) < minSigma_) sigma_ = minSigma_;
		else if(fabs(sigma_) > maxSigma_) sigma_ = maxSigma_;
	}

	/********************************************************************************************/
	/**
	 * This is where the actual mutation of the supplied value takes place.
	 * The sigma_ retrieved with GDoubleGaussAdaptor::getSigma() might get
	 * mutated itself, if the sigmaSigma_ parameter is not 0. The function is
	 * declared inline as it will be called very frequently. The gr obkject is
	 * a protected member of GObject, one of our parents.
	 *
	 * @param value The value that is going to be mutated
	 */
	inline void customMutations(num_type &value)
	{
		// adapt the value in situ. Note that this changes
		// the argument of this function
		value += gr.gaussRandom(0.,sigma_);
	}

private:
	/********************************************************************************************/
	double sigma_; ///< The width of the gaussian used to adapt values
	double sigmaSigma_; ///< affects sigma_ adaption
	double minSigma_; ///< minimum allowed value for sigma_
	double maxSigma_; ///< maximum allowed value for sigma_
};

/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GINTADAPTOR_HPP_ */
