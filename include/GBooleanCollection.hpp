/**
 * @file GBooleanCollection.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>

#ifndef GBOOLEANCOLLECTION_HPP_
#define GBOOLEANCOLLECTION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here
#include "GParameterCollectionT.hpp"
#include "GEnums.hpp"
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
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & make_nvp("GParameterCollectionT_bool", boost::serialization::base_object<GParameterCollectionT<bool> >(*this));
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

	/** @brief Checks for equality with another GBooleanCollection object */
	bool operator==(const GBooleanCollection&) const;
	/** @brief Checks for inequality with another GBooleanCollection object */
	bool operator!=(const GBooleanCollection&) const;
	/** @brief Checks for equality with another GBooleanCollection object */
	virtual bool isEqualTo(const GObject&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GBooleanCollection object */
	virtual bool isSimilarTo(const GObject&, const double&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;


    /** @brief Loads the data of another GBooleanCollection class */
    virtual void load(const GObject *);

    /** @brief Adds random bits to the collection */
    void addRandomData(const std::size_t&);
    /** @brief Adds random bits to the collection with a given probability structure */
    void addRandomData(const std::size_t&, const double&);

  protected:
    /** @brief Creates a deep copy of this object */
    virtual GObject *clone_() const;
  };

  /**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOLEANCOLLECTION_HPP_ */
