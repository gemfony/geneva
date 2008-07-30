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

#include <cfloat>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GBOUNDARY_H_
#define GBOUNDARY_H_

using namespace std;

#include "GObject.hpp"
#include "GException.hpp"

namespace GenEvA
{

  /***************************************************************************/
  /**
   * Boundaries can be either above ("upper") or below ("lower") a given
   * range of floating point values. They can be open boundaries (i.e. exclude
   * their base value) or closed boundaries (i.e. they include their base
   * value. The GBoundary class implements this model. It is mainly used in
   * the context of the GRange class.
   */
  class GBoundary
    :public GenEvA::GObject
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GBGObject", boost::serialization::base_object<GObject>(*this));
      ar & make_nvp("boundary", _boundary);
      ar & make_nvp("isactive",_isactive);
      ar & make_nvp("isupper", _isupper);
      ar & make_nvp("isopen", _isopen);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GBoundary(void);
    /** \brief A constructor that lets us set most values in one go */
    GBoundary(double boundary, bool isupper, bool isopen);
    /** \brief A standard copy constructor */
    GBoundary(const GBoundary& cp);
    /** \brief A standard destructor */
    virtual ~GBoundary();
	
    /** \brief A standard assignment operator */
    const GBoundary& operator=(const GBoundary&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);


    /** \brief Resets the object to its initial state */
    virtual void reset(void);
    /** \brief Loads the data of another GBoundary object */
    virtual void load(const GObject *cp);
    /** \brief Creates a deep copy of this object */
    virtual GObject *clone(void);

    /** \brief Retrieves the current value of this boundary */
    virtual double value() const;

    /** \brief Marks the boundary as active or inactive */
    void setIsActive(bool);
    /** \brief Checks whether this boundary is active */
    bool isActive(void) const;

    /** \brief Checks whether this is an upper boundary */
    bool isUpper(void) const;

    /** \brief Checks whether this is an open boundary */
    bool isOpen(void) const;

    /** \brief Sets the boundary to a given value (open/closed, upper/lower) */
    double setBoundary(double, bool, bool);
    /** \brief Retrieves the current value of the boundary */
    double getBoundary(void) const;

    /** \brief Report the inner state of the object */
    virtual string assembleReport(uint16_t indention = 0) const;

  private:
    /** \brief Make sure we know whether we are an upper or lower boundary */
    void setIsUpper(bool);

    /** \brief Make sure we know whether we are an open or a closed boundary */
    void setIsOpen(bool);

    /** \brief Non-intrusively set the boundary */
    void setBoundary(double boundary);


    double _boundary; ///< Internal representation of the boundary
    bool _isactive; ///< Specifies whether this boundary is active
    bool _isupper; ///< Are we an upper or lower boundary ?
    bool _isopen; ///< Is this an open boundary ?
  };

  /***************************************************************************/

} /* namespace GenEvA */

#endif /*GBOUNDARY_H_*/
