/**
 * @file GDoubleBiGaussAdaptor.hpp
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

// Standard headers go here

// Boost headers go here

#ifndef GDOUBLEBIGAUSSADAPTOR_HPP_
#define GDOUBLEBIGAUSSADAPTOR_HPP_

// Geneva headers go here
#include "geneva/GFPBiGaussAdaptorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The GDoubleBiGaussAdaptor represents an adaptor used for the adaption of
 * double values through the addition of gaussian-distributed random numbers.
 * See the documentation of GNumGaussAdaptorT<T> for further information on adaptors
 * in the Geneva context. This class is at the core of evolutionary strategies,
 * as implemented by this library. It is now implemented through a generic
 * base class that can also be used to adapt other numeric types.
 */
class GDoubleBiGaussAdaptor
	:public GFPBiGaussAdaptorT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	G_API void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar
	  & make_nvp("GFPBiGaussAdaptorT_double", boost::serialization::base_object<GFPBiGaussAdaptorT<double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	G_API GDoubleBiGaussAdaptor();
	/** @brief The copy constructor */
	G_API GDoubleBiGaussAdaptor(const GDoubleBiGaussAdaptor&);
	/** @brief Initialization with a adaption probability */
	explicit G_API GDoubleBiGaussAdaptor(const double&);
	/** @brief The destructor */
	virtual G_API ~GDoubleBiGaussAdaptor();

	/** @brief A standard assignment operator */
	G_API const GDoubleBiGaussAdaptor& operator=(const GDoubleBiGaussAdaptor&);

	/** @brief Checks for equality with another GDoubleBiGaussAdaptor object */
	G_API bool operator==(const GDoubleBiGaussAdaptor&) const;
	/** @brief Checks for inequality with another GDoubleBiGaussAdaptor object */
	G_API bool operator!=(const GDoubleBiGaussAdaptor&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual G_API boost::optional<std::string> checkRelationshipWith(
      const GObject&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const OVERRIDE;

	/** @brief Retrieves the id of this adaptor */
	virtual G_API Gem::Geneva::adaptorId getAdaptorId() const OVERRIDE;

   /** @brief Emits a name for this class / object */
   virtual G_API std::string name() const OVERRIDE;

protected:
	/** @brief Loads the data of another GObject */
	virtual G_API void load_(const GObject*) OVERRIDE;
	/** @brief Creates a deep clone of this object. */
	virtual G_API GObject* clone_() const OVERRIDE;

public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual G_API bool modify_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual  G_API void specificTestsNoFailureExpected_GUnitTests() OVERRIDE;
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual G_API void specificTestsFailuresExpected_GUnitTests() OVERRIDE;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GDoubleBiGaussAdaptor)

#endif /* GDOUBLEBIGAUSSADAPTOR_HPP_ */
