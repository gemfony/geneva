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
    this->addRandomData(nval);
  }

  /**********************************************************************/
  /**
   * Initializes the class with nval random bits, of which probability percent
   * have the value true
   *
   * @param nval The size of the collection
   * @param probability The probability for true values in the collection
   */
  GBooleanCollection::GBooleanCollection(const std::size_t& nval, const double& probability)
	:GParameterCollectionT<bool>()
  {
	this->addRandomData(nval, probability);
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
  GObject *GBooleanCollection::clone() const{
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
  /**
   * Adds random bits to the collection, 50% of which have the value false.
   *
   * @param nval The number of boolean values to add to the collection
   */
  void GBooleanCollection::addRandomData(const std::size_t& nval){
	  for(std::size_t i= 0; i<nval; i++) this->push_back(gr.boolRandom());
  }

  /**********************************************************************/
  /**
   * Adds random bits to the collection with a given probability structure.
   *
   * @param nval The number of boolean values to add to the collection
   * @param probability The probability for true values in the collection
   */
  void GBooleanCollection::addRandomData(const std::size_t& nval, const double& probability){
	  for(std::size_t i= 0; i<nval; i++) this->push_back(gr.boolRandom(probability));
  }

  /**********************************************************************/
  /**
   * Checks for equality with another GBooleanCollection object
   *
   * @param  cp A constant reference to another GBooleanCollection object
   * @return A boolean indicating whether both objects are equal
   */
  bool GBooleanCollection::operator==(const GBooleanCollection& cp) const {
	  return GBooleanCollection::isEqualTo(cp, boost::logic::indeterminate);
  }

  /**********************************************************************/
  /**
   * Checks for inequality with another GBooleanCollection object
   *
   * @param  cp A constant reference to another GBooleanCollection object
   * @return A boolean indicating whether both objects are inequal
   */
  bool GBooleanCollection::operator!=(const GBooleanCollection& cp) const {
	  return !this->isEqualTo(cp, boost::logic::indeterminate);
  }

  /**********************************************************************/
  /**
   * Checks for equality with another GBooleanCollection object. As we have
   * no local data, we just check for equality of the parent class'es object.
   *
   * @param  cp A constant reference to another GBooleanCollection object
   * @return A boolean indicating whether both objects are equal
   */
  bool GBooleanCollection::isEqualTo(const GObject& cp, const boost::logic::tribool& expected) const {
	  using namespace Gem::Util;

	  // Check that we are indeed dealing with a GBooleanCollection reference
	  const GBooleanCollection *gbc_load = GObject::conversion_cast(&cp,  this);

	  // Check our paren class'es data
	  if(!Gem::GenEvA::GParameterCollectionT<bool>::isEqualTo(*gbc_load, expected)) return false;

	  // No local data, so we can leave
	  return true;
  }

  /**********************************************************************/
  /**
   * Checks for similarity with another GBooleanCollection object.  As we have
   * no local data, we just check for similarity of the parent class'es object.
   *
   * @param  cp A constant reference to another GBooleanCollection object
   * @param limit A double value specifying the acceptable level of differences of floating point values
   * @return A boolean indicating whether both objects are similar to each other
   */
  bool GBooleanCollection::isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected) const {
	  using namespace Gem::Util;

	  // Check that we are indeed dealing with a GBooleanCollection reference
	  const GBooleanCollection *gbc_load = GObject::conversion_cast(&cp,  this);

	  // Check our paren class'es data
	  if(!Gem::GenEvA::GParameterCollectionT<bool>::isSimilarTo(*gbc_load, limit, expected)) return false;

	  // No local data, so we can leave
	  return true;
  }

  /**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
