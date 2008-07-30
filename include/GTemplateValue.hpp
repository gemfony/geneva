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

#ifndef GTEMPLATEVALUE_H_
#define GTEMPLATEVALUE_H_

using namespace std;

#include "GObject.hpp"
#include "GMember.hpp"
#include "GMutable.hpp"

/**************************************************************************/

namespace GenEvA
{
  /**
   * This class forms the basis of value types with GMember interface. 
   * Such values can be mutated with the mutate() function and can be
   * customized in terms of the mutations supported and the calculation
   * of their value. This way GenEvA values can form intelligent entities
   * by themselves. E.g., the likelihood for a bit flip of a bit value can
   * be mutated alongside the value itself  and adapt to changing 
   * circumstances.
   *
   * Note that we make an assumption in this class that operator= gives
   * usefull results for T-objects.
   */
  template <class T>
  class GTemplateValue
    :public GMutable<T>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GMutable", boost::serialization::base_object<GMutable<T> >(*this));
      ar & make_nvp("_tValue", _tValue);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /************************************************************************/
    /** 
     * The standard constructor for this class, labelled "explicit" so 
     * it is not accidentally used for value conversion.
     * 
     * \param val The new internal value 
     */
    explicit GTemplateValue(const T& val) : GMutable<T>()
    {
      this->setInternalValue(val);
    }
	
    /************************************************************************/
    /**
     * A standard copy constructor.
     *
     * \param cp A copy of another object of this type
     */
    GTemplateValue(const GTemplateValue<T>& cp)
      :GMutable<T>(cp), _tValue(cp._tValue)
    { /* nothing */ }
	
    /************************************************************************/
    /** 
     * The standard destructor. We have no local, dynamically allocated 
     * data, hence it is empty. 
     */
    virtual ~GTemplateValue()
    { /* nothing */ }

    /************************************************************************/
    /**
     * A standard assignment operator
     * 
     * \param cp Another GTemplateValue<T> object
     * \return A constant reference to this object
     */
    const GTemplateValue<T>& operator=(const GTemplateValue<T>& cp){
      GTemplateValue<T>::load(&cp);
      return *this;		
    }

    /************************************************************************/
    /**
     * An assignment operator for objects of parent class GObject
     * 
     * \param cp Another GTemplateValue<T> object, camouflaged as a GObject
     * \return A constant reference to this object, camouflaged as a GObject
     */
    virtual const GObject& operator=(const GObject& cp){
      GTemplateValue<T>::load(&cp);
      return *this;
    }
	
    /************************************************************************/
    /** 
     * Resets the object to its initial state upon initialization with the 
     * default constructor. First it resets local values, then it takes
     * care of the parent class. 
     */
    virtual void reset(void){
      // can this cause problems if T is an object type ? 
      // Apparently not - tried with gcc 4.3.1 in a sample program
      _tValue = (T)NULL;
      GMutable<T>::reset();
    }
    
    /************************************************************************/
    /** 
     * Loads the data of another GTemplateValue<T> object, camouflaged as a GObject
     * 
     * \param gm Another GTemplateValue<T> object, camouflaged as a GObject 
     */
    virtual void load(const GObject * gm){
      const GTemplateValue<T> *gtv = dynamic_cast<const GTemplateValue<T> *>(gm);	

      // dynamic_cast will emit a NULL pointer, if the conversion failed
      if(!gtv){
	this->gls << "In GTemplateValue::load() : Conversion Error!" << endl 
		  << logLevel(CRITICAL);

	exit(1);
      }


      // Check that this object is not accidently assigned to itself. 
      if(gtv == this){
	this->gls << "In GTemplateValue<T>::load(): Error!" << endl
		  << "Tried to assign an object to itself." << endl
		  << logLevel(CRITICAL);
	
	exit(1);
      }

      // Now we can load the actual data
      GMutable<T>::load(gm);
      _tValue = gtv->_tValue; 
    }

