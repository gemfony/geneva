/**
 * @file GBooleanCollection.cpp
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

#include "GBooleanCollection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBooleanCollection)

namespace Gem
{
namespace GenEvA
{
  /**********************************************************************/
  /**
   * The standard constructor. As we have no local data, all work is done
   * by the parent class.
   */
  GBooleanCollection::GBooleanCollection()
	:GParameterCollectionT<bool>()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * Initializes the class with a set of nval random bits.
   *
   * @param nval The size of the collection
   */
  GBooleanCollection::GBooleanCollection(const std::size_t& nval)
    :GParameterCollectionT<bool>()
  {
    for(std::size_t i= 0; i<nval; i++) this->push_back(gr.boolRandom());
  }

  /**********************************************************************/
  /**
   * Initializes the class with nval random bits, of which probability percent
   * have the value Gem::GenEvA::G_TRUE
   *
   * @param nval The size of the collection
   * @param probability The probability for G_TRUE values in the collection
   */
  GBooleanCollection::GBooleanCollection(const std::size_t& nval, const double& probability)
	:GParameterCollectionT<bool>()
  {
	for(std::size_t i= 0; i<nval; i++) this->push_back(gr.boolRandom(probability));
  }

  /**********************************************************************/
  /**
   * No local data, hence we can rely on the parent class.
   *
   * @param cp A copy of another GBooleanCollection object
   */
  GBooleanCollection::GBooleanCollection(const GBooleanCollection& cp)
    :GParameterCollectionT<bool>(cp)
  { /* nothing */ }

  /**********************************************************************/
  /**
   * The standard destructor. No local data, hence it is empty.
   */
  GBooleanCollection::~GBooleanCollection()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * A standard assignment operator for  GBooleanCollection objects.
   *
   * @param cp A copy of another GBooleanCollection object
   */
  const GBooleanCollection& GBooleanCollection::operator=(const GBooleanCollection& cp){
    GBooleanCollection::load(&cp);
    return *this;
  }

  /**********************************************************************/
  /**
   * Creates a deep clone of this object
   *
   * @return A deep clone of this object
   */
  GObject *GBooleanCollection::clone(){
	  return new GBooleanCollection(*this);
  }

  /**********************************************************************/
  /**
   * Loads the data of another GBooleanCollection object, camouflaged as
   * a GObject.
   *
   * @param gb A pointer to another GBooleanCollection object, camouflaged as a GObject
   */
  void GBooleanCollection::load(const GObject * cp){
    // Check that this object is not accidently assigned to itself.
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GBooleanCollection *>(cp) == this){
		std::ostringstream error;
		error << "In GBooleanCollection::load() : Error!" << std::endl
			  << "Tried to assign an object to itself." << std::endl;

		throw geneva_error_condition(error.str());
    }

    GParameterCollectionT<bool>::load(cp);
  }

  /**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
