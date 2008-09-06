/**
 * @file GDoubleGaussAdaptor.hpp
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

#ifndef GDOUBLEGAUSSADAPTOR_HPP_
#define GDOUBLEGAUSSADAPTOR_HPP_

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

const std::string GDGASTANDARDNAME = "GDoubleGaussAdaptor";

/*************************************************************************/
/**
 * The GDoubleGaussAdaptor represents an adaptor used for the mutation of
 * double values through the addition of gaussian-distributed random numbers.
 * See the documentation of GAdaptorT<T> for further information on adaptors
 * in the GenEvA context. This class is at the core of evolutionary strategies,
 * as implemented by this library.
 */
class GDoubleGaussAdaptor:
	public GAdaptorT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT_double", boost::serialization::base_object<GAdaptorT<double> >(*this));
		ar & make_nvp("sigma_", sigma_);
		ar & make_nvp("sigmaSigma_", sigmaSigma_);
		ar & make_nvp("minSigma_", minSigma_);
		ar & make_nvp("maxSigma_", maxSigma_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor - every adaptor needs a name */
	GDoubleGaussAdaptor();
	/** @brief A standard constructor, including initialization of the sigma value */
	explicit GDoubleGaussAdaptor(const double&);
	/** @brief A standard constructor including initialization of the sigma_, sigmaSigma_,
	 *  minSigma_ and maxSigma_ values */
	GDoubleGaussAdaptor(const double&, const double&, const double&, const double&);
	/** @brief The standard copy constructor */
	GDoubleGaussAdaptor(const GDoubleGaussAdaptor&);
	/** @brief The standard destructor */
	virtual ~GDoubleGaussAdaptor();

	/** @brief A standard assignment operator */
	const GDoubleGaussAdaptor& operator=(const GDoubleGaussAdaptor&);

	/** @brief Loads the values of another GDoubleGaussAdaptor */
	virtual void load(const GObject *);
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone();

	/** @brief Sets the width of the gaussian used to adapt values */
	void setSigma(const double&);
	/** @brief Retrieves the current width of the gaussian */
	double getSigma() const throw();

	/** @brief Sets the allowed range of the sigma_ value */
	void setSigmaRange(const double&, const double&);
	/** @brief Retrieves the allowed range of the sigma_ value */
	std::pair<double,double> getSigmaRange() const throw();
	/** @brief Sets a new value for sigmaSigma_ */
	void setSigmaAdaptionRate(const double&);
	/** @brief Retrieves the current value of sigmaSigma_ */
	double getSigmaAdaptionRate() const throw();

	/** @brief Sets all values needed for the mutation in one go */
	void setAll(const double&, const double&, const double&, const double&);

	/**********************************************************************/
	/**
	 * Returns the standard name of a GDoubleGaussAdaptor
	 *
	 * @return The name assigned to adaptors of this type
	 */
	static std::string adaptorName() throw() {
		return GDGASTANDARDNAME;
	}

	/**********************************************************************/

protected:
	/** @brief Adapts the mutation parameters */
	virtual void adaptMutation();

	/** @brief Specifies the mutations performed in this class */
	virtual void customMutations(double &);

private:
	double sigma_; ///< The width of the gaussian used to adapt values
	double sigmaSigma_; ///< affects sigma_ adaption
	double minSigma_; ///< minimum allowed value for sigma_
	double maxSigma_; ///< maximum allowed value for sigma_
};

/*************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GDOUBLEGAUSSADAPTOR_HPP_ */
