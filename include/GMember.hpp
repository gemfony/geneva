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

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GMEMBER_H_
#define GMEMBER_H_

using namespace std;

#include "GRandom.hpp"
#include "GObject.hpp"

namespace GenEvA
{

  /** 
   * These values are needed to determine whether customFitness() should be prevented
   * from running, may run or should be executed immediately.
   */
  const uint8_t PREVENTEVALUATION = 0;
  const uint8_t ALLOWEVALUATION = 1;
  const uint8_t ENFORCEEVALUATION = 2;

  /*******************************************************************************************/
  /**
   * GMember is the parent class for all those classes that can be part of
   * a population or an individual. Major characteristics of a GMember or its
   * derived classes include the ability to have a fitness (used for quality
   * assessment and sorting) and to be mutated.
   */
  class GMember
    :public GObject
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GObject", boost::serialization::base_object<GObject>(*this));
      ar & make_nvp("_parentPopGeneration",_parentPopGeneration);
      ar & make_nvp("_myCurrentFitness",_myCurrentFitness);
      ar & make_nvp("_dirtyFlag",_dirtyFlag);
      ar & make_nvp("_isparent", _isparent);
      ar & make_nvp("_evaluationPermission", _evaluationPermission);
      ar & make_nvp("_isRoot", _isRoot);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief The default constructor */
    GMember(void) throw();
    /** \brief A standard copy constructor */
    GMember(const GMember& cp) throw();
    /** \brief The destructor */
    virtual ~GMember();

    /** \brief Resets the class to its initial state */
    virtual void reset(void);
    /** \brief Loads the data of another GMember object */
    virtual void load(const GObject *gmi);
    /** \brief Creates a clone of this GMember object */
    virtual GObject *clone(void)=0;

    /** \brief A standard assignment operator */
    const GMember& operator=(const GMember&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);

    /** \brief Returns the value of this object for quality assessment */
    double fitness(void) throw();
    /** \brief Mutates this object in order to achieve a different, hopefully better quality */
    void mutate(void) throw();

    /** \brief "true" if the object has been mutated (and not yet evaluated)
	or the <em>dirty flag</em> has been set manually */
    bool isDirty(void) const throw();
    /** \brief Get the state of the <em>dirty flag</em> */
    bool getDirtyFlag(void) const throw();
    /** \brief Set the dirty flag to <em>true</em>*/
    virtual void setDirtyFlag(void);
    /** \brief Reset all members stored in this object. Must be overloaded in dervied classes */
    virtual void setDirtyFlagAll(void);

    /** \brief Returns the internal value of this object. */
    double getMyCurrentFitness() const throw();

    /** \brief Informs this object about the generation it is in */
    void setParentPopGeneration(uint32_t parentPopGeneration) throw();
    /** \brief Retrieves information about the current generation of our population */
    uint32_t getParentPopGeneration(void) const throw();

    /** \brief Checks whether we are a parent or a child */
    bool isParent(void) const throw();
    /** \brief Marks us as a parent or child */
    void setIsParent(bool parent) throw();

    /** \brief Sets the _evaluationPermission parameter to a given value */
    virtual uint8_t setEvaluationPermission(uint8_t);
    /** \brief Retrieves the _evaluationPermission parameter */
    bool getEvaluationPermission(void) const throw();
	
    // Should be put in GBasePopulation or GManagedMemberCollection ? 
    // Doesn't e.g. make sense for a GTemplateValueCollection
    /** \brief Specifies whether this GMember is at the root of the hierarchy */ 
    virtual void setIsNotRoot(void);
    /** \brief Retrieves the _isRoot parameter */
    bool isRoot(void) const throw();
	
    /** \brief Emit information about this class */
    virtual string assembleReport(uint16_t indention = 0) const;

  protected:
    /** \brief User-specified value-calculation of this object. Used in GMember::fitness() */
    virtual double customFitness(void)=0;
    /** \brief User-specified mutation scheme for this object. Used in GMember::mutate() */
    virtual void customMutate(void)=0;
	
  private:
    /** \brief Sets the <em>dirty flag</em> to any desired value */
    void setDirtyFlag(bool flag) throw();

    /** \brief Holds the parent population's current generation */
    uint32_t _parentPopGeneration;
    /** \brief Holds this object's internal fitness */
    double _myCurrentFitness;
    /** \brief Internal representation of the mutation status of this object */
    bool _dirtyFlag;
    /** \brief Steers whether evaluation is disallowed, allowed or needs to be enforced */
    uint8_t _evaluationPermission;
    /** \brief Allows populations to mark members as parents or children */
    bool _isparent;
    /** \brief Specifies whether this GMember is at the top of the hierarchy */
    bool _isRoot;
	
  }; /* class GMember */

  /*******************************************************************************************/

} /* namespace GenEvA */

/**************************************************************************************************/
/**
 * \brief Needed for Boost.Serialization
 */

#if BOOST_VERSION <= 103500

BOOST_IS_ABSTRACT(GenEvA::GMember)

#else

BOOST_SERIALIZATION_ASSUME_ABSTRACT(GenEvA::GMember)

#endif /* Serialization traits */

/**************************************************************************************************/

#endif /*GMEMBER_H_*/
