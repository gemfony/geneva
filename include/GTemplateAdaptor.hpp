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


#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

#ifndef GTEMPLATEADAPTOR_H_
#define GTEMPLATEADAPTOR_H_

using namespace std;

#include "GObject.hpp"
#include "GHelperFunctions.hpp"

namespace GenEvA
{

  /******************************************************************************************/
  /**
   * In GenEvA, two mechanisms exist that let the user specify the
   * type of mutation he wants to have executed on collections of
   * items (basic types or any other types).  The most basic
   * possibility is for the user to overload the GMember::customMutate()
   * function and manually specify the types of mutations (s)he
   * wants. This allows great flexibility, but is not very practicable
   * for standard mutations.
   *
   * Classes derived from GMutable<T> can additionally store
   * "adaptors". These are templatized function objects that can act
   * on the items of a collection of user-defined types. Predefined
   * adaptors exist for standard types (with the most prominent
   * examples being bits and double values).
   *
   * The GTemplateAdaptor class mostly acts as an interface for these
   * adaptors, but also implements some functionality of its own.
   *
   * Adaptors can be applied to single items T or collections of items
   * vector<T>. In collections, the initialization function
   * initNewRun() can be called either for each invocation of the
   * adaptor, or for sequences, indicated by the variable _alwaysInit,
   * set by the caller.
   *
   * In order to use this class, the user must derive a class fom
   * GTemplateAdaptor and specify the type of mutation he wishes to
   * have applied to items, by overloading of
   * GTemplateAdaptor<T>::customMutate(T&) .  T will often be
   * represented by a basic value (double, long, bool, ...). Where
   * this is not the case, the adaptor will only be able to access
   * public functions of T, unless T declares the adaptor as a friend.
   *
   * As a derivative of GObject, this class follows similar rules as
   * the other GenEvA classes.
   */
  template <class T>
  class GTemplateAdaptor
    :public GObject
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GTAGObject", boost::serialization::base_object<GObject>(*this));
      ar & make_nvp("_alwaysInit", GTemplateAdaptor<T>::_alwaysInit);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /***********************************************************************************/
    /**
     * Every adaptor is required to have a name. It is set in this
     * constructor and the name is passed to the underlying GObject
     * class.  No default constructor exists, as we want to enforce
     * names for adaptops. By default we want to call the
     * initialization function of the adaptor for every item of a
     * collection.
     * 
     * \param name The name assigned to the adaptor
     */
    explicit GTemplateAdaptor(string name) throw()
      :GObject(name),
       _alwaysInit(true)
    { /* nothing */ }

    /***********************************************************************************/
    /**
     * A standard copy constructor.
     * 
     * \param cp A copy of another GTemplateAdaptor<T>
     */
    GTemplateAdaptor(const GTemplateAdaptor<T>& cp) throw()
      :GObject(cp),
       _alwaysInit(cp._alwaysInit)
    { /* nothing */ }

    /***********************************************************************************/
    /**
     * The standard destructor. We have no local, dynamically allocated data, so the body of
     * this function is empty.
     */
    virtual ~GTemplateAdaptor()
    { /* nothing */ }

    /***********************************************************************************/
    /** 
     * A standard assignment operator for GTemplateAdaptor<T> objects,
     * 
     * \param cp A copy of another GTemplateAdaptor<T> object
     */
    const GTemplateAdaptor<T>& operator=(const GTemplateAdaptor<T>& cp){
      GTemplateAdaptor<T>::load(&cp);
      return *this;
    }
	
    /***********************************************************************************/    
    /** 
     * A standard assignment operator for GTemplateAdaptor<T> objects,
     * 
     * \param cp A copy of another GTemplateAdaptor<T> object, camouflaged as a GObject
     */	
    virtual const GObject& operator=(const GObject& cp){
      GTemplateAdaptor<T>::load(&cp);
      return *this;	
    }

    /***********************************************************************************/    
    /**
     * This function resets the GTemplateAdaptor<T> to its initial state.
     */
    virtual void reset(void){
      _alwaysInit = true;
      GObject::reset();
    }

    /***********************************************************************************/    
    /**
     * Loads the content of another GTemplateAdaptor<T>. The function
     * is similar to a copy constructor (but with a pointer as
     * argument). As this function might be called in an environment
     * where we do not know the exact type of the class, the
     * GTemplateAdaptor<T> is camouflaged as a GObject . This implies the
     * need for dynamic conversion.
     * 
     * \param gb A pointer to a GTemplateAdaptor<T>, camouflaged as a GObject
     */
    virtual void load(const GObject *gb){
      const GTemplateAdaptor<T> *gta = dynamic_cast<const GTemplateAdaptor<T> *>(gb);

      // dynamic_cast will emit a NULL pointer, if the conversion failed
      if(!gta){
	gls << "In GTemplateAdaptor<T>::load() : Conversion error!" << endl
	    << logLevel(CRITICAL);
	
	exit(1);
      }

      // Check that this object is not accidently assigned to itself. 
      if(gta == this){
	gls << "In GTemplateAdaptor<T>::load(): Error!" << endl
	    << "Tried to assign an object to itself." << endl
	    << logLevel(CRITICAL);

	exit(1);
      }

      // Now we can load the actual data	
      GObject::load(gb);
      _alwaysInit = gta->_alwaysInit;
    }

    /***********************************************************************************/    
    /** \brief Creates a deep copy of this object */
    virtual GObject *clone(void)=0;

