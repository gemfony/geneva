/**
 * @file
 */

/* GBoundedDouble.hpp
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
#include <vector>
#include <sstream>
#include <cmath>

// Boost headers go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#ifndef GBOUNDEDDOUBLE_HPP_
#define GBOUNDEDDOUBLE_HPP_

// GenEvA headers go here
#include "GDoubleGaussAdaptor.hpp"
#include "GParameterT.hpp"
#include "GObject.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem
{
namespace GenEvA
{

  /******************************************************************************/
  /* The GBoundedDouble class represents a double value, equipped with the
   * ability to mutate itself. The value range can have an upper and a lower
   * limit. The range has to be set during the initial creation. Once set, it cannot
   * be changed anymore. Mutated values will only appear inside the given range.
   * Note that appropriate adaptors (see e.g the GDoubleGaussAdaptor class) need
   * to be loaded in order to benefit from the mutation capabilities.
   */
  class GBoundedDouble
     :public GParameterT<double>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GParameterT_dbl", boost::serialization::base_object<GParameterT<double> >(*this));
      ar & make_nvp("internalValue_", internalValue_);
      ar & make_nvp("lowerBoundary_", lowerBoundary_);
      ar & make_nvp("lowerBoundary_", upperBoundary_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
	/** @brief Initialization with the boundaries */
	GBoundedDouble(double, double);
    /** @brief Initialization with a double and the boundaries */
    GBoundedDouble(double, double, double);
    /** @brief Standard copy constructor */
    GBoundedDouble(const GBoundedDouble&);
    /** @brief Standard destructor */
    virtual ~GBoundedDouble();

    /** @brief A standard assignment operator */
    const GBoundedDouble& operator=(const GBoundedDouble&);
    /** @brief An assignment operator for double values */
    virtual double operator=(double);

    /** @brief Resets the class to its initial state */
    virtual void reset();
    /** @brief Loads the data of another GBoundedDouble */
    virtual void load(const GObject *);
    /** @brief Creates a deep copy of this class */
    virtual GObject *clone();

    /** @brief Retrieves the lower boundary */
    double getLowerBoundary() const throw();
    /** @brief Retrieves the upper boundary */
    double getUpperBoundary() const throw();

    /** @brief Automatic conversion to a double */
    operator double ();

    /** @brief Mutates this object */
    virtual void mutate();

  private:
	/** @brief Standard constructor, intentionally private */
	GBoundedDouble();
    /** @brief Sets the external value */
	double setExternalValue(double);

    double lowerBoundary_, upperBoundary_;
    double internalValue_; ///< The internal representation of this classes value
  };

  /******************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOUNDEDDOUBLE_HPP_ */
