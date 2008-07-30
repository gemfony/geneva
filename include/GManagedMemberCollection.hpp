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

#include <typeinfo>
#include <vector>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>


#ifndef GMANAGEDMEMBERCOLLECTION_H_
#define GMANAGEDMEMBERCOLLECTION_H_

using namespace std;
using namespace boost;

#include "GMember.hpp"
#include "GMutable.hpp"


namespace GenEvA
{

  /******************************************************************************************/
  /**
   * This is the base class for populations as well as some individuals and chromosomes.
   * It can store objects of type boost::shared_ptr<GMember>. It is thus possible to
   * apply the standard GMember functions fitness() and mutate() functions for objects 
   * stored in this container. Note that this class takes responsibility of the values
   * stored in it. No interface is provided to the vector<T>(size_t, T) function, as
   * this would just create copies of the shared_ptr objects, so we would only have
   * one "real" pointer in the entire collection. 
   */
  class GManagedMemberCollection
    :public GenEvA::GMutable<boost::shared_ptr<GMember> >,
     public vector<boost::shared_ptr<GMember> >
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GMMCGMutable", boost::serialization::base_object<GenEvA::GMutable<boost::shared_ptr<GMember> > >(*this));
      ar & make_nvp("GMMCvector", boost::serialization::base_object<vector<boost::shared_ptr<GMember> > >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GManagedMemberCollection(void);
    /** \brief A standard copy constructor */
    GManagedMemberCollection(const GManagedMemberCollection& cp);
    /** \brief The destructor */
    virtual ~GManagedMemberCollection();

    /** \brief A standard assignment operator */
    const GManagedMemberCollection& operator=(const GManagedMemberCollection&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);

    /** \brief Creates a deep clone of this object */
    virtual GObject *clone(void)=0;
    /** \brief Resets the object to its initial state upon creation with the default constructor */
    virtual void reset(void);
    /** \brief Loads the data of another GManagedMemberCollection object */
    virtual void load(const GObject * gm);

    /** \brief Redefined version of GMember::setDirtyFlagAll() which
     * recursively sets the flag of members of this collection */
    virtual void setDirtyFlagAll(void);

    /** \brief Appends a new object derived from the GMember class. */
    void appendMember(GMember *gmptr);

    /** \brief Retrieve the size of this population */
    uint16_t getSize(void) const;
		
    /** \brief Recursively sets the evaluation permission for this class and all its members */
    virtual uint8_t setEvaluationPermission(uint8_t);
    
    /** \brief Recursively sets the GMember::_isRoot parameter to false */
    virtual void setIsNotRoot(void);

    /** \brief Assembles a report about the inner state of the object */
    virtual string assembleReport(uint16_t indention = 0) const;

  protected:
    /** \brief Mutates the members in sequence */
    virtual void customMutate(void);
    /** \brief Recursively sets the GMember::_isRoot parameter to false, ecluding this object */
    void setIsNotRootExcl(void);
  };

  /******************************************************************************************/

} /* namespace GenEvA */

/**************************************************************************************************/
/**
 * \brief Needed for Boost.Serialization
 */

#if BOOST_VERSION <= 103500

BOOST_IS_ABSTRACT(GenEvA::GManagedMemberCollection)

#else

BOOST_SERIALIZATION_ASSUME_ABSTRACT(GenEvA::GManagedMemberCollection)

#endif /* Serialization traits */

/**************************************************************************************************/

#endif /* GMANAGEDMEMBERCOLLECTION_H_ */
