/**
 * @file GBooleanAdaptor.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard headers go here

// Boost headers go here

#ifndef GBOOLEANADAPTOR_HPP_
#define GBOOLEANADAPTOR_HPP_

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "GAdaptorT.hpp"
#include "GConstrainedDoubleObject.hpp"
#include "GIntFlipAdaptorT.hpp"
#include "GObject.hpp"
#include "GObjectExpectationChecksT.hpp"
#include "GOptimizationEnums.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GBooleanAdaptor represents an adaptor used for the adaption of
 * boolean values by flipping its value. See the documentation of GAdaptorT<T> for
 * further information on adaptors in the Geneva context. Most functionality
 * (with the notable exception of the actual adaption logic) is currently
 * implemented in the GIntFlipAdaptorT class. Most of the logic is implemented
 * in the base classes, in particular customAdaptions();
 */
class GBooleanAdaptor
	:public GIntFlipAdaptorT<bool>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GIntFlipAdaptorT_bool",
	        boost::serialization::base_object<GIntFlipAdaptorT<bool> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GBooleanAdaptor();
	/** @brief The copy constructor */
	GBooleanAdaptor(const GBooleanAdaptor&);
	/** @brief Initialization with a adaption probability */
	explicit GBooleanAdaptor(const double&);

	/** @brief The destructor */
	virtual ~GBooleanAdaptor();

	/** @brief A standard assignment operator */
	const GBooleanAdaptor& operator=(const GBooleanAdaptor&);

	/** @brief Checks for equality with another GBooleanAdaptor object */
	bool operator==(const GBooleanAdaptor&) const;
	/** @brief Checks for inequality with another GBooleanAdaptor object */
	bool operator!=(const GBooleanAdaptor&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	/** @brief Retrieves the id of this adaptor */
	virtual Gem::Geneva::adaptorId getAdaptorId() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object. */
	virtual GObject* clone_() const OVERRIDE;

	/** The actual adaption logic */
	virtual void customAdaptions(bool&, const bool&) OVERRIDE;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GBooleanAdaptor)

#endif /* GBOOLEANADAPTOR_HPP_ */
