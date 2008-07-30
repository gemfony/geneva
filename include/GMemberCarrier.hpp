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
#include <iostream>

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
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/strong_typedef.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GMEMBERCARRIER_H_
#define GMEMBERCARRIER_H_

using namespace std;

#include "GMember.hpp"

namespace GenEvA
{

  /*********************************************************************/
  /**
   * This is a wrapper for GMember objects. It is used for communication
   * between populations and consumers.
   *
   * Note that it is not foreseen that objects of this class are copied,
   * once they have been created. Hence the class is derived from
   * boost::noncopyable.
   *
   * Note that we do not provide functions for assembly of reports for
   * this class. This is done for GObject-derivatives only.
   */
  class GMemberCarrier
    :boost::noncopyable
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("parent_",parent_);
      ar & make_nvp("generation_",generation_);
      ar & make_nvp("payload_", payload_);
      ar & make_nvp("command_", command_);
      ar & make_nvp("id_",id_);
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** \brief Only defined to satisfy serialization requirements */
    GMemberCarrier(void);
    /** \brief Initializes the object from a string */
    GMemberCarrier(const string&);
    /** \brief Sets all information about the object at once */
    GMemberCarrier(const shared_ptr<GMember>&, const string&, const string&, uint32_t, bool);
    /** \brief Sets all information except the id at once */
    GMemberCarrier(const shared_ptr<GMember>&, const string&, uint32_t, bool);
    /** \brief Standard destructor */
    ~GMemberCarrier();

    /** \brief Does the actual processing */
    void process(void);

    /** \brief Allows to find out whether the payload has no home anymore */
    bool orphaned(void);
    /** \brief Allows to check whether the GMember is a parent or not */
    bool isParent(void) const;
    /** \brief Retrieves information about the generation the payload belongs to */
    uint32_t getGeneration(void) const;

    /** \brief Retrieves the payload */
    shared_ptr<GMember> payload(void) const;
    /** \brief Retrieves the command */
    string command(void) const;

    /** \brief Sets the id associated with the payload */
    void setId(const string&);
    /** \brief Retrieves the id associated with the payload */
    const string& getId(void) const;

    /** \brief transform the carrier into a string */
    string toString(void);
    /** \brief Load content from a string */
    void fromString(const string&);

  private:
    GMemberCarrier(const GMemberCarrier&); ///< Intentionally left undefined
    const GMemberCarrier& operator=(const GMemberCarrier&); ///< Intentionally left undefined

    bool parent_; ///< Do we hold a parent object ?
    uint32_t generation_; ///< Which generation does the object belong to ?

    shared_ptr<GMember> payload_; ///< The actual payload
    string command_; ///< The command associated with the payload
    string id_; ///< An id assigned to the payload (usually referring to a population)
  };

  /*********************************************************************/

} /* namespace GenEvA */

#endif /* GMEMBERCARRIER_H_ */
