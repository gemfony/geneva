/**
 * @file GParameterTCollectionT_test.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

// Boost header files go here

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#ifndef GPARAMETERTCOLLECTIONT_TEST_HPP_
#define GPARAMETERTCOLLECTIONT_TEST_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GParameterT.hpp"
#include "geneva/GParameterTCollectionT.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GInt32Object.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedInt32.hpp"
#include "geneva/GBooleanObject.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GBooleanObjectCollection.hpp"
#include "geneva/GInt32ObjectCollection.hpp"
#include "GStdVectorInterface_test.hpp"
#include "GEqualityPrinter.hpp"

using namespace Gem::Hap;
using namespace Gem::Geneva;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/***********************************************************************************/
// This template allows to create default entries for the collection
// This template allows to create items different of the default item.
// Note that these will not have an adaptor assigned to them and
// can thus not be adapted.
template <typename T>
boost::shared_ptr<typename T::collection_type> getTemplateItemNoAdaptor() {
	return boost::shared_ptr<typename T::collection_type>(new typename T::collection_type(0));
}

// Specialization for GConstrainedDoubleObject
template <>
boost::shared_ptr<GConstrainedDoubleObject> getTemplateItemNoAdaptor<GConstrainedDoubleObjectCollection>() {
	return boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(0.,0.,1.));
}

// Specialization for GConstrainedInt32
template <>
boost::shared_ptr<GConstrainedInt32> getTemplateItemNoAdaptor<GConstrainedInt32ObjectCollection>() {
	return boost::shared_ptr<GConstrainedInt32>(new GConstrainedInt32(0,0,100));
}

// This template allows to create items different of the default item.
// Note that these will not have an adaptor assigned to them and
// can thus not be adapted.
template <typename T>
boost::shared_ptr<typename T::collection_type> getFindItemNoAdaptor() {
	return boost::shared_ptr<typename T::collection_type>(new typename T::collection_type(1));
}

// Specialization for GConstrainedDoubleObject
template <>
boost::shared_ptr<GConstrainedDoubleObject> getFindItemNoAdaptor<GConstrainedDoubleObjectCollection>() {
	return boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(1.,0.,1.));
}

// Specialization for GConstrainedInt32
template <>
boost::shared_ptr<GConstrainedInt32> getFindItemNoAdaptor<GConstrainedInt32ObjectCollection>() {
	return boost::shared_ptr<GConstrainedInt32>(new GConstrainedInt32(1,0,100));
}

/***********************************************************************************/
// This template allows to create default entries for the collection, fully
// equipped with adaptors. As these are different for each type, this
// template should not be called directly. Instead, specializations should
// be used.
template <typename T>
boost::shared_ptr<typename T::collection_type> getTemplateItem() {
	// This function should never be called directly
	std::ostringstream error;
	error << "In getTemplateItem<T>(): The template itself should never be called!" << std::endl;
	throw(Gem::Common::gemfony_error_condition(error.str()));
}

// Specialization for GConstrainedDoubleObject
template <>
boost::shared_ptr<GConstrainedDoubleObject> getTemplateItem<GConstrainedDoubleObjectCollection>() {
	boost::shared_ptr<GConstrainedDoubleObject> gbd_ptr(new GConstrainedDoubleObject(0.,0.,1.));
	gbd_ptr->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor()));
	return gbd_ptr;
}

// Specialization for GConstrainedInt32
template <>
boost::shared_ptr<GConstrainedInt32> getTemplateItem<GConstrainedInt32ObjectCollection>() {
	boost::shared_ptr<GConstrainedInt32> gbi_ptr(new GConstrainedInt32(0,0,100));
	gbi_ptr->addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor()));
	return gbi_ptr;
}

// Specialization for GDoubleObject
template <>
boost::shared_ptr<GDoubleObject> getTemplateItem<GDoubleObjectCollection>() {
	boost::shared_ptr<GDoubleObject> gbd_ptr(new GDoubleObject(0.));
	gbd_ptr->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor()));
	return gbd_ptr;
}

// Specialization for GInt32Object
template <>
boost::shared_ptr<GInt32Object> getTemplateItem<GInt32ObjectCollection>() {
	boost::shared_ptr<GInt32Object> gint32_ptr(new GInt32Object(0));
	gint32_ptr->addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor()));
	return gint32_ptr;
}

// Specialization for GBooleanObject
template <>
boost::shared_ptr<GBooleanObject> getTemplateItem<GBooleanObjectCollection>() {
	boost::shared_ptr<GBooleanObject> gboolean_ptr(new GBooleanObject(false));
	gboolean_ptr->addAdaptor(boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor()));
	return gboolean_ptr;
}

