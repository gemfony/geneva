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
#include <boost/shared_ptr.hpp>

#ifndef GSIMPLEMEMBERADAPTOR_H_
#define GSIMPLEMEMBERADAPTOR_H_

using namespace std;
using namespace boost;

#include "GTemplateAdaptor.hpp"
#include "GMember.hpp"

namespace GenEvA
{

  typedef shared_ptr<GMember> GMember_ptr;

  /********************************************************************************************/
  /**
   * This class is designed to allow mutation in the context of the
   * GManagedMemberCollection class. Populations, such as the
   * GBasePopulation class, are the most well-known representatives of
   * the GManagedMemberCollection . Each member has the ability to be
   * mutated, using its mutate() function.  So the main purpose of
   * this class is to call the GMember::mutate() function. This design
   * has been chosen so GMember objects and other types of values
   * found in populations or individuals can be treated alike.
   */
  class GSimpleMemberAdaptor
    :public GTemplateAdaptor<GMember_ptr>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GTemplateAdaptor", boost::serialization::base_object<GTemplateAdaptor<GMember_ptr> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief Standard constructor. Every adaptor needs a name */
    explicit GSimpleMemberAdaptor(string name) throw();
    /** \brief A standard copy constructor */
    GSimpleMemberAdaptor(const GSimpleMemberAdaptor& cp) throw();
    /** \brief The standard destructor */
    virtual ~GSimpleMemberAdaptor();

    /** \brief A standard assignment operator */
    const GSimpleMemberAdaptor& operator=(const GSimpleMemberAdaptor&);
    /** \brief An assignment operator for GObject objects */
    virtual const GObject& operator=(const GObject&);

    /** \brief Resets the object to its initial state */
    virtual void reset(void);
    /** \brief Similar to a copy constructor */
    virtual void load(const GObject *gb);
    /** \brief Creates a deep copy of the object */
    virtual GObject *clone(void);

    /** \brief Reports about the inner state of the object */
    virtual string assembleReport(uint16_t indention = 0) const;

  protected:
    /** \brief The actual code for the mutation */
    virtual void customMutate(GMember_ptr &value);

  private:
    /** \brief The default constructor - we do not want it */
    GSimpleMemberAdaptor(void) throw();
  };

  /********************************************************************************************/

} /* namespace GenEvA */

#endif /*GSIMPLEMEMBERADAPTOR_H_*/
