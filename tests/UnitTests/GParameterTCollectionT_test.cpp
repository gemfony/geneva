/**
 * @file GParameterTCollectionT_test.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
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

// Geneva header files go here
#include "GenevaExceptions.hpp"
#include "GRandom.hpp"
#include "GParameterT.hpp"
#include "GParameterTCollectionT.hpp"
#include "GBoolean.hpp"
#include "GBooleanAdaptor.hpp"
#include "GChar.hpp"
#include "GCharFlipAdaptor.hpp"
#include "GInt32.hpp"
#include "GInt32FlipAdaptor.hpp"
#include "GInt32GaussAdaptor.hpp"
#include "GDouble.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GBoundedDouble.hpp"
#include "GBoundedInt32.hpp"
#include "GDouble.hpp"
#include "GChar.hpp"
#include "GInt32.hpp"
#include "GBoolean.hpp"
#include "GBoundedDoubleCollection.hpp"
#include "GBoundedInt32Collection.hpp"
#include "GDoubleObjectCollection.hpp"
#include "GBooleanObjectCollection.hpp"
#include "GCharObjectCollection.hpp"
#include "GInt32ObjectCollection.hpp"
#include "GStdVectorInterface_test.hpp"

using namespace Gem;
using namespace Gem::Util;
using namespace Gem::GenEvA;

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

/***********************************************************************************/
// This template allows to create default entries for the collection
// This template allows to create items different of the default item.
// Note that these will not have an adaptor assigned to them and
// can thus not be mutated.
template <typename T>
boost::shared_ptr<T> getTemplateItemNoAdaptor() {
	return boost::shared_ptr<T>(new T(0));
}

// Specialization for GBoundedDouble
template <>
boost::shared_ptr<GBoundedDouble> getTemplateItemNoAdaptor<GBoundedDouble>() {
	return boost::shared_ptr<GBoundedDouble>(new GBoundedDouble(0.,0.,1.));
}

// Specialization for GBoundedInt32
template <>
boost::shared_ptr<GBoundedInt32> getTemplateItemNoAdaptor<GBoundedInt32>() {
	return boost::shared_ptr<GBoundedInt32>(new GBoundedInt32(0,0,100));
}

// This template allows to create items different of the default item.
// Note that these will not have an adaptor assigned to them and
// can thus not be mutated.
template <typename T>
boost::shared_ptr<T> getFindItemNoAdaptor() {
	return boost::shared_ptr<T>(new T(1));
}

// Specialization for GBoundedDouble
template <>
boost::shared_ptr<GBoundedDouble> getFindItemNoAdaptor<GBoundedDouble>() {
	return boost::shared_ptr<GBoundedDouble>(new GBoundedDouble(1.,0.,1.));
}

// Specialization for GBoundedInt32
template <>
boost::shared_ptr<GBoundedInt32> getFindItemNoAdaptor<GBoundedInt32>() {
	return boost::shared_ptr<GBoundedInt32>(new GBoundedInt32(1,0,100));
}

/***********************************************************************************/

// This template allows to create default entries for the collection, fully
// equipped with adaptors. As these are different for each type, this
// template should not be called directly. Instead, specializations should
// be used.
template <typename T>
boost::shared_ptr<T> getTemplateItem() {
	// This function should never be called directly
	std::ostringstream error;
	error << "In getTemplateItem<T>(): The template itself should never be called!" << std::endl;
	throw(Gem::GenEvA::geneva_error_condition(error.str()));
}

// Specialization for GBoundedDouble
template <>
boost::shared_ptr<GBoundedDouble> getTemplateItem<GBoundedDouble>() {
	boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(0.,0.,1.));
	gbd_ptr->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor()));
	return gbd_ptr;
}

// Specialization for GBoundedInt32
template <>
boost::shared_ptr<GBoundedInt32> getTemplateItem<GBoundedInt32>() {
	boost::shared_ptr<GBoundedInt32> gbi_ptr(new GBoundedInt32(0,0,100));
	gbi_ptr->addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor()));
	return gbi_ptr;
}

// Specialization for GDouble
template <>
boost::shared_ptr<GDouble> getTemplateItem<GDouble>() {
	boost::shared_ptr<GDouble> gbd_ptr(new GDouble(0.));
	gbd_ptr->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor()));
	return gbd_ptr;
}

// Specialization for GInt32
template <>
boost::shared_ptr<GInt32> getTemplateItem<GInt32>() {
	boost::shared_ptr<GInt32> gint32_ptr(new GInt32(0));
	gint32_ptr->addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor()));
	return gint32_ptr;
}

