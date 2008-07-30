/**
 * \file
 */

/**
 * Copyright (C) 2008 Dr. Ruediger Berlich
 * Copyright (C) 2008 Forschungszentrum Karlsruhe GmbH
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

#ifndef GRATEABLE_H_
#define GRATEABLE_H_

using namespace std;

#include "GObject.hpp"
#include "GRandom.hpp"
#include "GMutable.hpp"

namespace GenEvA
{
    /** 
     * These values are needed to determine whether customFitness() should be prevented
     * from running, may run or should be executed immediately.
     */
    const uint8_t PREVENTEVALUATION = 0;
    const uint8_t ALLOWEVALUATION = 1;
    const uint8_t ENFORCEEVALUATION = 2;

    /**
     * GIndividual is the parent class for all those classes that can be part of
     * a population or an individual (possibly as the base class of a user-defined
     * class. Major characteristics of a GIndividual or its derived classes include 
     * the ability to have a fitness (used for quality assessment and sorting).
     * Along with this ability goes the mutability, which is implemented by the 
     * parent class GMutable.
     *
     * \todo Add ability to register evaluation functions as function objects
     */
    template <class T>
    class GIndividual
	:public GMutable<T>
    {      
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
	    using boost::serialization::make_nvp;
	    ar & make_nvp("GMutable", boost::serialization::base_object<GMutable<T> >(*this));
	    ar & make_nvp("_myCurrentFitness",_myCurrentFitness);
	    ar & make_nvp("_dirtyFlag",_dirtyFlag);
	    ar & make_nvp("_evaluationPermission", _evaluationPermission);
	}
	///////////////////////////////////////////////////////////////////////

    public:
	/*******************************************************************************************/
	/**
	 * The default constructor.
	 */
	GIndividual(void) throw()
	    :GMutable<T>(),
	     _myCurrentFitness(0),
	     _dirtyFlag(true),
	     _evaluationPermission(ALLOWEVALUATION), // by default we do lazy evaluation
	    { /* nothing */ }
      
	/*******************************************************************************************/
	/**
	 * A standard copy constructor.
	 * 
	 * \param cp Another GIndividual<T> object
	 */
	GIndividual(const GIndividual<T>& cp) 
	    :GMutable<T>(cp),
	     _myCurrentFitness(cp._myCurrentFitness),
	     _dirtyFlag(cp._dirtyFlag),
	     _evaluationPermission(cp._evaluationPermission)
	    { /* nothing */ }
      
	/*******************************************************************************************/
	/**
	 * The destructor.
	 */
	virtual ~GIndividual()
	    { /* nothing */ }

	/*******************************************************************************************/
	/**
	 * We want to be able to reset the object to the state it had
	 * upon initialisation with the default constructor.
	 */
	virtual void reset(void){
	    _myCurrentFitness = 0.;
	    setDirtyFlag();
	    setEvaluationPermission(ALLOWEVALUATION);
	    
	    GMutable<T>::reset();
	}

	/*******************************************************************************************/
	/**
	 * Like the GIndividual<T>::reset() function, this function needs to be
	 * overloaded by the user in each derived class. 
	 *
	 * First, we load the data of our parent object, then we load our own
	 * local data. All load() functions take a pointer to a GObject object 
	 * as parameter, so we need to do a cast to a GMember object
	 * before loading the local data.
	 * 
	 * The reason for passing a GObject pointer to this function is that the 
	 * caller will usually only see a base pointer (polymorphism).
	 *
	 * \param gmi A pointer to a GMember object, camouflaged as a pointer to a GObject.
	 */
	virtual void load(const GObject *gr){
	    const GIndividual<T> *gr_load = dynamic_cast<const GIndividualy<T> *>(gr);

	// dynamic_cast will emit a NULL pointer, if the conversion failed
	if(!gr_load){
	    this->gls << "In GIndividual<T>::load(): Conversion error!" << endl 
		      << logLevel(CRITICAL);
		
	    exit(1);
	}
	    
	// Check that this object is not accidentally assigned to itself. 
	if(gr_load == this){
	    this->gls << "In GIndividual<T>::load(): Error!" << endl
		      << "Tried to assign an object to itself." << endl
		      << logLevel(CRITICAL);
		
	    exit(1);
	}
	
	// Load all necessary data
	GMutable<T>::load(gr);
	_myCurrentFitness=gr_load->_myCurrentFitness;
	_dirtyFlag=gr_load->_dirtyFlag;
	_evaluationPermission=gr_load->_evaluationPermission;
    }

    /*******************************************************************************************/
    /** \brief Creates a clone of this GIndividual<T> object */
	virtual GObject *clone(void)=0;
	
    /*******************************************************************************************/
    /** 
     * A standard assignment operator for GIndividual<T> objects
     * 
     * \param cp A copy of another GIndividual<T> object
     * \return A constant reference to this object
     */
    const GIndividual<T>& operator=(const GIndividual<T>& cp){
	GIndividual<T>::load(&cp);
	return *this;
    }

    /*******************************************************************************************/
    /** 
     * A standard assignment operator for GIndividual<T> objects, camouflaged as a GObject
     * 
     * \param cp A copy of another GIndividual<T> object, camouflaged as a GObject
     * \return A constant reference to this object
     */
    virtual const GObject& operator=(const GObject& cp){
	GIndividual<T>::load(&cp);
	return *this;	
    }

    /*******************************************************************************************/
    /**
     * The GIndividual<T>::fitness() function is a core part of the GenEvA library. The entire
     * optimization process depends on the evaluation of GIndividual<T>-derivatives.
     *
     * The calculation of the fitness of a GenEvA individual will
     * usually be very costly, as the library is particularly designed
     * for very large optimization problems. The calculation of an
     * individual's fitness could potentially take hours or even days. We
     * thus have to make sure that a re-calculation of the fitness
     * <em>only</em> takes place, if the object has been changed
     * (i.e. "mutated"). This is done by setting a flag whenever changes
     * to the object's core data have occurred. The actual fitness
     * calculation is done in a user-supplied function customFitness() .
     * This function is only called, if a <em>dirty flag</em> has been
     * set.  Otherwise a previously stored fitness is returned. This
     * behaviour is the same for all objects derived from the GIndividual<T>
     * class.
     *
     * In some cases it is dangerous to call customFitness() from within this
     * function. This can be the case when we run in a networked environment
     * where the server shall only do sorting and communicating.
     *
     * As a safeguard against accidental calls to customFitness(), this function
     * will emit an error and exit if fitness-recalculation is not permitted.
     * 
     * \return The fitness of an object derived from this class
     */
    double fitness(void) throw(){
	if (isDirty()) {
	    // It is an error to attempt evaluation in some environments
	    if(_evaluationPermission == PREVENTEVALUATION){
		this->gls << "In GIndividual<T>::fitness() : Error! Evaluation" << endl
			  << "attempted while evaluation is not allowed" << endl
			  << logLevel(CRITICAL);

		exit(1);
	    }
	    else if(_evaluationPermission == ALLOWEVALUATION ||
		    _evaluationPermission == ENFORCEEVALUATION){	  
		try{
		    _myCurrentFitness = customFitness();
		    setDirtyFlag(false);
		}
		catch(std::exception& e){
		    this->gls << "In GIndividual<T>::fitness(): Error! Caught exception " << endl
			      << "from user-supplied customFitness() function with message" << endl
			      << e.what() << endl
			      << logLevel(CRITICAL);
	  
		    exit(1);
		}
		catch(...){
		    this->gls << "In GIndividual<T>::fitness(): Error! Caught unknown exception " << endl
			      << "from user-supplied customFitness() function." << endl
			      << logLevel(CRITICAL);
	  
		    exit(1);
		}		
	    }
	    else {
		this->gls << "In GIndividual<T>::fitness(): Error! Invalid value of" << endl
			  << "_evaluationPermission variable: " << _evaluationPermission << endl
			  << logLevel(CRITICAL);
	  
		exit(1);
	    }
	}

	return _myCurrentFitness;
    }

    /*******************************************************************************************/
    /** \brief "true" if the object has been mutated (and not yet evaluated) or the <em>dirty flag</em> has been set manually */
    bool isDirty(void) const throw();

    /*******************************************************************************************/
    /** \brief Get the state of the <em>dirty flag</em> */
    bool getDirtyFlag(void) const throw();

    /*******************************************************************************************/
    /** \brief Set the dirty flag to <em>true</em>*/
    virtual void setDirtyFlag(void);

    /*******************************************************************************************/
    /** \brief Reset all members stored in this object. Must be overloaded in dervied classes */
    virtual void setDirtyFlagAll(void);

    /*******************************************************************************************/
    /** \brief Returns the internal value of this object. */
    double getMyCurrentFitness() const throw();

    /*******************************************************************************************/
    /** \brief Emit information about this class */
    virtual string assembleReport(uint16_t indention = 0) const;

    /*******************************************************************************************/
    /** \brief Sets the _evaluationPermission parameter to a given value */
    virtual uint8_t setEvaluationPermission(uint8_t);

    /*******************************************************************************************/
    /** \brief Retrieves the _evaluationPermission parameter */
    bool getEvaluationPermission(void) const throw();

