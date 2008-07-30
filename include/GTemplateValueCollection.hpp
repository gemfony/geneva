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
#include <boost/function.hpp>

#ifndef GTEMPLATEVALUECOLLECTION_H_
#define GTEMPLATEVALUECOLLECTION_H_

using namespace std;

#include "GMutable.hpp"

namespace GenEvA
{

  /******************************************************************************************/
  /**
   * This is the base class for GenEvA value collections, 
   * i.e. collections of basic values such as double, long, etc. .
   * It fills a similar niche as the GTemplateValue<T> class.
   * Note that it is assumed that there is an operator= for T.
   * It is generally recommended to use this class as the base
   * class for simple types only. Use GManagedMemberCollection
   * for objects with the GMember interface. If you store 
   * pointers in this class, use a smart pointer such as 
   * boost::shared_ptr<T> .
   */
  template <class T>
  class GTemplateValueCollection
    :public GMutable<T>, 
     public vector<T>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GTVCGMutable", boost::serialization::base_object<GMutable<T> >(*this));
      ar & make_nvp("GTVCVector", boost::serialization::base_object<vector<T> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /************************************************************************/
    /**
     * The default constructor. Provided so we can provide empty collections
     * that can be filled at a later time.
     */
    GTemplateValueCollection(void)
      :GMutable<T>(), vector<T>()
    { /* nothing */ }

    /************************************************************************/
    /**
     * Fill the object with a given number of values. This function has
     * the same interface as the corresponding vector<T> constructor.
     * 
     * \param num The number of values to fill the vector with
     * \param val The value used to initialise the vector's values with
     */
    GTemplateValueCollection(size_t num, const T& val)
      :GMutable<T>(), vector<T>(num,val)
    { /* nothing */ }

    /************************************************************************/
    /**
     * The default copy constructor. Its main purpose is to call
     * the parent class'es copy constructors.
     * 
     * \param cp Another GTemplateValueCollection<T> object
     */
    GTemplateValueCollection(const GTemplateValueCollection<T>& cp)
      :GMutable<T> (cp), vector<T>(cp)
    { /* nothing */ }
	
    /************************************************************************/
    /**
     * The standard destructor. We only have the vector data to destroy,
     * Note that this means that derived classes do not need to take
     * care of emptying the vector. IMPORTANT: If you store pointers
     * in this class, we recommend to use boost::shared_ptr<T>, so it is
     * ensured that the destructors are called for objects pointed to.
     */
    virtual ~GTemplateValueCollection(){
      vector<T>::clear();
    }

    /************************************************************************/
    /**
     * A standard assignment operator
     * 
     * \param cp Another GTemplateValueCollection<T> object
     */
    const GTemplateValueCollection<T>& operator=(const GTemplateValueCollection<T>& cp){
      GTemplateValueCollection<T>::load(&cp);
      return *this;		
    }

    /************************************************************************/
    /**
     * An assignment operator for objects of parent class GObject
     * 
     * \param cp Another GTemplateValueCollection<T> object, camouflaged as a GObject
     */
    virtual const GObject& operator=(const GObject& cp){
      GTemplateValueCollection<T>::load(&cp);
      return *this;
    }

    /************************************************************************/
    /**
     * Resets the object it had at the time of construction using
     * the default constructor.
     */
    virtual void reset(void){
      vector<T>::clear();
      GMutable<T>::reset();
    }

    /************************************************************************/
    /**
     * Loads the data of another GTemplateValueCollection<T> object, 
     * camouflaged as a GObject.
     * 
     * \param gm Another GTemplateValueCollection<T> object, camouflaged as a GObject.
     */
    virtual void load(const GObject * gm){
      // Check that this object is not accidently assigned to itself. 
      // As we do not actually do any calls with this pointer, we
      // can use the faster static_cast<>
      if(static_cast<const GTemplateValueCollection<T> *>(gm) == this){
	GException ge;
	ge << "In GTemplateValueCollection<T>::load(): Error!" << endl
	   << "Tried to assign an object to itself." << endl
	   << raiseException;
      }
		
      GMutable<T>::load(gm);

      const GTemplateValueCollection<T> *gtvc = dynamic_cast<const GTemplateValueCollection<T> *>(gm);
      if(gtvc){
	uint16_t sz_f = gtvc->size(), sz_l = this->size();
	typename vector<T>::iterator it_l;
	typename vector<T>::const_iterator it_f;	
	
	if(sz_f == sz_l){
	  // note that we only check one iterator. We already know the two vectors have the same size
	  for(it_l=this->begin(), it_f=gtvc->begin(); 
	      it_l!=this->end();	++it_l, ++it_f){
	    *it_l = *it_f; // ATTENTION: This assumes that there is an operator= for T! 
	  }
	}
	// the foreign collection has fewer entries, so
	// we need to erase some entries locally
	else if(sz_f < sz_l){
	  for(it_l=this->begin(), it_f=gtvc->begin(); 
	      it_f!=gtvc->end();	++it_l, ++it_f){ // we check against the foreign vector, which is smaller
	    *it_l = *it_f; // ATTENTION: This assumes that there is an operator= for T! 
	  }
	  this->resize(sz_f); // The last elements will be dropped
	}
	else{ // so we have (sz_f > sz_l) and need to create new entries
	  for(it_l=this->begin(), it_f=gtvc->begin(); 
	      it_l!=this->end();	++it_l, ++it_f){ // we check against our own vector, which is smaller
	    *it_l = *it_f; // ATTENTION: This assumes that there is an operator= for T! 
	  }
	  for(it_f=gtvc->begin()+sz_l; it_f!=gtvc->end(); ++it_f) this->push_back(T(*it_f));
	}
			
	// Make sure our evaluation function is copied over
	evaluationFunction_=gtvc->evaluationFunction_;
      }
      else{
	GException ge;
	ge << "In GTemplateValueCollection<T>::load() : Conversion error!" << endl << raiseException;
      }
    }
	
    /************************************************************************/
    /**
     * Creates a deep copy of this object. As it is not intended to be instantiable, we 
     * leave this function empty.
     */
    virtual GObject *clone(void)=0;

    /************************************************************************/
    /**
     * This function can serve as the basis for cross-over operations (common
     * in genetic algorithms) with other GTemplateValueCollection<T> objects. 
     * Cross-over happens at the position indicated.
     * 
     * \param gtc A pointer to another GTemplateValueCollection<T> object
     * \param pos The position where cross-over should take place
     * \return A value indicating whether the cross-over worked
     */
    bool crossOver(GTemplateValueCollection<T> *gtc, uint16_t pos){
      // check that sizes are correct
      uint16_t size_a = this->size(), size_b = gtc->size();
      if(pos > size_a || pos > size_b) return false;

      // exchange entries. As we only work on the lower parts of the arrays,
      // no bound checking has to take place anymore.
      this->swap_ranges(this->begin(), this->begin()+pos, gtc->begin());
      return true;
    }
	
    /************************************************************************/
    /** 
     * Perform a cross-over operation at a random position within the bounds of
     * both vectors. 
     * 
     * \param gtc A pointer to another GTemplateValueCollection<T> object
     * \return A value indicating whether the cross-over worked
     */
    bool crossOver(GTemplateValueCollection<T> *gtc){
      return crossOver(gtc, this->gr.discreteRandom(0,GMin<uint16_t>(this->size(), gtc->size())));
    }
	
    /************************************************************************/
    /**
     * Registers an evaluation function for this class.
     * 
     * \param evaluationFunction An evaluation function for this class
     */
    void registerEvaluationFunction(boost::function<double (GTemplateValueCollection<T> * const)> evaluationFunction){
      evaluationFunction_ = evaluationFunction;
    }
	
    /************************************************************************/
    /**
     * This function assembles a report about the class'es internal state and then calls the 
     * parent classes' assembleReport() function. As we do not know what values are stored in
     * our vector and whether they are suitable for a stream, we do not emit information about them.
     * 
     * \param indention The number of white spaces in front of each output line 
     * \return A string containing a report about the inner state of the object
     */
    virtual string assembleReport(uint16_t indention = 0) const{
      ostringstream oss;
      oss << ws(indention) << "GTemplateValueCollection<T>: " << this << endl
	  << ws(indention) << "value vector <intentionally unreported>" << endl 
	  << ws(indention) << "-----> Report from parent class GMutable<T> : " << endl
	  << GMutable<T>::assembleReport(indention+NINDENTION) << endl;
      return oss.str();
    }

  protected:
    /************************************************************************/
    /** 
     * User-supplied function to specify how mutation is to be carried out. 
     */
    virtual void customMutate(void){
      GMutable<T>::applyAllAdaptors(*this);
    }

    /************************************************************************/
    /** 
     * User-supplied evaluation function. It is possible for the user to 
     * register an evaluation function with this object. Where it is not present,
     * this function just returns 0. . Note that such callback functions
     * cannot be serialized. Hence, in environments where objects might be
     * serialized and the user wants to retain information about the evaluation
     * function, it might be necessary to derive a class from this one and to
     * specifiy the customFitness function. In case you are sure a suitable evaluation
     * function has been registered with all entities, this is of course not
     * necessary. A recommended approach is to create server and clients in main() 
     * from the same source.
     * 
     * \return The value of this object or 0, if no evaluation function has been registered
     */
    virtual double customFitness(void){
      if(evaluationFunction_) return evaluationFunction_(this);
      else return 0.;
    }
	
  private:
    boost::function<double (GTemplateValueCollection<T> * const)> evaluationFunction_; ///< Used to evaluate this object
  };

  /******************************************************************************************/

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
    struct is_abstract<GenEvA::GTemplateValueCollection<T> > {
      typedef mpl::bool_<true> type;
      BOOST_STATIC_CONSTANT(bool, value = true);
    };
  }}

#else

// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {                                    
  namespace serialization {                             
    template<class T>        
    struct is_abstract<GenEvA::GTemplateValueCollection<T> > : boost::true_type {};
    template<class T>                                            
    struct is_abstract< const GenEvA::GTemplateValueCollection<T> > : boost::true_type {};  
  }} 

#endif /* Serialization traits */

/****************************************************************************/

#endif /*GTEMPLATEVALUECOLLECTION_H_*/
