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

#ifndef GBIT_H_
#define GBIT_H_

using namespace std;

#include "GTemplateValue.hpp"
#include "GEnums.hpp"

namespace GenEvA
{

  /************************************************************************/
  /**
   * This class encapsulates a single bit. This might appear heavy weight,
   * and indeed for most applications this is not the recommended solution -
   * use the GBitCollection instead. Bits are mutated by the GBitFlipAdaptor in 
   * GenEvA. It incorporates a mutable bit-flip probability. The reason for this class 
   * is that there might be applications where one might want different flip probabilities 
   * for different bits. Where this is the case, separate GBitFlipAdaptors must be assigned 
   * to each bit value, which cannot be done with the GBitCollection. Plus, having
   * a separate bit class adds some consistency to GenEvA, as other values (most 
   * notably doubles) have their own class as well (GDouble).   
   * 
   * \todo Operators are needed!
   */
  class GBit
    :public GTemplateValue<GenEvA::bit>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GTemplateValue", boost::serialization::base_object<GTemplateValue<GenEvA::bit> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The standard constructor */
    GBit(void);
    /** \brief Initialization with a value */
    explicit GBit(GenEvA::bit);
    /** \brief The standard copy constructor */
    GBit(const GBit&);
    /** \brief The standard destructor */
    virtual ~GBit();

    /** \brief The standard assignment operator */
    const GBit& operator=(const GBit&);
    /** \brief An assignment operator for GBit objects, camouflaged as a GObject */
    virtual const GObject& operator=(const GObject&);
	
    /** \brief Resets the class to its original state */
    virtual void reset(void);
    /** \brief Loads another GBit object, camouflaged as a GObject */
    virtual void load(const GObject * gb);
    /** \brief Creates a deep copy of this object */
    virtual GObject *clone(void);

    /** \brief Retrieves the user-visible value. Use this instead of value() ! */
    virtual GenEvA::bit getValue(void) const;

    /** \brief Creates a report about the inner state of the object */
    virtual string assembleReport(uint16_t indention = 0) const;

  protected:
    /** \brief User-defined evaluation schemes */
    virtual double customFitness(void);
    /** \brief User-defined mutation schemes */
    virtual void customMutate(void);
  };

  /************************************************************************/

} /* namespace GenEvA */

#endif /*GBIT_H_*/