    /***********************************************************************************/    
    /**
     * Common interface for all adaptors to the mutation
     * functionality. The user specifies this functionality in the
     * customMutate() function. As customMutate is user-supplied, it
     * may throw, so we need to catch its exceptions here.
     * 
     * \param val The value that needs to be mutated
     */
    void mutate(T& val) throw()
    {
      try{
	this->customMutate(val);
      }
      catch(std::exception& e){
	gls << "In GTemplateAdaptor<T>::mutate(T& val):" << endl
	    << "Caught exception with message" << endl
	    << e.what() << endl
	    << logLevel(CRITICAL);

	exit(1);
      }
      catch(...){
	gls << "In GTemplateAdaptor<T>::mutate(T& val):" << endl
	    << "Caught unknown exception" << endl
	    << logLevel(CRITICAL);

	exit(1);
      }
    }

    /***********************************************************************************/    
    /**
     * Mutation of sequences of values, stored in an STL vector. This function also calls an
     * initialization function for the mutation function, either for each value, or only once
     * per sequence (depending on the value of _alwaysInit). initNewRun() may throw(), as it is
     * user-supplied, so we also need to catch its exceptions.
     * 
     * \param collection A vector of values that need to be mutated 
     */
    virtual void mutate(vector<T>& collection){
      typename vector<T>::iterator it;
      for(it=collection.begin(); it!=collection.end(); ++it)
	{
	  if(alwaysInit() || it==collection.begin()){
	    try{
	      initNewRun();
	    }
	    catch(std::exception& e){
	      gls << "In GTemplateAdaptor<T>::mutate(vector<T>& collection):" << endl
		  << "Caught exception with message" << endl
		  << e.what() << endl
		  << logLevel(CRITICAL);

	      exit(1);
	    }
	    catch(...){
	      gls << "In GTemplateAdaptor<T>::mutate(vector<T>& collection):" << endl
		  << "Caught unknown exception" << endl
		  << logLevel(CRITICAL);
	      
	      exit(1);
	    }
	  }

	  this->mutate(*it);
	}
    }

    /***********************************************************************************/    
    /**
     * This is a convenience interface to the GTemplateAdaptor<T>::mutate(T&) function.
     * 
     * \param val The value that is going to be mutated 
     */
    void operator()(T& val) throw()
    {
      this->mutate(val);
    }

    /***********************************************************************************/    
    /**
     * This is a convenience interface to the GTemplateAdaptor<T>::mutate(vector<T> &) function.
     * 
     * \param val The collection of values that is going to be mutated 
     */
    void operator()(vector<T>& collection){
      this->mutate(collection);
    }

    /***********************************************************************************/    
    /**
     * Retrieves the value of the _alwaysInit variable. If set to true, mutations will be 
     * initialized for each item of a sequence. If set to false, initialization will only
     * happen for the first item.
     * 
     * \return The value of the _alwaysInit variable
     */
    bool alwaysInit(void) const throw()
    {
      return _alwaysInit;
    }

    /***********************************************************************************/    
    /** 
     * Sets the value of _alwaysInit. If set to true, mutations will be 
     * initialized for each item of a sequence. If set to false, initialization will only
     * happen for the first item.
     * 
     * \param val The value that should be assigned to the _alwaysInit variable
     */
    void setAlwaysInit(bool val) throw()
    {
      _alwaysInit = val;
    }

    /***********************************************************************************/    
    /** 
     *  This function is re-implemented by derived classes, if they wish to
     *  implement special behaviour upon a new mutation run. E.g., an internal
     *  variable could be set to a new value. This is used in GDoubleGaussAdaptor
     *  to modify the sigma of the gaussian, if desired. The function will be
     *  called for each item of a sequence, if _alwaysInit is set to true, otherwise
     *  it will be called only for the first item. 
     */
    virtual void initNewRun(void)
    { /* nothing */ }

    /***********************************************************************************/    
    /**
     * Reports about the inner state of this object. The GObject report function is called so
     * we also know about the state of the parent class.
     *
     * \param indention The number of white spaces in front of each output line 
     * \return A string containing a report about the inner state of the object
     */
    virtual string assembleReport(uint16_t indention) const{
      ostringstream oss;
      oss << ws(indention) << "GTemplateAdaptor<T>: " << this << endl
	  << ws(indention) << "Mutations will be initialized for " << (_alwaysInit?"each item":"only the first item") << " of a sequence" << endl
	  << ws(indention) << "-----> Report from parent class GObject : " << endl
	  << GObject::assembleReport(indention+NINDENTION) << endl;
      return oss.str();
    }

  protected:
    /***********************************************************************************/    
    /** \brief Mutation of values as specified by the user */
    virtual void customMutate(T& val)=0;

  private:
    /***********************************************************************************/    
    /** \brief Default constructor. Private as we want every adaptor to have a name.
	Intentionally left undefined. */
    GTemplateAdaptor(void);

    bool _alwaysInit;
  };


  /******************************************************************************************/

} /* namespace GenEvA */

/********************************************************************************************/

#if BOOST_VERSION <= 103500

/**
 * The macro BOOST_IS_ABSTRACT() of Boost's serialization framework
 * does not work for templatized classes. Hence we need to provide a suitable 
 * replacement.
 */
namespace boost {
  namespace serialization {
    template<class T>
    struct is_abstract<GenEvA::GTemplateAdaptor<T> > {
      typedef mpl::bool_<true> type;
      BOOST_STATIC_CONSTANT(bool, value = true);
    };
  }}

#else

// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {                                    
  namespace serialization {                             
    template<class T>        
    struct is_abstract<GenEvA::GTemplateAdaptor<T> > : boost::true_type {};
    template<class T>                                            
    struct is_abstract< const GenEvA::GTemplateAdaptor<T> > : boost::true_type {};  
  }} 

#endif /* Serialization traits */

/********************************************************************************************/

#endif /*GTEMPLATEADAPTOR_H_*/
