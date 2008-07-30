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

#include "GManagedMemberCollection.hpp"

/** 
 * Included in the implementation file so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers 
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GManagedMemberCollection)


namespace GenEvA
{

  /******************************************************************************************/
  /**
   * There is currently no local data, so all the default constructor needs
   * to do is to initialise its parent class GMutable<boost::shared_prt<GMember> > .
   */
  GManagedMemberCollection::GManagedMemberCollection()
    :GMutable<boost::shared_ptr<GMember> >(),
     vector<boost::shared_ptr<GMember> >()
  { /* nothing */ }

  /******************************************************************************************/
  /**
   * The standard copy constructor. We cannot rely on the vector's copy constructor, as
   * we store shared_ptr<GMember> objects in this class. Copying them would keep the 
   * pointer to the same GMember object. Instead we need to clone the members stored
   * in the shared_ptr objects.
   *
   * \param cp A constant reference to another GManagedMemberCOllection object
   */
  GManagedMemberCollection::GManagedMemberCollection(const GManagedMemberCollection& cp)
    :GMutable<boost::shared_ptr<GMember> >(cp)
  {
    vector<boost::shared_ptr<GMember> >::const_iterator it;
    for(it=cp.begin(); it!=cp.end(); ++it){
      GMember *member = dynamic_cast<GMember *>((*it)->clone());
      if(member){			
	boost::shared_ptr<GMember> p(member);
	this->push_back(p);
      }
      else{ // We are in a constructor, hence we do not throw an exception
	GLogStreamer gls;
	gls << "In GManagedMemberCollection::GManagedMemberCollection(const GManagedMemberCollection&):" << endl
	    << "Conversion Error!"
	    << logLevel(CRITICAL);
      }
    }
  }

  /******************************************************************************************/
  /**
   * Manually clears the vector so deletion of members takes place at a defined place in the
   * code. The destructor of GMutable<boost::shared_ptr<GMember> > will be called 
   * automatically, if no pointers to its members remain somewhere else.
   */
  GManagedMemberCollection::~GManagedMemberCollection(){
    vector<shared_ptr<GMember> >::clear();
  }

  /*******************************************************************************************/
  /** 
   * A standard assignment operator for GManagedMemberCollection objects,
   * 
   * \param cp A constant reference to another GManagedMemberCollection object
   * \return A constant reference to this object
   */
  const GManagedMemberCollection& GManagedMemberCollection::operator=(const GManagedMemberCollection& cp){
    GManagedMemberCollection::load(&cp);
    return *this;
  }
	
  /*******************************************************************************************/	
  /** 
   * A standard assignment operator for GManagedMemberCollection objects,
   * 
   * \param cp A copy of another GManagedMemberCollection object, camouflaged as a GObject
   * \return A constant reference to this object, camouflaged as a GObject
   */	
  const GObject& GManagedMemberCollection::operator=(const GObject& cp){
    GManagedMemberCollection::load(&cp);
    return *this;	
  }


  /******************************************************************************************/
  /**
   * This function removes all elements from the vector. As we use
   * smart pointers from the boost library, we do not have to take care
   * of deleting each element. After the call the class is in the same
   * condition as after the initial creation.
   */
  void GManagedMemberCollection::reset(void){
    vector<shared_ptr<GMember> >::clear();
    GMutable<boost::shared_ptr<GMember> >::reset();
  }

  /******************************************************************************************/
  /**
   * We reset ourself to the condition present during initialisation with
   * the default constructor. Then we load the data contained in another
   * GManagedMemberCollection object. As we do not have local data, we simply
   * pass the pointer to our parent class, which loads all the components
   * stored in the container of GObject *gm .
   *
   * \param gb Another GManagedMemberCollection object, camouflaged as a GObject
   */
  void GManagedMemberCollection::load(const GObject * gm){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GManagedMemberCollection *>(gm) == this){
      GException ge;
      ge << "In GManagedMemberCollection::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    // Load the GMutable<boost::shared_ptr<GMember> > data
    GMutable<boost::shared_ptr<GMember> >::load(gm);

    // convert the gm pointer to a GManagedMemberCollection
    const GManagedMemberCollection *gmmc = dynamic_cast<const GManagedMemberCollection *>(gm);

    // ... and load the data contained in the array
    if(gmmc){
      // check the sizes of the two containers
      uint16_t size_f=gmmc->size(), size_l=this->size();

      vector<boost::shared_ptr<GMember> >::iterator it_l;
      vector<boost::shared_ptr<GMember> >::const_iterator it_f;

      // same size or smaller?
      if(size_l <= size_f){
	// We first copy the data in the same positions
	for(it_l=this->begin(), it_f=gmmc->begin(); it_l!=this->end(); ++it_l, ++it_f){
	  // Are the types the same ? This is a polymorphic environment ...
	  if(typeid(*(it_l->get())) == typeid(*(it_f->get()))){
	    (*it_l)->load(it_f->get());
	  }
	  // we need to assign a new shared_ptr. The old one will
	  // take care of the destruction of the old pointer (?)				
	  else{
	    GMember *gmp = dynamic_cast<GMember *>((*it_f)->clone());
	    if(!gmp){
	      GException ge;
	      ge << "In GManagedMemberCollection::load() (1.): Error!" << endl
		 << "Invalid GMember conversion!" << endl << raiseException;
	    }
        			
	    shared_ptr<GMember> p(gmp);
	    *it_l = p;
	  }
	}
		    
	// Now copy any remaining data
	for(it_f=gmmc->begin()+size_l; it_f!=gmmc->end(); ++it_f){
	  GMember *gmp = dynamic_cast<GMember *>((*it_f)->clone());
	  if(!gmp){
	    GException ge;
	    ge << "In GManagedMemberCollection::load() (2.): Error!" << endl
	       << "Invalid GMember conversion!" << endl << raiseException;
	  }
	  shared_ptr<GMember> p(gmp);
	  this->push_back(p);
	}
      }
      else{ // size_f < size_l
	// First copy the items in the known positions
	for(it_l=this->begin(), it_f=gmmc->begin(); it_f!=gmmc->end(); ++it_l, ++it_f){
	  // Are the types the same ? This is a polymorphic environment ...
	  if(typeid(*(it_l->get())) == typeid(*(it_f->get()))){
	    (*it_l)->load(it_f->get());
	  }
	  // we need to assign a new shared_ptr. The old one will
	  // take care of the destruction of the old pointer (?)				
	  else{
	    GMember *gmp = dynamic_cast<GMember *>((*it_f)->clone());
	    if(!gmp){
	      GException ge;
	      ge << "In GManagedMemberCollection::load() (3.): Error!" << endl
		 << "Invalid GMember conversion!" << endl << raiseException;
	    }
        			
	    shared_ptr<GMember> p(gmp);
	    *it_l = p;
	  }
	}
			
	// We need to delete some elements. This command will drop the items at the end of
	// the vector.
	this->resize(size_f); 		
      }
    }
    else{
      GException ge;
      ge << "In GManagedMemberCollection<T>::load() (4.): Error!" << endl
	 << "Invalid GManagedMemberCollection conversion!" << endl << raiseException;
    }
  }


  /******************************************************************************************/
  /**
   * We are dealing with a container of GMember objects, each equipped
   * with its own <em>dirty flag</em>. Quite possibly, these objects are
   * themselves containers of GMember objects. Hence we want a function, which recursively 
   * sets the dirty flag for all members contained in this object. This is achieved by using 
   * the setDirtyFlagAll() function. For standard classes derived from the GMember class, 
   * the setDirtyFlagAll() function defaults to GMember::setDirtyFlag. Collections of members 
   * are assumed to be derived from the GManagedMemberCollection class. For these classes we 
   * overload the setDirtyFlagAll() function and call the same function for all members
   * contained in this container.
   *
   * Please note: In some cases marking all objects as dirty does not make sense,
   * as it triggers the recalculations of all GMember objects fitnesses stored in
   * this container. One example are populations. In these cases you need to
   * provide your own setDirtyFlagAll() function.
   */
  void GManagedMemberCollection::setDirtyFlagAll(void){
    // let's first reset our own dirty flag
    GMember::setDirtyFlag();

    // now let's recursively set the dirty flag for
    // all members contained in this object. If they
    // do not represent containers, setDirtyFlagAll()
    // simply defaults to the local setDirtyFlag() function.
    for(vector<shared_ptr<GMember> >::iterator it=this->begin(); it!=this->end(); ++it)
      (*it)->setDirtyFlagAll();
  }

  /******************************************************************************************/
  /**
   * This is a convenience function, so users do not need to deal with
   * smart pointers directly, when they add a member to the collection.
   * Note that pointers given to this function become the property of this
   * class. Users may not call the delete function on them and need
   * to ensure that the pointer remains valid. Usually this can only
   * be done by constructing the object with "new".
   *
   * Check: We need to do an error check here whether the GMember indeed has
   * the correct type ...
   *
   * \param gmptr A member that should be added to this collection
   */
  void GManagedMemberCollection::appendMember(GMember *gmptr){
    shared_ptr<GMember> p(gmptr);
    this->push_back(p);
  }

  /******************************************************************************************/
  /**
   * Provided here so we are somewhat independent of vector. At a later time
   * we might replace vector with our own class.
   *
   * \return The size of the collection
   */
  uint16_t GManagedMemberCollection::getSize(void) const{
    return this->size();
  }

  /******************************************************************************************/
  /**
   * This function uses adaptors to mutate each member in sequence
   */
  void GManagedMemberCollection::customMutate(void){
    GMutable<shared_ptr<GMember> >::applyAllAdaptors(*this);
  }

  /******************************************************************************************/
  /**
   * Assembles a report about the inner state of the object. Recursively calls
   * the same function for all GMember objects stored in this class.
   *
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GManagedMemberCollection::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GManagedMemberCollection: " << this << endl;
	
    uint16_t counter=0;
    vector<boost::shared_ptr<GMember> >::const_iterator it;
    for(it=this->begin(); it!=this->end(); ++it){
      oss << ws(indention) << "-----> Report from item " << counter++ << " : " << endl
	  << (*it)->assembleReport(indention+NINDENTION) << endl;
    }

    return oss.str();
  }

  /******************************************************************************************/
  /**
   * This function recursively sets the _evaluationPermission parameter in all contained members.
   * 
   * \param ep The new value of the _evaluationPermission parameter
   */
  uint8_t GManagedMemberCollection::setEvaluationPermission(uint8_t ep){
    // First set the parameter for ourselves
    uint8_t originalValue = GMember::setEvaluationPermission(ep);
    // Then for all members stored in this class
    vector<boost::shared_ptr<GMember> >::iterator it;
    for(it=this->begin(); it!=this->end(); ++it){
      (*it)->setEvaluationPermission(ep);
    }

    return originalValue;
  }

  /******************************************************************************************/
  /**
   * Recursively sets the GMember::_isRoot parameter for all GMember objects
   * contained in this object to false. This is an overloaded version of the
   * GMember::setIsNotRoot() function,
   */
  void GManagedMemberCollection::setIsNotRoot(void){
    // first we care for ourself
    GMember::setIsNotRoot();
    // And then for all members contained in this object. 
    GManagedMemberCollection::setIsNotRootExcl();
  }

  /******************************************************************************************/
  /**
   * Recursively sets the GMember::_isRoot parameter for all GMember objects
   * contained in this object to false. In contrast to GManagedMemberCollection::setIsNotRoot(),
   * this function ignores the object for which it is called.
   */
  void GManagedMemberCollection::setIsNotRootExcl(void){
    vector<boost::shared_ptr<GMember> >::iterator it;
    for(it=this->begin(); it!=this->end(); ++it){
      (*it)->setIsNotRoot();
    }
  }

  /******************************************************************************************/

} /* namespace GenEvA */