protected:
    /*******************************************************************************************/
    /** \brief User-specified value-calculation of this object. Used in GRateable<T>::fitness() */
    virtual double customFitness(void)=0;
      
private:
    /*******************************************************************************************/
    /** \brief Sets the <em>dirty flag</em> to any desired value */
    void setDirtyFlag(bool flag) throw();

    /*******************************************************************************************/
    /** \brief Holds this object's internal fitness */
    double _myCurrentFitness;
    /** \brief Internal representation of the mutation status of this object */
    bool _dirtyFlag;
    /** \brief Steers whether evaluation is disallowed, allowed or needs to be enforced */
    uint8_t _evaluationPermission;
    /*******************************************************************************************/
};

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
	struct is_abstract<GenEvA::GRateable<T> > {
	    typedef mpl::bool_<true> type;
	    BOOST_STATIC_CONSTANT(bool, value = true);
	};
    }}

#else

// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {                                    
    namespace serialization {                             
	template<class T>        
	struct is_abstract<GenEvA::GRateable<T> > : boost::true_type {};
	template<class T>                                            
	struct is_abstract< const GenEvA::GRateable<T> > : boost::true_type {};  
    }} 

#endif /* Serialization traits */

/********************************************************************************************/


#endif /* GRATEABLE_H_ */
