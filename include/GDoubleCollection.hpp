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


#ifndef GDOUBLECOLLECTION_H_
#define GDOUBLECOLLECTION_H_

using namespace std;

#include "GObject.hpp"
#include "GTemplateValueCollection.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GRandom.hpp"


namespace GenEvA
{

  const double DEFINIT = 100.;

  /**********************************************************************/
  /**
   * This class represents a collection of double values, all modified
   * using the same algorithm. Using the framework provided by GTemplateValueCollection 
   * and GDoubleGaussAdaptor, this class becomes rather simple.
   */
  class GDoubleCollection
    :public GTemplateValueCollection<double>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GDCGTemplateValueCollection", boost::serialization::base_object<GTemplateValueCollection<double> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GDoubleCollection();
    /** \brief Initialize with a number of random numbers */
    explicit GDoubleCollection(int16_t);
    /** \brief Initialize with a number of random values 
     * within given boundaries */
    GDoubleCollection(int16_t, double, double);
    /** \brief The standard copy constructor */
    GDoubleCollection(const GDoubleCollection& cp);
    /** \brief The standard destructor */
    virtual ~GDoubleCollection();

    /** \brief The standard assignment operator */
    const GDoubleCollection& operator=(const GDoubleCollection&);
    /** \brief An assignment operator for the base class GObject */
    virtual const GObject& operator=(const GObject&);

    /** \brief Creates a deep copy of this object. */
    virtual GObject *clone(void);
    /** \brief Resets the object to its initial state */
    virtual void reset(void);
    /** \brief Loads the data fron another GDoubleCollection object */
    virtual void load(const GObject * gb);

    /** \brief Appends double values in a given range */
    void addData(int16_t, double, double);
   	
    /** \brief Assembles a report about the objects internal state */
    virtual string assembleReport(uint16_t indention = 0) const;
  };

  /**********************************************************************/

} /* namespace GenEvA */

#endif /*GDOUBLECOLLECTION_H_*/
