/**
 * @file
 */

/* GBoundary.hpp
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

// Standard header files go here
#include <cfloat>
#include <cmath>
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GBOUNDARY_HPP_
#define GBOUNDARY_HPP_

// GenEvA header files go here
#include "GObject.hpp"
#include "GHelperFunctionsT.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem
{
namespace GenEvA
{
	const bool BNDISCLOSED=false;
	const bool BNDISOPEN=true;

  /***************************************************************************/
  /**
   * Boundaries can be either above ("upper") or below ("lower") a given
   * range of floating point values. They can be open boundaries (i.e. exclude
   * their base value) or closed boundaries (i.e. they include their base
   * value. The GBoundary class implements this model. It is mainly used in
   * the context of the GRange class.
   */
  class GBoundary
    :public GObject
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GObject", boost::serialization::base_object<GObject>(*this));
      ar & make_nvp("boundary_", boundary_);
      ar & make_nvp("isactive_",isactive_);
      ar & make_nvp("isupper_", isupper_);
      ar & make_nvp("isopen_", isopen_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** @brief The default constructor */
    GBoundary();
    /** @brief A constructor that lets us set most values in one go */
    GBoundary(double boundary, bool isupper, bool isopen);
    /** @brief A standard copy constructor */
    GBoundary(const GBoundary& cp);
    /** @brief A standard destructor */
    virtual ~GBoundary();

    /** @brief A standard assignment operator */
    const GBoundary& operator=(const GBoundary&);

    /** @brief Resets the object to its initial state */
    virtual void reset();
    /** @brief Loads the data of another GBoundary object */
    virtual void load(const GObject *cp);
    /** @brief Creates a deep copy of this object */
    virtual GObject *clone();

    /** @brief Marks the boundary as active or inactive */
    void setIsActive(bool) throw();
    /** @brief Checks whether this boundary is active */
    bool isActive() const throw();

    /** @brief Checks whether this is an upper boundary */
    bool isUpper() const throw();

    /** @brief Checks whether this is an open boundary */
    bool isOpen() const throw();

    /** @brief Sets the boundary to a given value (open/closed, upper/lower) */
    double setBoundary(double, bool, bool);
    /** @brief Retrieves the current value of the boundary */
    double getBoundary() const throw();

  private:
    /** @brief Make sure we know whether we are an upper or lower boundary */
    void setIsUpper(bool) throw();

    /** @brief Make sure we know whether we are an open or a closed boundary */
    void setIsOpen(bool) throw();

    /** @brief Non-intrusively set the boundary */
    void setBoundary(double boundary) throw();

    /** @brief Returns the smallest distinguishable double value for the argument */
    double getMinDouble(double) const;

    double boundary_; ///< Internal representation of the boundary
    bool isactive_; ///< Specifies whether this boundary is active
    bool isupper_; ///< Are we an upper or lower boundary ?
    bool isopen_; ///< Is this an open boundary ?
  };

  /***************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /*GBOUNDARY_HPP_*/
