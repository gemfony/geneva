/**
 * @file GSubmissionContainerT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/tuple/tuple.hpp>

#ifndef GSUBMISSIONCONTAINERBASE_HPP_
#define GSUBMISSIONCONTAINERBASE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GSerializeTupleT.hpp"
#include "courtier/GCourtierEnums.hpp"

namespace Gem {
namespace Courtier {

/**********************************************************************************************/
/**
 * This class can serve as a base class for items to be submitted through the broker. You need to
 * re-implement the purely virtual functions in derived classes. Note that it is mandatory for
 * derived classes to be serializable and to trigger serialization of this class.
 */
template <typename submission_type>
class GSubmissionContainerT {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & BOOST_SERIALIZATION_NVP(id_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/**********************************************************************************/
	/**
	 * The default constructor
	 */
	GSubmissionContainerT()
	{ /* nothing */ }

	/**********************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GSubmissionContainer object
	 */
	GSubmissionContainerT(
	      const GSubmissionContainerT<submission_type>& cp
	) : id_(cp.id_)
	{ /* nothing */ }


	/**********************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GSubmissionContainerT()
	{ /* nothing */ }

	/**********************************************************************************/
	/** @brief Allows derived classes to specify the tasks to be performed for this object */
	virtual bool process() = 0;

	/**********************************************************************************/
	/**
	 * Loads user-specified data. This function can be overloaded by derived classes. It
	 * is mainly intended to provide a mechanism to "deposit" an item at a remote site
	 * that holds otherwise constant data. That data then does not need to be serialized
	 * but can be loaded whenever a new work item arrives and has been de-serialized. Note
	 * that, if your individuals do not serialize important parts of an object, you need
	 * to make sure that constant data is loaded after reloading a checkpoint.
	 *
	 * @param cD_ptr A pointer to the object whose data should be loaded
	 */
	virtual void loadConstantData(boost::shared_ptr<submission_type>)
	{ /* nothing */ }

	/**********************************************************************************/
	/**
	 * Allows the courtier library to associate an id with the container
	 *
	 * @param id An id that allows the broker connector to identify this object
	 */
	void setCourtierId(const boost::tuple<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2>& id) {
		id_ = id;
	}

	/**********************************************************************************/
	/**
	 * Allows to retrieve the courtier-id associated with this container
	 *
	 * @return An id that allows the broker connector to identify this object
	 */
	boost::tuple<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2> getCourtierId() const {
		return id_;
	}

private:
	/**********************************************************************************/
    /** @brief A two-part id that can be assigned to this container object */
    boost::tuple<Gem::Courtier::ID_TYPE_1, Gem::Courtier::ID_TYPE_2> id_;
};

/**********************************************************************************************/

} /* namespace Courtier */
} /* namespace Gem */

/**************************************************************************************************/
/** @brief Mark this class as abstract. This is the content of
 * BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) */
namespace boost {
	namespace serialization {
		template<typename submission_type>
		struct is_abstract<Gem::Courtier::GSubmissionContainerT<submission_type> > : public boost::true_type {};
		template<typename submission_type>
		struct is_abstract< const Gem::Courtier::GSubmissionContainerT<submission_type> > : public boost::true_type {};
	}
}

/**************************************************************************************************/

#endif /* GSUBMISSIONCONTAINERBASE_HPP_ */
