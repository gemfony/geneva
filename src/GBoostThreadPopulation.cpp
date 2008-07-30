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

#include "GBoostThreadPopulation.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GBoostThreadPopulation)

namespace GenEvA
{

  /********************************************************************/
  /**
   * A standard constructor. No local, dynamically allocated data, 
   * hence this function is empty.
   */
  GBoostThreadPopulation::GBoostThreadPopulation()
    :GBasePopulation(), 
     maxThreads_(DEFAULTBOOSTTHREADS),
     tp_(maxThreads_)
  { /* nothing */ }

  /********************************************************************/
  /**
   * A standard copy constructor.
   * 
   * \param cp Reference to another GBoostThreadPopulation object 
   */
  GBoostThreadPopulation::GBoostThreadPopulation(const GBoostThreadPopulation& cp)
    :GBasePopulation(cp),
     maxThreads_(cp.maxThreads_),
     tp_(maxThreads_)
  { /* nothing */ }

  /********************************************************************/
  /**
   * The standard destructor. We clear remaining work items in the
   * thread pool and wait for active tasks to finish.
   */
  GBoostThreadPopulation::~GBoostThreadPopulation(){
    tp_.clear();
    tp_.wait();
  }

  /********************************************************************/
  /**
   * A standard assignment operator for GBoostThreadPopulation objects
   * 
   * \param cp Reference to another GBoostThreadPopulation object
   * \return A constant reference to this object 
   */
  const GBoostThreadPopulation& GBoostThreadPopulation::operator=(const GBoostThreadPopulation& cp){
    GBoostThreadPopulation::load(&cp);
    return *this;
  }

  /********************************************************************/
  /**
   * A standard assignment operator for GBoostThreadPopulation objects,
   * camouflaged as GObjects.
   * 
   * \param cp Reference to another GBoostThreadPopulation object, camouflaged as a GObject
   * \return A constant reference to this object, camouflaged as a GObject
   */
  const GObject& GBoostThreadPopulation::operator=(const GObject& cp){
    GBoostThreadPopulation::load(&cp);
    return *this;	
  }

  /********************************************************************/
  /**
   * Resets this object to its initial state. As there is no local,
   * dynamically allocated data, all work is done by the parent class.
   */
  void GBoostThreadPopulation::reset(void){
    maxThreads_=DEFAULTBOOSTTHREADS;
    tp_.clear();
    tp_.size_controller().resize(maxThreads_);
	
    GBasePopulation::reset();
  }

  /********************************************************************/
  /**
   * Loads the data from another GBoostThreadPopulation object. As there is 
   * no local data, all work is done by the parent class.
   * 
   * \param Pointer to another GBoostThreadPopulation object, camouflaged as a GObject
   */
  void GBoostThreadPopulation::load(const GObject *cp){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GBoostThreadPopulation *>(cp) == this){
      GException ge;
      ge << "In GBoostThreadPopulation::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    GBasePopulation::load(cp);
	
    // Get a pointer to a GBoostThreadPopulation ...
    const GBoostThreadPopulation *gbp = dynamic_cast<const GBoostThreadPopulation *>(cp);
    if(gbp){
      maxThreads_ = gbp->maxThreads_;
      tp_.clear();
      tp_.size_controller().resize(maxThreads_);
    }
    else{
      GException ge;
      ge << "In GBoostThreadPopulation::load() : Conversion error! " << endl 
	 << raiseException;
    }
  }

  /********************************************************************/
  /**
   * Creates a deep clone of this object
   * 
   * \return A deep copy of this object, camouflaged as a GObject
   */
  GObject *GBoostThreadPopulation::clone(void){
    return new GBoostThreadPopulation(*this);
  }

  /********************************************************************/
  /**
   * We want to do all fitness calculation in the threads. Hence lazy
   * evaluation is not allowed.
   */
  void GBoostThreadPopulation::optimize(void){
    uint8_t originalEvaluationPermission = GManagedMemberCollection::setEvaluationPermission(PREVENTEVALUATION);
    GBasePopulation::optimize();
    GManagedMemberCollection::setEvaluationPermission(originalEvaluationPermission);
  }

  /********************************************************************/
  /**
   * An overloaded version of GBasePopulation::mutateChildren() . Mutation
   * and evaluation of children is handled by threads in a thread pool. The maximum
   * number of threads is DEFAULTBOOSTTHREADS (possibly 2) and can be overridden
   * with the GBoostThreadPopulation::setMaxThreads() function.
   */
  void GBoostThreadPopulation::mutateChildren(void){
    uint16_t np = getNParents();
    uint32_t generation = GBasePopulation::getGeneration();	
    vector<boost::shared_ptr<GMember> >::iterator it;

    // We first create a vector of GMemberCarriers and fill it with items
    vector<shared_ptr<GMemberCarrier> > carrierset;
    vector<shared_ptr<GMemberCarrier> >::iterator itc;
	
    // We start with the parents, if this is generation 0 
    if(generation == 0){
      for(it=this->begin(); it!=this->begin() + np; ++it){
	if((*it)->isDirty()){
	  carrierset.push_back(shared_ptr<GMemberCarrier>(new GMemberCarrier(*it, 
									     "evaluate", getId(), getGeneration(), true)));
	}
      }
    }
	
    // Next we fill up with child individuals
    for(it=this->begin() + np; it!=this->end(); ++it){
      carrierset.push_back(shared_ptr<GMemberCarrier>(new GMemberCarrier(*it,
									 "mutate", getId(), getGeneration(), false)));
    }
	
    // Now we submit the work tasks to the thread pool ...
    for(itc=carrierset.begin(); itc!=carrierset.end(); ++itc)
      tp_.schedule(boost::bind(&GMemberCarrier::process, itc->get()));
	
    // ... and wait for the pool to become empty
    tp_.wait();
  }

  /********************************************************************/
  /**
   * Creates a report about the inner state of this object
   * 
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of this object
   */
  string GBoostThreadPopulation::assembleReport(uint16_t indention ) const
  {
    ostringstream oss;
    oss << ws(indention) << "GBoostThreadPopulation : " << this << endl
	<< ws(indention) << "maxThreads_ = " << maxThreads_ << endl
	<< ws(indention) << "-----> Report from parent class GBasePopulation : " << endl
	<< GBasePopulation::assembleReport(indention+NINDENTION) << endl;

    return oss.str();
  }

  /********************************************************************/
  /** 
   * Sets the maximum number of threads for this population. Standard 
   * value is DEFAULTBOOSTTHREADS (possibly 2).
   * 
   * \param maxThreads The maximum number of allowed threads 
   */ 
  void GBoostThreadPopulation::setMaxThreads(uint8_t maxThreads){
    maxThreads_ = maxThreads;
    tp_.size_controller().resize(maxThreads_);
  }
	
  /********************************************************************/
  /** 
   * Retrieves the maximum number of threads
   * 
   * \return The maximum number of allowed threads
   */
  uint8_t GBoostThreadPopulation::getMaxThreads(void){
    return maxThreads_;
  }
	
  /********************************************************************/

} /* namespace GenEvA */
