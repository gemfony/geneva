/**
 * @file GBooleanCollection.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

// Standard headers go here
#include <sstream>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GBOOLEANCOLLECTION_HPP_
#define GBOOLEANCOLLECTION_HPP_

// GenEvA headers go here

#include "GParameterCollectionT.hpp"
#include "GEnums.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem
{
namespace GenEvA
{

  /**********************************************************************/
  /**
   * This class represents collections of bits. They are usually mutated by
   * the GBooleanAdaptor, which has a mutable flip probability. One adaptor
   * is applied to all bits. If you want individual flip probabilities for
   * all bits, use GBool objects instead.
   */
  class GBooleanCollection
    :public GParameterCollectionT<bool>
  {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GParameterCollectionT_bool",
    		  boost::serialization::base_object<GParameterCollectionT<bool> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

  public:
    /** @brief The default constructor */
    GBooleanCollection();
    /** @brief Random initialization with a given number of values */
    explicit GBooleanCollection(const std::size_t&);
    /** @brief Random initialization with a given number of values of
     * a certain probability structure */
    GBooleanCollection(const std::size_t&, const double&);
    /** @brief A standard copy constructor */
    GBooleanCollection(const GBooleanCollection&);
    /** @brief The standard destructor */
    virtual ~GBooleanCollection();

    /** @brief A standard assignment operator */
    const GBooleanCollection& operator=(const GBooleanCollection&);

    /** @brief Creates a deep copy of this object */
    virtual GObject *clone();
    /** @brief Loads the data of another GBooleanCollection class */
    virtual void load(const GObject *);
  };

  /**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOLEANCOLLECTION_HPP_ */
