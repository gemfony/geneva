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

// Boost headers go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#ifndef GBOUNDEDDOUBLE_HPP_
#define GBOUNDEDDOUBLE_HPP_

// GenEvA headers go here
#include "GRange.hpp"
#include "GBoundary.hpp"
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
   * limit, and gaps may be added to the value range. Mutated values will
   * then only appear inside the given range and outside the gaps. Note that
   * appropriate adaptors (see e.g the GDoubleGaussAdaptor class) need to
   * be loaded in order to benefit from the mutation capabilities.
   *
   * Note that this class has several issues that need to be fixed:
   * - Constructors may still throw exceptions (?)
   * - This class (and its sibling GDoubleCollection), together with
   *   GDoubleGaussAdaptor are the source of most CPU time used in
   *   Evolutionary Strategies
   * - Floating point accuracy needs to be examined for the conversion
   *   applied between internal and external value. In particular there
   *   seems to be problems at the peaks.
   */

  class GBoundedDouble
    :public GParameterT<double>,
     private boost::operators<GBoundedDouble>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GParameterT_dbl", boost::serialization::base_object<GParameterT<double> >(*this));
      ar & make_nvp("range_", range_);
      ar & make_nvp("gps_",gps_);
      ar & make_nvp("internal_value_", internal_value_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** @brief The standard constructor */
    GBoundedDouble();
    /** @brief Initialization with a double */
    GBoundedDouble(double);
    /** @brief Standard copy constructor */
    GBoundedDouble(const GBoundedDouble&);
    /** @brief Standard destructor */
    virtual ~GBoundedDouble();

    /** @brief A standard assignment operator */
    const GBoundedDouble& operator=(const GBoundedDouble&);
    /** @brief An assignment operator for double values */
    virtual double operator=(double);

    /** @brief Resets the class to its initial state */
    virtual void reset(void);
    /** @brief Loads the data of another GBoundedDouble */
    virtual void load(const GObject *);
    /** @brief Creats a deep copy of this class */
    virtual GObject *clone(void);

    /** @brief Sets the outer boundaries of the double value */
    void setBoundaries(double, bool, double, bool);
    /** @brief Sets the outer, closed boundaries of the double value */
    void setBoundaries(double, double);
    /** @brief Adds a gap in the value range */
    void addRange(double, bool, double, bool);
    /** @brief Adds a closed gap in the value range */
    void addRange(double, double);

    /** @brief Retrieves the external value */
    virtual double getValue();
    /** @brief Sets the extern value */
    double setExternalValue(double);

    /** @brief  Automatic conversion to a double */
    operator double ();

    /** @brief Comparison operator, aids Boost.Operators */
    bool operator<(GBoundedDouble&);
    /** @brief Comparison operator, aids Boost.Operators */
    bool operator==(GBoundedDouble&);
    /** @brief Self-assignment operator, aids Boost.Operators */
    GBoundedDouble& operator+=(GBoundedDouble&);
    /** @brief Self-assignment operator, aids Boost.Operators */
    GBoundedDouble& operator-=(GBoundedDouble&);
    /** @brief Self-assignment operator, aids Boost.Operators */
    GBoundedDouble& operator*=(GBoundedDouble&);
    /** @brief Self-assignment operator, aids Boost.Operators */
    GBoundedDouble& operator/=(GBoundedDouble&);
    /** @brief Self-assignment operator, aids Boost.Operators */
    GBoundedDouble& operator%=(GBoundedDouble&);
    /** @brief Self-assignment operator, aids Boost.Operators */
    GBoundedDouble& operator|=(GBoundedDouble&);
    /** @brief Self-assignment operator, aids Boost.Operators */
    GBoundedDouble& operator&=(GBoundedDouble&);
    /** @brief Self-assignment operator, aids Boost.Operators */
    GBoundedDouble& operator^=(GBoundedDouble&);
    /** @brief Increment operator, aids Boost.Operators */
    GBoundedDouble& operator++();
    /** @brief Decrement operator, aids Boost.Operators */
    GBoundedDouble& operator--();

  protected:
    /** @brief Allows to customize value calculation */
    virtual double customFitness(void);

  private:
	/** @brief Sanity check for overlapping ranges/gaps */
	bool rangesOk(void);

	/** @brief Checks whether a given value is inside one of the ranges */
	bool valueInsideRange(double);

	/** @brief Helper function to copy the ranges/gaps of another vector */
	void copyRanges(const vector<shared_ptr<GRange> > & cp);

	/** @brief Sorts gaps according to their lower value */
	void sortRanges(void);

    GRange range_; ///< A GBoundedDouble can be assigned boundaries
    vector<shared_ptr<GRange> > gps_; /// < Holds gaps in the external value range, if set

    double internal_value_; ///< The internal representation of this classes value
  };

  /******************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOUNDEDDOUBLE_HPP_ */
