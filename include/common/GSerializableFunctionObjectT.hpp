/**
 * @file GSerializableFunctionObjectT.hpp
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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>

#ifndef GENEVA_LIBRARY_COLLECTION_GSERIALIZABLEFUNCTIONOBJECT_HPP
#define GENEVA_LIBRARY_COLLECTION_GSERIALIZABLEFUNCTIONOBJECT_HPP

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class forms the basis of a hierarchy of function objects that are
 * required to be serializable, so they may be registered with serializable
 * objects and thus modify their behaviour.
 */
template <typename processable_type>
class GSerializableFunctionObjectT
	: public GCommonInterfaceT<GSerializableFunctionObjectT<processable_type>>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GCommonInterfaceT_GSerializableFunctionObjectT_T"
						, boost::serialization::base_object<GCommonInterfaceT<GSerializableFunctionObjectT<processable_type>>>(*this));
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor
	  */
	 GSerializableFunctionObjectT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GSerializableFunctionObjectT(const GSerializableFunctionObjectT<processable_type>& cp)
		 : GCommonInterfaceT<GSerializableFunctionObjectT<processable_type>>(cp)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
     */
	 virtual ~GSerializableFunctionObjectT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GSerializableFunctionObjectT<processable_type> object
	  *
	  * @param  cp A constant reference to another GSerializableFunctionObjectT<processable_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GSerializableFunctionObjectT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another GSerializableFunctionObjectT<processable_type> object
	  *
	  * @param  cp A constant reference to another GSerializableFunctionObjectT<processable_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GSerializableFunctionObjectT<processable_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Function call operator
  	  */
	 bool operator()(processable_type& p) {
		 return this->process_(p);
	 }

	 /***************************************************************************/
	 /**
	  * Returns the name of this class
	  */
	 std::string name() const override {
		 return std::string("GSerializableFunctionObjectT<processable_type>");
	 }

	 /***************************************************************************/
	 /**
	  * Checks for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GSerializableFunctionObjectT<processable_type> object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GSerializableFunctionObjectT<processable_type> &cp
		 , const Gem::Common::expectation &e
		 , const double &limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		 const GSerializableFunctionObjectT<processable_type> *p_load
			 = Gem::Common::g_convert_and_compare<GSerializableFunctionObjectT<processable_type>, GSerializableFunctionObjectT<processable_type>>(cp, this);

		 GToken token("GSerializableFunctionObjectT<processable_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GCommonInterfaceT<GSerializableFunctionObjectT<processable_type>>>(IDENTITY(*this, *p_load), token);

		 // ... no local data

		 // React on deviations from the expectation
		 token.evaluate();
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GSerializableFunctionObjectT<processable_type> object
	  */
	 void load_(const GSerializableFunctionObjectT<processable_type> *cp) override {
		 // Check that we are dealing with a GSerializableFunctionObjectT<processable_type> reference independent of this object and convert the pointer
		 const GSerializableFunctionObjectT<processable_type> *p_load = Gem::Common::g_convert_and_compare<GSerializableFunctionObjectT<processable_type>, GSerializableFunctionObjectT<processable_type>>(cp, this);

		 // ... no local data
	 }

	 /***************************************************************************/

	 /** @brief overload this function to make this class operational */
	 virtual G_API_COMMON bool process_(processable_type& p) BASE = 0;

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 G_API_COMMON GSerializableFunctionObjectT<processable_type> * clone_() const override = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
namespace serialization {
template<typename processable_type>
struct is_abstract<Gem::Common::GSerializableFunctionObjectT<processable_type>> : public boost::true_type {};
template<typename processable_type>
struct is_abstract< const Gem::Common::GSerializableFunctionObjectT<processable_type>> : public boost::true_type {};
} /* namespace serialization */
} /* namespace boost */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#endif /* GENEVA_LIBRARY_COLLECTION_GSERIALIZABLEFUNCTIONOBJECT_HPP */