// This template allows to create items different of the default item, , fully
// equipped with adaptors. As these are different for each type, this
// template should not be called directly. Instead, specializations should
// be used.
template <typename T>
boost::shared_ptr<typename T::collection_type> getFindItem() {
	// This function should never be called directly
	std::ostringstream error;
	error << "In getFindItem<T>(): The template itself should never be called!" << std::endl;
	throw(Gem::Common::gemfony_error_condition(error.str()));
}

// Specialization for GConstrainedDoubleObject
template <>
boost::shared_ptr<GConstrainedDoubleObject> getFindItem<GConstrainedDoubleObjectCollection>() {
	boost::shared_ptr<GConstrainedDoubleObject> gbd_ptr(new GConstrainedDoubleObject(1.,0.,1.));
	gbd_ptr->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor()));
	return gbd_ptr;
}

// Specialization for GConstrainedInt32
template <>
boost::shared_ptr<GConstrainedInt32> getFindItem<GConstrainedInt32ObjectCollection>() {
	boost::shared_ptr<GConstrainedInt32> gbi_ptr(new GConstrainedInt32(1,0,100));
	gbi_ptr->addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor()));
	return gbi_ptr;
}

// Specialization for GDoubleObject
template <>
boost::shared_ptr<GDoubleObject> getFindItem<GDoubleObjectCollection>() {
	boost::shared_ptr<GDoubleObject> gbd_ptr(new GDoubleObject(1.));
	gbd_ptr->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor()));
	return gbd_ptr;
}

// Specialization for GInt32Object
template <>
boost::shared_ptr<GInt32Object> getFindItem<GInt32ObjectCollection>() {
	boost::shared_ptr<GInt32Object> gint32_ptr(new GInt32Object(1));
	gint32_ptr->addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor()));
	return gint32_ptr;
}

// Specialization for GBooleanObject
template <>
boost::shared_ptr<GBooleanObject> getFindItem<GBooleanObjectCollection>() {
	boost::shared_ptr<GBooleanObject> gboolean_ptr(new GBooleanObject(true));
	gboolean_ptr->addAdaptor(boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor()));
	return gboolean_ptr;
}

/********************************************************************************************/
// The actual unit tests for this class