    /************************************************************************/
    /** 
     * Creates a deep copy of the object. We do not want this class to be instantiable,
     * hence we make this function purely virtual. 
     */
    virtual GObject *clone(void)=0;

    /************************************************************************/
    /** 
     * This function retrieves the internal value of this class. Please note that
     * this is not necessarily the same as the externally visible value. An example would
     * be the GDouble class, where a mapping from internal to externally visible value is
     * made. Use GTemplateValue<T>::getValue() (or an overloaded version) to make sure to
     * use the external value.
     * 
     * \return The internal value of this object 
     */
    T getInternalValue(void) const{
      return _tValue;
    }
	
    /************************************************************************/
    /** 
     * Retrieves the externally visible value of the object. This function needs 
     * to be overloaded for classes that have a different external value than the 
     * internal value. This is e.g. the case for the GDouble class. The standard 
     * way to access the value of a GTemplateValue class (i.e. of its derivatives 
     * -- this class is purely virtual) is to use the GTemplateValue<T>::getValue() 
     * function.
     * 
     * \return The externally visible value of this object
     */
    virtual T getValue(void) const{
      return getInternalValue();
    }
		
    /************************************************************************/
    /**
     * This function assembles a report about the classes' internal state and then calls the 
     * parent classes' assembleReport() function. As we do not know whether _tValue can 
     * be passed to a stream, we do not emit information about it, but simply call the 
     * parent class'es assembleReport() function.
     * 
     * \param indention The number of white spaces in front of each output line 
     * \return A string containing a report about the inner state of the object
     */
    virtual string assembleReport(uint16_t indention = 0) const{
      ostringstream oss;
      oss << ws(indention) << "GTemplateValue<T>: " << this << endl
	  << ws(indention) << "_tValue <intentionally unreported>" << endl // needs to be reported in derived classes!
	  << ws(indention) << "-----> Report from parent class GObject : " << endl
	  << GMutable<T>::assembleReport(indention+NINDENTION) << endl;
      return oss.str();
    }

  protected:
    /************************************************************************/
    /** 
     * Sets the internal value to a given value. Accessible only
     * to derived classes. 
     */
    virtual void setInternalValue(const T& val){
      _tValue = val;
    }

  private:
    /************************************************************************/
    /**
     * The internal representation of this object. Note that this can
     * very well differ from the user-visible value. 
     * 
     * IMPORTANT: As we use Boost to serialize this class, _tValue
     * needs to be serializable! This is the case for all GenEvA
     * types derived from GObject, for standard, built-in types and
     * for most classes of the Boost framework and the C++ standard
     * library.
     */ 
    T _tValue;

    /************************************************************************/
    /**
     * We do not want this constructor to be called. It must nevertheless be
     * accessible to the Boost serialization framework, hence we provide an
     * empty implementation.
     */
    GTemplateValue(void) : GMutable<T>()
    {/* nothing */}	
  };

  /**************************************************************************/

} /* namespace GenEvA */


/****************************************************************************/

#if BOOST_VERSION <= 103500

/**
 * The macro BOOST_IS_ABSTRACT() of Boost's serialization framework
 * does not work for templatized classes. Hence we need to provide a suitable 
 * replacement.
 */
namespace boost {
  namespace serialization {
    template<class T>
    struct is_abstract<GenEvA::GTemplateValue<T> > {
      typedef mpl::bool_<true> type;
      BOOST_STATIC_CONSTANT(bool, value = true);
    };
  }}

#else

// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {                                    
  namespace serialization {                             
    template<class T>        
    struct is_abstract<GenEvA::GTemplateValue<T> > : boost::true_type {};
    template<class T>                                            
    struct is_abstract< const GenEvA::GTemplateValue<T> > : boost::true_type {};  
  }} 

#endif /* Serialization traits */

/****************************************************************************/

#endif /* GTEMPLATEVALUE_H_ */