// Specialization for GBoolean
template <>
boost::shared_ptr<GBoolean> getTemplateItem<GBoolean>() {
	boost::shared_ptr<GBoolean> gboolean_ptr(new GBoolean(false));
	gboolean_ptr->addAdaptor(boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor()));
	return gboolean_ptr;
}

// Specialization for GChar
template <>
boost::shared_ptr<GChar> getTemplateItem<GChar>() {
	boost::shared_ptr<GChar> gchar_ptr(new GChar('a'));
	gchar_ptr->addAdaptor(boost::shared_ptr<GCharFlipAdaptor>(new GCharFlipAdaptor()));
	return gchar_ptr;
}

// This template allows to create items different of the default item, , fully
// equipped with adaptors. As these are different for each type, this
// template should not be called directly. Instead, specializations should
// be used.
template <typename T>
boost::shared_ptr<T> getFindItem() {
	// This function should never be called directly
	std::ostringstream error;
	error << "In getFindItem<T>(): The template itself should never be called!" << std::endl;
	throw(Gem::GenEvA::geneva_error_condition(error.str()));
}

// Specialization for GBoundedDouble
template <>
boost::shared_ptr<GBoundedDouble> getFindItem<GBoundedDouble>() {
	boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble(1.,0.,1.));
	gbd_ptr->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor()));
	return gbd_ptr;
}

// Specialization for GBoundedInt32
template <>
boost::shared_ptr<GBoundedInt32> getFindItem<GBoundedInt32>() {
	boost::shared_ptr<GBoundedInt32> gbi_ptr(new GBoundedInt32(1,0,100));
	gbi_ptr->addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor()));
	return gbi_ptr;
}

// Specialization for GDouble
template <>
boost::shared_ptr<GDouble> getFindItem<GDouble>() {
	boost::shared_ptr<GDouble> gbd_ptr(new GDouble(1.));
	gbd_ptr->addAdaptor(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor()));
	return gbd_ptr;
}

// Specialization for GInt32
template <>
boost::shared_ptr<GInt32> getFindItem<GInt32>() {
	boost::shared_ptr<GInt32> gint32_ptr(new GInt32(1));
	gint32_ptr->addAdaptor(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor()));
	return gint32_ptr;
}

// Specialization for GBoolean
template <>
boost::shared_ptr<GBoolean> getFindItem<GBoolean>() {
	boost::shared_ptr<GBoolean> gboolean_ptr(new GBoolean(true));
	gboolean_ptr->addAdaptor(boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor()));
	return gboolean_ptr;
}

// Specialization for GChar
template <>
boost::shared_ptr<GChar> getFindItem<GChar>() {
	boost::shared_ptr<GChar> gchar_ptr(new GChar('b'));
	gchar_ptr->addAdaptor(boost::shared_ptr<GCharFlipAdaptor>(new GCharFlipAdaptor()));
	return gchar_ptr;
}

/********************************************************************************************/
// The actual unit tests for this class

