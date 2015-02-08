/**
 * @file GParameterObjectCollection.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here

// Boost header files go here
#include <boost/cast.hpp>

#ifndef GPARAMETROBJECTCOLLECTION_HPP_
#define GPARAMETROBJECTCOLLECTION_HPP_

// Geneva header files go here
#include "geneva/GParameterBase.hpp"
#include "geneva/GParameterTCollectionT.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GBooleanObject.hpp"
#include "geneva/GInt32Object.hpp"
#include "geneva/GDoubleObject.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of GParameterBase objects, ready for use in a
 * GParameterSet derivative.
 */
class GParameterObjectCollection
	:public GParameterTCollectionT<GParameterBase>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GParameterTCollectionT_gbd",
			  boost::serialization::base_object<GParameterTCollectionT<GParameterBase> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GParameterObjectCollection();
	/** @brief Initialization with a number of GParameterBase objects */
	GParameterObjectCollection(const std::size_t&, boost::shared_ptr<GParameterBase>);
	/** @brief The copy constructor */
	GParameterObjectCollection(const GParameterObjectCollection&);
	/** @brief The destructor */
	virtual ~GParameterObjectCollection();

	/** @brief A standard assignment operator */
	const GParameterObjectCollection& operator=(const GParameterObjectCollection&);

	/** @brief Checks for equality with another GParameterObjectCollection object */
	bool operator==(const GParameterObjectCollection&) const;
	/** @brief Checks for inequality with another GParameterObjectCollection object */
	bool operator!=(const GParameterObjectCollection&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
	      const GObject&
	      , const Gem::Common::expectation&
	      , const double&
	      , const std::string&
	      , const std::string&
	      , const bool&
	 ) const OVERRIDE;

	/** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
	boost::shared_ptr<Gem::Geneva::GParameterBase> at(const std::size_t& pos);

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

	/***************************************************************************/
	/**
	 * This function returns a parameter item at a given position of the data set.
     * Note that this function will only be accessible to the compiler if parameter_type
     * is a derivative of GParameterBase, thanks to the magic of Boost's enable_if
     * and Type Traits libraries.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GParameterBase object, as required by the user
	 */
	template <typename parameter_type>
	const boost::shared_ptr<parameter_type> at(
			  const std::size_t& pos
			, typename boost::enable_if<boost::is_base_of<GParameterBase, parameter_type> >::type* dummy = 0
	)  const {
#ifdef DEBUG
	   if(this->empty() || pos >= this->size()) {
	      glogger
	      << "In GParameterObjectCollection::at<>(): Error!" << std::endl
	      << "Tried to access position " << pos << " while size is " << this->size() << std::endl
	      << GEXCEPTION;

	      // Make the compiler happy
	      return boost::shared_ptr<parameter_type>();
 	   }
#endif /* DEBUG */

	   // Does error checks on the conversion internally
      return Gem::Common::convertSmartPointer<GParameterBase, parameter_type>(data.at(pos));
	}

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const OVERRIDE;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Fills the collection with GParameterBase objects */
	void fillWithObjects();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterObjectCollection)

#endif /* GPARAMETROBJECTCOLLECTION_HPP_ */
