/**
 * @file GBitCollection.cpp
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

#include "GBitCollection.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBitCollection)

namespace Gem
{
namespace GenEvA
{
  /**********************************************************************/
  /**
   * The standard constructor. As we have no local data, all work is done
   * by the parent class.
   */
  GBitCollection::GBitCollection()
	:GParameterCollectionT<Gem::GenEvA::bit>()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * Initializes the class with a set of nval random bits.
   *
   * @param nval The size of the collection
   */
  GBitCollection::GBitCollection(const std::size_t& nval)
    :GParameterCollectionT<Gem::GenEvA::bit>()
  {
    for(std::size_t i= 0; i<nval; i++) data.push_back(gr.bitRandom());
  }

  /**********************************************************************/
  /**
   * Initializes the class with nval random bits, of which probability percent
   * have the value Gem::GenEvA::G_TRUE
   *
   * @param nval The size of the collection
   * @param probability The probability for G_TRUE values in the collection
   */
  GBitCollection::GBitCollection(const std::size_t& nval, const double& probability)
	:GParameterCollectionT<Gem::GenEvA::bit>()
  {
	for(std::size_t i= 0; i<nval; i++) data.push_back(gr.bitRandom(probability));
  }

  /**********************************************************************/
  /**
   * No local data, hence we can rely on the parent class.
   *
   * @param cp A copy of another GBitCollection object
   */
  GBitCollection::GBitCollection(const GBitCollection& cp)
    :GParameterCollectionT<Gem::GenEvA::bit>(cp)
  { /* nothing */ }

  /**********************************************************************/
  /**
   * The standard destructor. No local data, hence it is empty.
   */
  GBitCollection::~GBitCollection()
  { /* nothing */ }

  /**********************************************************************/
  /**
   * A standard assignment operator for  GBitCollection objects.
   *
   * @param cp A copy of another GBitCollection object
   */
  const GBitCollection& GBitCollection::operator=(const GBitCollection& cp){
    GBitCollection::load(&cp);
    return *this;
  }

  /**********************************************************************/
  /**
   * Creates a deep clone of this object
   *
   * @return A deep clone of this object
   */
  GObject *GBitCollection::clone(){
	  return new GBitCollection(*this);
  }

  /**********************************************************************/
  /**
   * Loads the data of another GBitCollection object, camouflaged as
   * a GObject.
   *
   * @param gb A pointer to another GBitCollection object, camouflaged as a GObject
   */
  void GBitCollection::load(const GObject * cp){
    // Check that this object is not accidently assigned to itself.
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GBitCollection *>(cp) == this){
		std::ostringstream error;
		error << "In GBitCollection::load() : Error!" << std::endl
			  << "Tried to assign an object to itself." << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		throw geneva_error_condition() << error_string(error.str());
    }

    GParameterCollectionT<Gem::GenEvA::bit>::load(cp);
  }

  /**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