// Test features that are expected to work
BOOST_TEST_CASE_TEMPLATE_FUNCTION( GParameterTCollectionT_no_failure_expected, T )
{
	GRandom gr;

	// Default construction
	GParameterTCollectionT<T> gptct;

	// Check the vector interface
	boost::shared_ptr<T> templItem_ptr = getTemplateItem<T>();
	boost::shared_ptr<T> findItem_ptr = getFindItem<T>();
	// Make sure both items are indeed different
	BOOST_CHECK(*templItem_ptr != *findItem_ptr);

	// Run the actual vector tests
	stdvectorinterfacetestSP(gptct, templItem_ptr, findItem_ptr);

	// vector functionality of the collection has now been thoroughly tested.
	// Collection items should be remaining in the object. Check.
	BOOST_CHECK(!gptct.empty());

	// Create two copies of the object
	GParameterTCollectionT<T> gptct_cp1, gptct_cp2, gptct_cp4;
	gptct_cp1 = gptct_cp2 = gptct_cp4 = gptct;

	// Check that they are indeed identical
	BOOST_CHECK(gptct_cp1 == gptct);
	BOOST_CHECK(gptct_cp2 == gptct);

	// Mutate the second copy and check that it has become different from the other two collections
	BOOST_CHECK_NO_THROW(gptct_cp2.mutate());
	BOOST_CHECK(gptct_cp2 != gptct);
	BOOST_CHECK(gptct_cp2 != gptct_cp1);

	// Assign for later usage
	gptct_cp4=gptct_cp2;

	// Test copy construction
	GParameterTCollectionT<T> gptct_cc(gptct);
	BOOST_CHECK(gptct_cc.isEqualTo(gptct));
	BOOST_CHECK(gptct_cc.isNotEqualTo(gptct_cp2));

	// Test cloning and loading
	GObject *gptct_clone_ptr = gptct.clone();
	BOOST_CHECK_NO_THROW(gptct_cp4.load(gptct_clone_ptr));
	delete gptct_clone_ptr;
	BOOST_CHECK(gptct_cp4 == gptct);
	BOOST_CHECK(gptct_cp4 == gptct_cp1);
	BOOST_CHECK(gptct_cp4 != gptct_cp2);

	// Test serialization and loading in different serialization modes
	{ // plain text format
		// Copy construction of a new object
		GParameterTCollectionT<T> gptct_cp3(gptct);

		// Check equalities and inequalities
		BOOST_CHECK(gptct_cp3 == gptct);
		BOOST_CHECK(gptct_cp3 == gptct_cp1);
		BOOST_CHECK(gptct_cp3 != gptct_cp2);

		// Serialize cp2 and load into cp3, check inequalities and similarities
		BOOST_CHECK_NO_THROW(gptct_cp3.fromString(gptct_cp2.toString(TEXTSERIALIZATION), TEXTSERIALIZATION));
		//gptct_cp3.fromString(gptct_cp2.toString(TEXTSERIALIZATION), TEXTSERIALIZATION);
		BOOST_CHECK(!gptct_cp3.isEqualTo(gptct));
		BOOST_CHECK(!gptct_cp3.isEqualTo(gptct_cp1));
		BOOST_CHECK(gptct_cp3.isSimilarTo(gptct_cp2, exp(-10)));
	}

	{ // XML format
		// Copy construction of a new object
		GParameterTCollectionT<T> gptct_cp3(gptct);

		// Check equalities and inequalities
		BOOST_CHECK(gptct_cp3 == gptct);
		BOOST_CHECK(gptct_cp3 == gptct_cp1);
		BOOST_CHECK(gptct_cp3 != gptct_cp2);

		// Serialize cp2 and load into cp3, check inequalities and similarities
		BOOST_CHECK_NO_THROW(gptct_cp3.fromString(gptct_cp2.toString(XMLSERIALIZATION), XMLSERIALIZATION));
		BOOST_CHECK(!gptct_cp3.isEqualTo(gptct));
		BOOST_CHECK(!gptct_cp3.isEqualTo(gptct_cp1));
		BOOST_CHECK(gptct_cp3.isSimilarTo(gptct_cp2, exp(-10)));
	}

	{ // binary test format
		// Copy construction of a new object
		GParameterTCollectionT<T> gptct_cp3(gptct);

		// Check equalities and inequalities
		BOOST_CHECK(gptct_cp3 == gptct);
		BOOST_CHECK(gptct_cp3 == gptct_cp1);
		BOOST_CHECK(gptct_cp3 != gptct_cp2);

		// Serialize cp2 and load into cp3, check inequalities and similarities
		BOOST_CHECK_NO_THROW(gptct_cp3.fromString(gptct_cp2.toString(BINARYSERIALIZATION), BINARYSERIALIZATION));
		BOOST_CHECK(!gptct_cp3.isEqualTo(gptct));
		BOOST_CHECK(!gptct_cp3.isEqualTo(gptct_cp1));
		BOOST_CHECK(gptct_cp3.isEqualTo(gptct_cp2));
	}
}

/***********************************************************************************/
// Test features that are expected to fail. Test with one derived class only.
BOOST_TEST_CASE_TEMPLATE_FUNCTION( GParameterTCollectionT_failures_expected, T )
{
	GRandom gr;

	{
		// Self assignment should throw in DEBUG mode
#ifdef DEBUG
		// Default construction
		GParameterTCollectionT<T> gptct;
		BOOST_CHECK_THROW(gptct.load(&gptct), Gem::GenEvA::geneva_error_condition);
#endif /* DEBUG */
	}
}

/***********************************************************************************/
// This test suite checks as much as possible of the functionality provided
// by the GParameterT class. Tests include features of the parent class
// GParameterBaseWithAdaptorsT, as it cannot be instantiated itself.
class GParameterTCollectionTSuite: public test_suite
{
public:
	/***********************************************************************************/
	GParameterTCollectionTSuite() :test_suite("GParameterTCollectionTSuite") {
		// typedef boost::mpl::list<GDouble, GBoundedDouble, GChar, GInt32, GBoolean> test_types;
		typedef boost::mpl::list<GDouble, GChar, GInt32, GBoolean, GBoundedDouble, GBoundedInt32> test_types;

		add( BOOST_TEST_CASE_TEMPLATE( GParameterTCollectionT_no_failure_expected, test_types ) );
		add( BOOST_TEST_CASE_TEMPLATE( GParameterTCollectionT_failures_expected, test_types ) );
	}
};