// Test features that are expected to work
BOOST_TEST_CASE_TEMPLATE_FUNCTION( GParameterTCollectionT_no_failure_expected, T )
{
	// Prepare printing of error messages in object comparisons
	GEqualityPrinter gep("GParameterTCollectionT_no_failure_expected",
						 pow(10,-10),
						 Gem::Common::CE_WITH_MESSAGES);

	// A local random number generator
	GRandomT<RANDOMLOCAL> gr;

	// Default construction
	T gptct;

	// Check the vector interface
	boost::shared_ptr<typename T::collection_type> templItem_ptr = getTemplateItem<T>();
	boost::shared_ptr<typename T::collection_type> findItem_ptr = getFindItem<T>();
	// Make sure both items are indeed different
	BOOST_CHECK(*templItem_ptr != *findItem_ptr);

	// Run the actual vector tests
	stdvectorinterfacetestSP(gptct, templItem_ptr, findItem_ptr);

	// vector functionality of the collection has now been thoroughly tested.
	// Collection items should be remaining in the object. Check.
	BOOST_CHECK(!gptct.empty());

	// Create two copies of the object
	T gptct_cp1, gptct_cp2, gptct_cp4;
	gptct_cp1 = gptct_cp2 = gptct_cp4 = gptct;

	// Check that they are indeed identical
	BOOST_CHECK(gptct_cp1 == gptct);
	BOOST_CHECK(gptct_cp2 == gptct);

	// Adapt the second copy and check that it has become different from the other two collections
	BOOST_CHECK_NO_THROW(gptct_cp2.adapt());
	BOOST_CHECK(gptct_cp2 != gptct);
	BOOST_CHECK(gptct_cp2 != gptct_cp1);

	// Assign for later usage
	gptct_cp4=gptct_cp2;

	// Test copy construction
	T gptct_cc(gptct);
	BOOST_CHECK(gep.isEqual(gptct_cc, gptct));
	BOOST_CHECK(gep.isInEqual(gptct_cc, gptct_cp2));

	// Test cloning and loading
	{
		boost::shared_ptr<GObject> gptct_clone_ptr = gptct.GObject::clone();
		BOOST_CHECK_NO_THROW(gptct_cp4.GObject::load(gptct_clone_ptr));
	}
	BOOST_CHECK(gptct_cp4 == gptct);
	BOOST_CHECK(gptct_cp4 == gptct_cp1);
	BOOST_CHECK(gptct_cp4 != gptct_cp2);

	// Test serialization and loading in different serialization modes
	{ // plain text format
		// Copy construction of a new object
		T gptct_cp3(gptct);

		// Check equalities and inequalities
		BOOST_CHECK(gptct_cp3 == gptct);
		BOOST_CHECK(gptct_cp3 == gptct_cp1);
		BOOST_CHECK(gptct_cp3 != gptct_cp2);

		// Serialize cp2 and load into cp3, check inequalities and similarities
		BOOST_CHECK_NO_THROW(gptct_cp3.fromString(gptct_cp2.toString(Gem::Common::SERIALIZATIONMODE_TEXT), Gem::Common::SERIALIZATIONMODE_TEXT));
		//gptct_cp3.fromString(gptct_cp2.toString(Gem::Common::SERIALIZATIONMODE_TEXT), Gem::Common::SERIALIZATIONMODE_TEXT);
		BOOST_CHECK(gep.isInEqual(gptct_cp3, gptct));
		BOOST_CHECK(gep.isInEqual(gptct_cp3, gptct_cp1));
		BOOST_CHECK(gep.isSimilar(gptct_cp3, gptct_cp2));
	}

	{ // XML format
		// Copy construction of a new object
		T gptct_cp3(gptct);

		// Check equalities and inequalities
		BOOST_CHECK(gptct_cp3 == gptct);
		BOOST_CHECK(gptct_cp3 == gptct_cp1);
		BOOST_CHECK(gptct_cp3 != gptct_cp2);

		// Serialize cp2 and load into cp3, check inequalities and similarities
		BOOST_CHECK_NO_THROW(gptct_cp3.fromString(gptct_cp2.toString(Gem::Common::SERIALIZATIONMODE_XML), Gem::Common::SERIALIZATIONMODE_XML));
		BOOST_CHECK(gep.isInEqual(gptct_cp3, gptct));
		BOOST_CHECK(gep.isInEqual(gptct_cp3, gptct_cp1));
		BOOST_CHECK(gep.isSimilar(gptct_cp3, gptct_cp2));
	}

	{ // binary test format
		// Copy construction of a new object
		T gptct_cp3(gptct);

		// Check equalities and inequalities
		BOOST_CHECK(gptct_cp3 == gptct);
		BOOST_CHECK(gptct_cp3 == gptct_cp1);
		BOOST_CHECK(gptct_cp3 != gptct_cp2);

		// Serialize cp2 and load into cp3, check inequalities and similarities
		BOOST_CHECK_NO_THROW(gptct_cp3.fromString(gptct_cp2.toString(Gem::Common::SERIALIZATIONMODE_BINARY), Gem::Common::SERIALIZATIONMODE_BINARY));
		BOOST_CHECK(gep.isInEqual(gptct_cp3, gptct));
		BOOST_CHECK(gep.isInEqual(gptct_cp3, gptct_cp1));
		BOOST_CHECK(gep.isEqual(gptct_cp3, gptct_cp2));
	}
}

/***********************************************************************************/
// Test features that are expected to fail. Test with one derived class only.
BOOST_TEST_CASE_TEMPLATE_FUNCTION( GParameterTCollectionT_failures_expected, T )
{
	GRandomT<RANDOMLOCAL> gr;

	{
		// Self assignment should throw in DEBUG mode
#ifdef DEBUG
		// Default construction
		boost::shared_ptr<T> gptct_ptr(new T());
		BOOST_CHECK_THROW(gptct_ptr->load(gptct_ptr), Gem::Common::gemfony_error_condition);
#endif /* DEBUG */
	}
}

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GParameterTCollectionT class (or its derivatives, to be precise).
class GParameterTCollectionTSuite: public test_suite
{
public:
	/***********************************************************************************/
	GParameterTCollectionTSuite() :test_suite("GParameterTCollectionTSuite") {
		// typedef boost::mpl::list<GDoubleObject, GConstrainedDoubleObject, GInt32Object, GBooleanObject> test_types;
		typedef boost::mpl::list<GDoubleObjectCollection,
						         GInt32ObjectCollection,
						         GBooleanObjectCollection,
						         GConstrainedDoubleObjectCollection,
						         GConstrainedInt32ObjectCollection> test_types;

		add( BOOST_TEST_CASE_TEMPLATE( GParameterTCollectionT_no_failure_expected, test_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( GParameterTCollectionT_failures_expected, test_types ) );
	}
};

#endif /* GPARAMETERTCOLLECTIONT_TEST_HPP_ */
