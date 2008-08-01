/**
 * @file
 */

/* GDoubleGaussAdaptor.hpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GDOUBLEGAUSSADAPTOR_H_
#define GDOUBLEGAUSSADAPTOR_H_

// GenEvA headers go here

#include "GAdaptorT.hpp"
#include "GObject.hpp"
#include "GRandom.hpp"
#include "GHelperFunctionsT.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

const double DEFAULTSIGMA = 0.1; // start value for sigma_
const double DEFAULTSIGMASIGMA = 0.; // means: do not mutate sigma_ at all
const double DEFAULTMINSIGMA = 0.0000001; // minimum allowed value for sigma_

/*************************************************************************/
/**
 * The GDoubleGaussAdaptor reprensents an adaptor used for the mutation of
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
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor - every adaptor needs a name */
	explicit GDoubleGaussAdaptor(std::string name);
	/** @brief A standard constructor, including initialization of the sigma value */
	GDoubleGaussAdaptor(double sigma, std::string name);
	/** @brief A standard constructor including initialization of the sigma, sigmaSigma and
	 *  minSigma values */
	GDoubleGaussAdaptor(double sigma, double sigmaSigma, double minSigma, std::string name);
	/** @brief The standard copy constructor */
	GDoubleGaussAdaptor(const GDoubleGaussAdaptor& cp);
	/** @brief The standard destructor */
	virtual ~GDoubleGaussAdaptor();

	/** @brief A standard assignment operator */
	const GDoubleGaussAdaptor& operator=(const GDoubleGaussAdaptor&);

	/** @brief Resets the object to its initial state */
	virtual void reset(void);
	/** @brief Loads the values of another GDoubleGaussAdaptor */
	virtual void load(const GObject *gb);
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone(void);

	/** @brief Initializes a new mutation run */
	virtual void initNewRun(void);
	/** @brief Specifies the mutations performed in this class */
	virtual void customMutations(double &value);

	/** @brief Sets the width of the gaussian */
	void setSigma(double sigma);
	/** @brief Retrieves the current width of the gaussian */
	double getSigma(void) const;

	/** @brief Sets the width of the sigma adaption and the minimally
	 * allowed  value for sigma */
	void setSigmaSigma(double sigmaSigma, double minSigma);
	/** @brief Sets a minimal value for sigma */
	void setMinSigma(double minSigma);
	/** @brief Retrieves the current value of the sigma adaption */
	double getSigmaSigma(void) const;
	/** @brief Retrieves the current minimal value allowed for sigma */
	double getMinSigma(void) const;

	/** @brief Sets all values needed for the mutation in one go */
	void setAll(double sigma, double sigmaSigma, double minSigma);

private:
	/** @brief The default constructor - not for public consumption */
	GDoubleGaussAdaptor(void);

	double sigma_;
	double sigmaSigma_;
	double minSigma_;
};

/*************************************************************************/

}} /* namespace Gem::GenEvA */

#endif /*GDOUBLEGAUSSADAPTOR_H_*/
