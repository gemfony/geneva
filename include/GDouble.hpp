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

#include <vector>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#ifndef GDOUBLE_H_
#define GDOUBLE_H_

using namespace std;

#include "GRange.hpp"
#include "GBoundary.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GTemplateValue.hpp"

namespace GenEvA
{

  /******************************************************************************/
  /* The GDouble class represents a double value, equipped with the 
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

  class GDouble
    :public GTemplateValue<double>,
     private boost::operators<GDouble>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GTemplateValue", boost::serialization::base_object<GTemplateValue<double> >(*this));
      ar & make_nvp("range_", range_);
      ar & make_nvp("gps_",gps_);
    }
    ///////////////////////////////////////////////////////////////////////
		
  public:
    /** \brief The standard constructor */
    GDouble();
    /** \brief Initialisation with a double */
    GDouble(double);
    /** \brief Standard copy constructor */
    GDouble(const GDouble&);
    /** \brief Standard destructor */
    virtual ~GDouble();
	
    /** \brief A standard assignment operator */
    const GDouble& operator=(const GDouble&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);
    /** \brief An assignment operator for double values */
    double operator=(double);

    /** \brief Resets the class to its initial state */
    virtual void reset(void);
    /** \brief Loads the data of another GDouble */
    virtual void load(const GObject * gb);
    /** \brief Creats a deep copy of this class */
    virtual GObject *clone(void);

    /** \brief Sets the outer boundaries of the double value */
    void setBoundaries(double, bool, double, bool);
    /** \brief Sets the outer, closed boundaries of the double value */
    void setBoundaries(double, double);
    /** \brief Adds a gap in the value range */
    void addGap(double, bool, double, bool);
    /** \brief Adds a closed gap in the value range */
    void addGap(double, double);

    /** \brief Retrieves the external value */
    virtual double getValue(void);
    /** \brief Sets the extern value */
    double setExternalValue(double);

    /** \brief  Automatic conversion to a double */
    operator double ();

    /** \brief Comparison operator, aids Boost.Operators */
    bool operator<(GDouble&);
    /** \brief Comparison operator, aids Boost.Operators */
    bool operator==(GDouble&);
    /** \brief Self-assignment operator, aids Boost.Operators */
    GDouble& operator+=(GDouble&);
    /** \brief Self-assignment operator, aids Boost.Operators */
    GDouble& operator-=(GDouble&);    
    /** \brief Self-assignment operator, aids Boost.Operators */
    GDouble& operator*=(GDouble&);    
    /** \brief Self-assignment operator, aids Boost.Operators */
    GDouble& operator/=(GDouble&);    
    /** \brief Self-assignment operator, aids Boost.Operators */
    GDouble& operator%=(GDouble&);    
    /** \brief Self-assignment operator, aids Boost.Operators */
    GDouble& operator|=(GDouble&);    
    /** \brief Self-assignment operator, aids Boost.Operators */
    GDouble& operator&=(GDouble&);    
    /** \brief Self-assignment operator, aids Boost.Operators */
    GDouble& operator^=(GDouble&);    
    /** \brief Increment operator, aids Boost.Operators */
    GDouble& operator++();
    /** \brief Decrement operator, aids Boost.Operators */
    GDouble& operator--();
        
    /** Used for debugging purposes */
    void setIval(double x){
      GTemplateValue<double>::setInternalValue(x);
    }
    
    /** \brief Emit information about the internal state of this class */
    virtual string assembleReport(uint16_t indention = 0) const;

  protected:
    /** \brief Allows to customize value calculation */
    virtual double customFitness(void);
    /** \brief Allows to customize mutation schemes */
    virtual void customMutate(void);

  private:
    /** \brief A GDouble can be assigned boundaries */
    GRange range_;

    /** \brief Holds gaps in the external value range, if set */
    vector<shared_ptr<GRange> > gps_;

    /** \brief Sanity check for overlapping ranges/gaps */
    bool checkRanges(void);

    /** \brief Helper function to copy the ranges/gaps of another vector */
    void copyGaps(const vector<shared_ptr<GRange> > & cp);

    /** \brief Sorts gaps according to their lower value */
    void sortGaps(void);
  };

  /******************************************************************************/

} /* namespace GenEvA */

#endif /* GDOUBLE_H_ */
