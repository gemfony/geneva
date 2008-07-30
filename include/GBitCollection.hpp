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

#ifndef GBITCOLLECTION_H_
#define GBITCOLLECTION_H_

using namespace std;

#include "GTemplateValueCollection.hpp"
#include "GEnums.hpp"

namespace GenEvA
{

  /**********************************************************************/
  /**
   * This class represents collections of bits. They are usually mutated by 
   * the GBitFlipAdaptor, which has a mutable flip probability. One adaptor
   * is applied to all bits. If you want individual flip probabilities for
   * all bits, use GBit objects instead and put them into a GMemberCollection . 
   */
  class GBitCollection
    :GTemplateValueCollection<GenEvA::bit>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GTemplateValueCollection", boost::serialization::base_object<GTemplateValueCollection<GenEvA::bit> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GBitCollection(void);
    /** \brief Random initialization with a given number of values */
    explicit GBitCollection(int16_t);
    /** \brief A standard copy constructor */
    GBitCollection(const GBitCollection&);
    /** \brief The standard destructor */
    virtual ~GBitCollection();

    /** \brief A standard assignment operator */
    const GBitCollection& operator=(const GBitCollection&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);
	
    /** \brief Creates a deep copy of this object */
    virtual GObject *clone(void)=0;
    /** \brief Resets the object to its initial state */
    virtual void reset(void);
    /** \brief Loads the data of another GBitCollection class */
    virtual void load(const GObject *);

    /** \brief Creates a report about the inner state of the object */
    virtual string assembleReport(uint16_t indention = 0) const;
  };

  /**********************************************************************/

} /* namespace GenEvA */

#endif /*GBITCOLLECTION_H_*/
