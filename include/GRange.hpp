/**
 * \file
 */

/**
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

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GRANGE_H_
#define GRANGE_H_

using namespace std;

#include "GObject.hpp"
#include "GBoundary.hpp"
#include "GException.hpp"

namespace GenEvA
{

  /***********************************************************************/

  const int16_t DEFMINDIGITS = 5;
  const int16_t MAXDIGITS = 16;

  const bool ISOPEN = true;
  const bool ISCLOSED = false;
  const bool ISUPPER = true;
  const bool ISLOWER = false;

  const double DEFAULTRANGE=10.;

  /** Forward declaration */
  class GBoundary;

  /**
   * A GRange represents a range of floating point values, with uper and lower,
   * open or closed boundaries (see the GBoundary class for a more detailed explanation).
   * A GRange obkect can be either active or inactive. This class represents this concept.
   * The class is mainly used in the context of the GDouble class. 
   */
  class GRange
    :public GenEvA::GObject
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GRGObject", boost::serialization::base_object<GObject>(*this));
      ar & make_nvp("lower", GRange::_lower);
      ar & make_nvp("upper", GRange::_upper);
      ar & make_nvp("isactive",_isactive);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GRange(void);
    /** \brief Allows to set all relevant values in one go */
    GRange(double lw, bool lwopen, double up, bool upopen);
    /** \brief A standard copy constructor */
    GRange(const GRange& cp);
    /** \brief The standard destructor */
    virtual ~GRange();

    /** \brief A standard assignment operator */
    const GRange& operator=(const GRange&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);

    /** \brief Loads the data of another GRange object */
    virtual void load(const GObject * gb);
    /** \brief Creates a deep copy of this object */
    virtual GObject *clone(void);
    /** \brief Resets the object to its initial state */
    void reset(void);

    /** \brief Marks the range as active */
    void setIsActive(bool);
    /** \brief Checks whether the range is active */
    bool isActive(void) const;

    /** \brief Tries to limit the range in such a way that a minimum 
     * number of decimal places is present */
    void setMinDigits(int16_t);

    /** \brief Sets the upper and lower limits in one go */
    void setBoundaries(double, bool, double, bool);

    /** \brief Retrieves the value of the lower boundary */
    double lowerBoundary(void);
    /** \brief Retrieves the value of the upper boundary */
    double upperBoundary(void);
    /** \brief Retrieves the width of the boundary */
    double width(void);

    /** \brief Checks whether a value is in the range */
    bool isIn(double);
    /** \brief Checks whether another range overlaps with this range */
    bool overlaps(GRange& other);
    /** \brief Checks whether another range is contained in this range */
    bool contains(GRange& other);

    /** \brief Assembles a report about the inner state of the object */
    virtual string assembleReport(uint16_t indention = 0) const;
  private:
    /** \brief Sets the lower boundary of the range */
    void setLowerBoundary(double, bool);
    /** \brief Sets the upper boundary of the range */
    void setUpperBoundary(double, bool);

    GBoundary _lower, _upper; ///< The lower and upper boundaries of the range
    bool _isactive; ///< A variable indicating whether the range is active
  };

  /***********************************************************************/

} // namespace GenEvA

/***********************************************************************/

#endif /*GRANGE_H_*/
